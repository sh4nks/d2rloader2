#include "gamelauncher_windows.h"
#include "../accounts/gamesettings.h"
#include "../accounts/lootfilters.h"
#include "../accounts/profile.h"
#include "../core/logging.h"
#include "d2rloaderconfig.h"

#include <KLocalizedString>
#include <QDir>
#include <QFileInfo>
#include <QPointer>
#include <QProcess>
#include <QProcessEnvironment>
#include <QTimer>
#include <QWinEventNotifier>

#include <chrono>
#include <iterator>
#include <optional>
#include <string>
#include <utility>
#include <vector>

// After the Qt headers: windows.h defines macros that clash with them.
#include <windows.h>

#include <tlhelp32.h>
#include <wincrypt.h>
#include <winternl.h>

namespace
{
const QString GameExecutable = QStringLiteral("D2R.exe");

/**
 * The game creates its single instance event right after starting, before
 * its window. Closing it is retried for a while in case the machine is slow.
 */
const int GuardAttempts = 60;
const std::chrono::milliseconds GuardInterval{500};

/**
 * The event every game holds to find other instances. Its full name is
 * "\Sessions\<session>\BaseNamedObjects\DiabloII Check For Other Instances".
 */
const QString InstanceGuardName = QStringLiteral("DiabloII Check For Other Instances");

/**
 * How long the game gets to close itself after WM_CLOSE before it is ended.
 */
const std::chrono::seconds ShutdownGrace{10};

/**
 * The window position is read this often while the game runs, and saved once
 * it stays the same for two reads, so a window being dragged is not saved at
 * every step.
 */
const std::chrono::seconds PositionInterval{1};

/**
 * The game reads its login token from the launch options Battle.net keeps
 * for it. The token is protected with DPAPI and this entropy, the same
 * Battle.net uses, so that the game can read it back.
 */
const wchar_t *const LaunchOptionsKey = L"SOFTWARE\\Blizzard Entertainment\\Battle.net\\Launch Options\\OSI";
BYTE TokenEntropy[] = {0xC8, 0x76, 0xF4, 0xAE, 0x4C, 0x95, 0x2E, 0xFE, 0xF2, 0xFA, 0x0F, 0x54, 0x19, 0xC0, 0x9C, 0x43};

/**
 * NtQueryInformationProcess(ProcessHandleInformation) lists the handles of a
 * process. Neither the class nor its structures are in winternl.h.
 */
struct HandleEntry {
    HANDLE HandleValue;
    ULONG_PTR HandleCount;
    ULONG_PTR PointerCount;
    ULONG GrantedAccess;
    ULONG ObjectTypeIndex;
    ULONG HandleAttributes;
    ULONG Reserved;
};

struct HandleSnapshot {
    ULONG_PTR NumberOfHandles;
    ULONG_PTR Reserved;
    HandleEntry Handles[1];
};

const auto ProcessHandleInformationClass = static_cast<PROCESSINFOCLASS>(51);
const auto ObjectNameInformationClass = static_cast<OBJECT_INFORMATION_CLASS>(1);
const auto StatusInfoLengthMismatch = static_cast<NTSTATUS>(0xC0000004);

QString systemError(DWORD code)
{
    wchar_t *buffer = nullptr;
    const DWORD length = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                        nullptr,
                                        code,
                                        0,
                                        reinterpret_cast<wchar_t *>(&buffer),
                                        0,
                                        nullptr);
    const QString message = length > 0 ? QString::fromWCharArray(buffer, static_cast<qsizetype>(length)).trimmed() : QString();
    LocalFree(buffer);
    return message.isEmpty() ? QStringLiteral("error %1").arg(code) : QStringLiteral("%1 (%2)").arg(message).arg(code);
}

QString lastError()
{
    return systemError(GetLastError());
}

QString toQString(const UNICODE_STRING &string)
{
    return QString::fromWCharArray(string.Buffer, string.Length / sizeof(wchar_t));
}

QString windowTitle(HWND window)
{
    wchar_t title[512];
    const int length = GetWindowTextW(window, title, static_cast<int>(std::size(title)));
    return QString::fromWCharArray(title, length);
}

qint64 windowProcess(HWND window)
{
    DWORD pid = 0;
    GetWindowThreadProcessId(window, &pid);
    return pid;
}

BOOL CALLBACK visitWindow(HWND window, LPARAM visitor)
{
    (*reinterpret_cast<const std::function<void(HWND)> *>(visitor))(window);
    return TRUE;
}

/**
 * Calls \a visit for every visible top level window that has a title and no
 * owner, which is what the game's main window is. Its other windows are
 * hidden or owned.
 */
void forEachMainWindow(const std::function<void(HWND)> &visit)
{
    const std::function<void(HWND)> filter = [&visit](HWND window) {
        if (IsWindowVisible(window) && GetWindow(window, GW_OWNER) == nullptr && GetWindowTextLengthW(window) > 0) {
            visit(window);
        }
    };
    EnumWindows(visitWindow, reinterpret_cast<LPARAM>(&filter));
}

QList<HWND> mainWindowsOf(qint64 pid)
{
    QList<HWND> windows;
    forEachMainWindow([&windows, pid](HWND window) {
        if (windowProcess(window) == pid) {
            windows << window;
        }
    });
    return windows;
}

/**
 * Whether \a window has a frame, which it only has in windowed mode. Moving a
 * full screen window would show the desktop behind it.
 */
bool isWindowed(HWND window)
{
    return (GetWindowLongPtrW(window, GWL_STYLE) & WS_CAPTION) == WS_CAPTION;
}

/**
 * Where \a window is, or nothing while it is minimized or not in windowed
 * mode, where its position is meaningless.
 */
std::optional<QPoint> windowPosition(HWND window)
{
    RECT rect{};
    if (!isWindowed(window) || IsIconic(window) || !GetWindowRect(window, &rect)) {
        return std::nullopt;
    }
    return QPoint(rect.left, rect.top);
}

/**
 * The pids of every running D2R.exe.
 */
QList<qint64> gameProcesses()
{
    QList<qint64> pids;
    const HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        qCWarning(LOG_GAME) << "could not list the running processes:" << lastError();
        return pids;
    }

    PROCESSENTRY32W entry{};
    entry.dwSize = sizeof(entry);
    if (Process32FirstW(snapshot, &entry)) {
        do {
            if (QString::fromWCharArray(entry.szExeFile).compare(GameExecutable, Qt::CaseInsensitive) == 0) {
                pids << entry.th32ProcessID;
            }
        } while (Process32NextW(snapshot, &entry));
    }
    CloseHandle(snapshot);
    return pids;
}

/**
 * The handles \a process holds, or nothing with \a error set when they
 * cannot be read.
 */
std::optional<std::vector<HandleEntry>> processHandles(HANDLE process, QString &error)
{
    // ULONG_PTR keeps the buffer aligned for the snapshot structure.
    std::vector<ULONG_PTR> buffer(8192);
    for (;;) {
        const auto size = static_cast<ULONG>(buffer.size() * sizeof(ULONG_PTR));
        ULONG needed = 0;
        const NTSTATUS status = NtQueryInformationProcess(process, ProcessHandleInformationClass, buffer.data(), size, &needed);
        if (status == StatusInfoLengthMismatch) {
            // The process keeps opening handles, so some room is added on top.
            buffer.resize(std::max<size_t>(needed / sizeof(ULONG_PTR), buffer.size()) + 1024);
            continue;
        }
        if (status < 0) {
            error = QStringLiteral("NtQueryInformationProcess failed with status 0x%1").arg(static_cast<quint32>(status), 8, 16, QLatin1Char('0'));
            return std::nullopt;
        }
        const auto *snapshot = reinterpret_cast<const HandleSnapshot *>(buffer.data());
        return std::vector<HandleEntry>(snapshot->Handles, snapshot->Handles + snapshot->NumberOfHandles);
    }
}

/**
 * The type of the object behind \a handle, like "Event" or "File".
 */
QString objectType(HANDLE handle)
{
    ULONG_PTR buffer[64];
    ULONG length = 0;
    if (NtQueryObject(handle, ObjectTypeInformation, buffer, sizeof(buffer), &length) < 0) {
        return {};
    }
    return toQString(reinterpret_cast<const PUBLIC_OBJECT_TYPE_INFORMATION *>(buffer)->TypeName);
}

/**
 * The name of the object behind \a handle, empty when it has none.
 */
QString objectName(HANDLE handle)
{
    // OBJECT_NAME_INFORMATION is a UNICODE_STRING followed by its characters.
    ULONG_PTR buffer[128];
    ULONG length = 0;
    if (NtQueryObject(handle, ObjectNameInformationClass, buffer, sizeof(buffer), &length) < 0) {
        return {};
    }
    return toQString(*reinterpret_cast<const UNICODE_STRING *>(buffer));
}

enum class GuardState {
    NotFound, /* the game has not created its event yet */
    Closed,
    Failed,
};

/**
 * Closes the single instance event in the game running as \a pid, which lets
 * another game start. The handle is closed from the outside, like handle.exe
 * does, by duplicating it with DUPLICATE_CLOSE_SOURCE. Unlike handle.exe this
 * needs no administrator rights, as the game runs as the same user.
 */
GuardState closeInstanceGuard(qint64 pid, QString &error)
{
    const HANDLE process = OpenProcess(PROCESS_DUP_HANDLE | PROCESS_QUERY_INFORMATION, FALSE, static_cast<DWORD>(pid));
    if (process == nullptr) {
        error = lastError();
        return GuardState::Failed;
    }

    const auto handles = processHandles(process, error);
    if (!handles) {
        CloseHandle(process);
        return GuardState::Failed;
    }

    GuardState state = GuardState::NotFound;
    // Handles of one type share an index, so once an event has been seen the
    // other handles are told apart without asking the kernel about each.
    std::optional<ULONG> eventType;
    for (const HandleEntry &entry : *handles) {
        if (eventType && entry.ObjectTypeIndex != *eventType) {
            continue;
        }
        HANDLE local = nullptr;
        if (!DuplicateHandle(process, entry.HandleValue, GetCurrentProcess(), &local, 0, FALSE, DUPLICATE_SAME_ACCESS)) {
            continue;
        }
        // Asking a pipe for its name can block, so the type is checked first.
        const bool isEvent = eventType.has_value() || objectType(local) == QStringLiteral("Event");
        if (isEvent) {
            eventType = entry.ObjectTypeIndex;
        }
        const bool found = isEvent && objectName(local).endsWith(InstanceGuardName);
        CloseHandle(local);
        if (!found) {
            continue;
        }

        if (DuplicateHandle(process, entry.HandleValue, nullptr, nullptr, 0, FALSE, DUPLICATE_CLOSE_SOURCE)) {
            state = GuardState::Closed;
        } else {
            error = lastError();
            state = GuardState::Failed;
        }
        break;
    }
    CloseHandle(process);
    return state;
}

/**
 * Writes the token of \a profile, and the region the game logs in to, into
 * the launch options the game reads when started with -uid osi.
 */
std::expected<void, QString> writeToken(const Profile *profile)
{
    QByteArray token = profile->token().toUtf8();
    DATA_BLOB input{static_cast<DWORD>(token.size()), reinterpret_cast<BYTE *>(token.data())};
    DATA_BLOB entropy{static_cast<DWORD>(sizeof(TokenEntropy)), TokenEntropy};
    DATA_BLOB output{};
    if (!CryptProtectData(&input, nullptr, &entropy, nullptr, nullptr, 0, &output)) {
        return std::unexpected(i18nc("@info", "Could not encrypt the token: %1", lastError()));
    }
    const QByteArray protectedToken(reinterpret_cast<const char *>(output.pbData), static_cast<qsizetype>(output.cbData));
    LocalFree(output.pbData);

    HKEY key = nullptr;
    LSTATUS status = RegCreateKeyExW(HKEY_CURRENT_USER, LaunchOptionsKey, 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, nullptr, &key, nullptr);
    if (status != ERROR_SUCCESS) {
        return std::unexpected(i18nc("@info", "Could not open the Battle.net launch options in the registry: %1", systemError(static_cast<DWORD>(status))));
    }

    status =
        RegSetValueExW(key, L"WEB_TOKEN", 0, REG_BINARY, reinterpret_cast<const BYTE *>(protectedToken.constData()), static_cast<DWORD>(protectedToken.size()));
    if (status == ERROR_SUCCESS) {
        // "eu.actual.battle.net" -> "EU", as Battle.net stores it.
        const std::wstring region = RegionModel::server(profile->region()).section(u'.', 0, 0).toUpper().toStdWString();
        status = RegSetValueExW(key,
                                L"REGION",
                                0,
                                REG_SZ,
                                reinterpret_cast<const BYTE *>(region.c_str()),
                                static_cast<DWORD>((region.size() + 1) * sizeof(wchar_t)));
    }
    RegCloseKey(key);
    if (status != ERROR_SUCCESS) {
        return std::unexpected(i18nc("@info", "Could not write the token into the registry: %1", systemError(static_cast<DWORD>(status))));
    }
    return {};
}
}

WindowsGameLauncher::WindowsGameLauncher(QObject *parent)
    : GameLauncher(parent)
{
}

WindowsGameLauncher::~WindowsGameLauncher()
{
    for (void *process : std::as_const(m_processes)) {
        CloseHandle(process);
    }
}

void WindowsGameLauncher::launch(Profile *profile)
{
    const auto arguments = gameArguments(profile);
    if (!arguments) {
        Q_EMIT failed(profile, arguments.error());
        return;
    }

    if (const auto applied = GameSettings::apply(profile); !applied) {
        qCWarning(LOG_GAME) << "starting" << profile->profileName() << "with the game's own settings:" << applied.error();
    }
    if (const auto applied = LootFilters::apply(profile); !applied) {
        qCWarning(LOG_GAME) << "starting" << profile->profileName() << "with the game's own loot filters:" << applied.error();
    }

    if (profile->authMethod() == AuthMethodModel::Steam) {
        launchSteamUrl(profile, *arguments);
        return;
    }

    const auto executable = gameExecutable(profile);
    if (!executable) {
        Q_EMIT failed(profile, executable.error());
        return;
    }

    startGame(profile, *executable, *arguments);
}

void WindowsGameLauncher::startGame(Profile *profile, const QString &executable, const QStringList &arguments)
{
    if (profile->authMethod() == AuthMethodModel::Token) {
        // The game reads the token while it starts, before its window shows.
        if (m_tokenGamePid != 0) {
            qCInfo(LOG_GAME) << "holding" << profile->profileName() << "until the game running as pid" << m_tokenGamePid << "has read its token";
            m_heldLaunches.append({QPointer(profile), executable, arguments});
            return;
        }
        if (const auto written = writeToken(profile); !written) {
            Q_EMIT failed(profile, written.error());
            return;
        }
        qCDebug(LOG_GAME) << "token written for" << profile->profileName();
    }

    // A game started outside the launcher still holds its single instance
    // event, which would keep this one from starting.
    for (const qint64 running : gameProcesses()) {
        QString error;
        switch (closeInstanceGuard(running, error)) {
        case GuardState::Closed:
            qCInfo(LOG_GAME) << "released the single instance check of the game already running as pid" << running;
            break;
        case GuardState::Failed:
            qCWarning(LOG_GAME) << "could not release the single instance check of the game running as pid" << running << ":" << error;
            break;
        case GuardState::NotFound:
            break;
        }
    }

    launchGame(profile, executable, arguments);
}

void WindowsGameLauncher::tokenRead(qint64 pid)
{
    if (pid != m_tokenGamePid) {
        return;
    }
    m_tokenGamePid = 0;
    // A held launch that fails leaves the next one its turn.
    while (m_tokenGamePid == 0 && !m_heldLaunches.isEmpty()) {
        const HeldLaunch held = m_heldLaunches.takeFirst();
        if (held.profile) {
            startGame(held.profile, held.executable, held.arguments);
        }
    }
}

void WindowsGameLauncher::terminate(qint64 pid)
{
    if (m_terminating.contains(pid)) {
        return;
    }

    // A game that is still starting has no window to ask.
    const QList<HWND> windows = mainWindowsOf(pid);
    if (windows.isEmpty()) {
        kill(pid);
        return;
    }
    m_terminating.insert(pid);

    qCInfo(LOG_GAME) << "asking pid" << pid << "to close";
    for (HWND window : windows) {
        PostMessageW(window, WM_CLOSE, 0, 0);
    }
    QTimer::singleShot(ShutdownGrace, this, [this, pid] {
        if (isRunning(pid)) {
            qCWarning(LOG_GAME) << "pid" << pid << "did not close in time";
            kill(pid);
        }
    });
}

void WindowsGameLauncher::kill(qint64 pid)
{
    const HANDLE process = OpenProcess(PROCESS_TERMINATE, FALSE, static_cast<DWORD>(pid));
    if (process == nullptr) {
        qCWarning(LOG_GAME) << "could not open pid" << pid << "to terminate it:" << lastError();
        return;
    }
    qCInfo(LOG_GAME) << "terminating pid" << pid;
    if (!TerminateProcess(process, 1)) {
        qCWarning(LOG_GAME) << "could not terminate pid" << pid << ":" << lastError();
    }
    CloseHandle(process);
}

void WindowsGameLauncher::adopt(const QList<Profile *> &profiles)
{
    const QList<qint64> games = gameProcesses();
    if (games.isEmpty()) {
        return;
    }

    forEachMainWindow([this, &games, &profiles](HWND window) {
        const qint64 pid = windowProcess(window);
        if (!games.contains(pid) || m_processes.contains(pid)) {
            return;
        }
        const QString title = windowTitle(window);
        for (Profile *profile : profiles) {
            if (title != profile->windowTitle()) {
                continue;
            }
            qCInfo(LOG_GAME) << "found" << profile->profileName() << "still running as pid" << pid;
            Q_EMIT launched(profile, pid);
            watch(pid);
            if (profile->rememberWindowPosition()) {
                followWindow(profile, pid, window);
            }
            break;
        }
    });
}

void WindowsGameLauncher::launchGame(Profile *profile, const QString &executable, const QStringList &arguments)
{
    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    addProfileEnvironment(environment, profile);

    QProcess process;
    process.setProgram(executable);
    process.setArguments(arguments);
    process.setWorkingDirectory(QFileInfo(executable).absolutePath());
    process.setProcessEnvironment(environment);

    qCInfo(LOG_GAME) << "launching" << profile->profileName() << ":" << executable << redacted(arguments);

    qint64 pid = 0;
    if (!process.startDetached(&pid)) {
        Q_EMIT failed(profile, i18nc("@info", "Could not start %1: %2", executable, process.errorString()));
        return;
    }

    qCInfo(LOG_GAME) << "started" << profile->profileName() << "as pid" << pid;
    Q_EMIT launched(profile, pid);
    if (profile->authMethod() == AuthMethodModel::Token) {
        m_tokenGamePid = pid;
    }
    watch(pid);
    releaseInstanceGuard(pid, GuardAttempts);
    renameWindow(profile, pid, RenameAttempts);
}

void WindowsGameLauncher::releaseInstanceGuard(qint64 pid, int attemptsLeft)
{
    if (!isRunning(pid)) {
        return;
    }

    QString error;
    switch (closeInstanceGuard(pid, error)) {
    case GuardState::Closed:
        qCInfo(LOG_GAME) << "released the single instance check of pid" << pid;
        return;
    case GuardState::Failed:
        qCWarning(LOG_GAME) << "could not release the single instance check of pid" << pid << ":" << error << "- no other game can start while it runs";
        return;
    case GuardState::NotFound:
        break;
    }

    if (attemptsLeft <= 1) {
        qCWarning(LOG_GAME) << "gave up releasing the single instance check of pid" << pid << "- no other game can start while it runs";
        return;
    }
    QTimer::singleShot(GuardInterval, this, [this, pid, attemptsLeft] {
        releaseInstanceGuard(pid, attemptsLeft - 1);
    });
}

void WindowsGameLauncher::renameWindow(Profile *profile, qint64 pid, int attemptsLeft)
{
    if (!isRunning(pid)) {
        tokenRead(pid);
        return;
    }

    const QString title = profile->windowTitle();
    const QList<HWND> windows = mainWindowsOf(pid);
    if (windows.isEmpty()) {
        if (attemptsLeft <= 1) {
            qCWarning(LOG_GAME) << "gave up renaming the window of pid" << pid << "to" << title;
            tokenRead(pid);
            return;
        }
        QTimer::singleShot(RenameInterval, this, [this, profile = QPointer(profile), pid, attemptsLeft] {
            if (profile) {
                renameWindow(profile, pid, attemptsLeft - 1);
            }
        });
        return;
    }

    // The window is up, so the game has read its token by now.
    tokenRead(pid);
    for (HWND window : windows) {
        if (!SetWindowTextW(window, title.toStdWString().c_str())) {
            qCWarning(LOG_GAME) << "could not rename the window of pid" << pid << ":" << lastError();
            return;
        }
    }
    qCInfo(LOG_GAME) << "renamed the window of pid" << pid << "to" << title;
    if (profile->rememberWindowPosition()) {
        followWindow(profile, pid, windows.first());
    }
}

void WindowsGameLauncher::followWindow(Profile *profile, qint64 pid, void *handle)
{
    HWND window = static_cast<HWND>(handle);
    const QString title = profile->windowTitle();

    if (const std::optional<QPoint> saved = profile->windowPosition(); saved && isWindowed(window)) {
        RECT rect{};
        GetWindowRect(window, &rect);
        RECT target{saved->x(), saved->y(), saved->x() + (rect.right - rect.left), saved->y() + (rect.bottom - rect.top)};
        if (MonitorFromRect(&target, MONITOR_DEFAULTTONULL) == nullptr) {
            qCWarning(LOG_GAME) << "not moving the window" << title << "to" << *saved << "- it would end up on no monitor";
        } else if (SetWindowPos(window, nullptr, saved->x(), saved->y(), 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE)) {
            qCInfo(LOG_GAME) << "moved the window" << title << "to" << *saved;
        } else {
            qCWarning(LOG_GAME) << "could not move the window" << title << ":" << lastError();
        }
    }

    auto *timer = new QTimer(this);
    timer->setInterval(PositionInterval);
    connect(timer, &QTimer::timeout, this, [timer, window, pid, title, profile = QPointer(profile), previous = std::optional<QPoint>()]() mutable {
        // Windows hands the handle of a closed window to a new one soon.
        if (!profile || windowProcess(window) != pid) {
            qCDebug(LOG_GAME) << "stopped following the window" << title;
            timer->deleteLater();
            return;
        }
        const std::optional<QPoint> position = windowPosition(window);
        if (position && position == previous) {
            profile->setWindowPosition(*position);
        }
        previous = position;
    });
    timer->start();
    qCDebug(LOG_GAME) << "following the window" << title;
}

void WindowsGameLauncher::watch(qint64 pid)
{
    const HANDLE process = OpenProcess(SYNCHRONIZE | PROCESS_QUERY_LIMITED_INFORMATION, FALSE, static_cast<DWORD>(pid));
    if (process == nullptr) {
        // Most likely the process is already gone.
        qCWarning(LOG_GAME) << "cannot watch pid" << pid << ":" << lastError();
        Q_EMIT finished(pid);
        return;
    }
    m_processes.insert(pid, process);

    // A process handle is signaled once the process exits.
    auto *notifier = new QWinEventNotifier(process, this);
    connect(notifier, &QWinEventNotifier::activated, this, [this, notifier, pid] {
        notifier->setEnabled(false);
        notifier->deleteLater();
        CloseHandle(m_processes.take(pid));
        m_terminating.remove(pid);
        qCInfo(LOG_GAME) << "pid" << pid << "exited";
        Q_EMIT finished(pid);
        tokenRead(pid);
    });
}

bool WindowsGameLauncher::isRunning(qint64 pid) const
{
    return m_processes.contains(pid);
}
