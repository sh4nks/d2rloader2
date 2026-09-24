#include "gamelauncher_linux.h"
#include "../accounts/gamesettings.h"
#include "../accounts/lootfilters.h"
#include "../accounts/profile.h"
#include "../core/logging.h"
#include "d2rloaderconfig.h"
#include "kwinwindowpositions.h"

#include <KLocalizedString>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMultiHash>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPointer>
#include <QProcess>
#include <QSaveFile>
#include <QSocketNotifier>
#include <QStandardPaths>
#include <QTimer>

#include <csignal>
#include <memory>
#include <sys/syscall.h>
#include <unistd.h>

namespace
{
const QString SteamAppId = QStringLiteral("2536520");
const QString DefaultProtonPath = QStringLiteral("UMU-Latest");

/**
 * d2rreg writes the token into the registry of a wineprefix, renames the game
 * window and keeps it in place. The version is part of the file name, so a
 * new version is downloaded next to the old one.
 */
const QString TokenToolVersion = QStringLiteral("v0.0.4");

/**
 * The game window shows up a while after D2R.exe starts, longer on a first
 * start that compiles shaders, so renaming it is retried for about 2 minutes.
 */
const int RenameAttempts = 60;
const std::chrono::seconds RenameInterval{2};
const QUrl TokenToolUrl(QStringLiteral("https://github.com/sh4nks/d2rreg/releases/download/%1/d2rreg.exe").arg(TokenToolVersion));

QString tokenToolPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QStringLiteral("/d2rloader/d2rreg-%1.exe").arg(TokenToolVersion);
}

QString umuRun()
{
    return QStandardPaths::findExecutable(QStringLiteral("umu-run"));
}

/**
 * Every process started for a profile carries its normalized name in this
 * variable, so its game can be told apart and found again after a restart.
 */
const QString ProfileMarker = QStringLiteral("D2RLOADER_PROFILE");

QList<qint64> processIds()
{
    QList<qint64> pids;
    const QStringList entries = QDir(QStringLiteral("/proc")).entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &entry : entries) {
        bool isPid = false;
        const qint64 pid = entry.toLongLong(&isPid);
        if (isPid) {
            pids << pid;
        }
    }
    return pids;
}

/**
 * After wineserver -k, the host side of the game (umu-run, pressure-vessel)
 * exits within a fraction of a second. Whatever is still alive after this is
 * killed.
 */
const std::chrono::seconds ShutdownGrace{5};

/**
 * wine gives each window 5 seconds to answer the end of the session before
 * showing its end-task dialog and waiting for the user.
 */
const std::chrono::seconds EndSessionTimeout{10};

/**
 * The value of \a name in the environment of \a pid, empty if it has none or
 * cannot be read.
 */
QString environmentValue(qint64 pid, const QString &name)
{
    QFile file(QStringLiteral("/proc/%1/environ").arg(pid));
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }

    const QByteArray prefix = name.toLatin1() + '=';
    const QList<QByteArray> variables = file.readAll().split('\0');
    for (const QByteArray &variable : variables) {
        if (variable.startsWith(prefix)) {
            return QString::fromUtf8(variable.mid(prefix.size()));
        }
    }
    return {};
}

/**
 * Whether \a pid is D2R.exe itself. Its comm is a thread name like "Main",
 * and the host side processes only pass D2R.exe as an argument, so argv[0]
 * is what tells them apart.
 */
bool isGameProcess(qint64 pid)
{
    QFile file(QStringLiteral("/proc/%1/cmdline").arg(pid));
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    const QByteArray cmdline = file.readAll();
    return cmdline.left(cmdline.indexOf('\0')).endsWith("D2R.exe");
}

/**
 * Maps every process to its children, read from /proc.
 */
QMultiHash<qint64, qint64> processChildren()
{
    QMultiHash<qint64, qint64> children;
    const QList<qint64> pids = processIds();
    for (const qint64 pid : pids) {
        QFile stat(QStringLiteral("/proc/%1/stat").arg(pid));
        if (!stat.open(QIODevice::ReadOnly)) {
            continue;
        }
        // "pid (comm) state ppid ...": comm may contain spaces and parentheses,
        // so the fields are only split after its closing parenthesis.
        const QByteArray line = stat.readAll();
        const QList<QByteArray> fields = line.mid(line.lastIndexOf(')') + 2).split(' ');
        if (fields.size() > 1) {
            children.insert(fields.at(1).toLongLong(), pid);
        }
    }
    return children;
}

/**
 * The wineserver of the processes carrying \a marker, 0 if none is running.
 */
qint64 wineserverProcess(const QString &marker)
{
    const QList<qint64> pids = processIds();
    for (const qint64 pid : pids) {
        QFile comm(QStringLiteral("/proc/%1/comm").arg(pid));
        if (comm.open(QIODevice::ReadOnly) && comm.readAll().trimmed() == "wineserver" && environmentValue(pid, ProfileMarker) == marker) {
            return pid;
        }
    }
    return 0;
}

/**
 * The wine program \a name of the Proton running \a wineserver. Wine programs
 * have to match their wineserver, as the protocol differs between versions.
 * Proton's run fine outside the pressure-vessel container, which saves the
 * seconds umu-run needs to start one.
 */
QString wineBinary(qint64 wineserver, const QString &name)
{
    return QFileInfo(QFileInfo(QStringLiteral("/proc/%1/exe").arg(wineserver)).symLinkTarget()).dir().filePath(name);
}

/**
 * The environment for running wine against \a wineserver: its wineprefix and
 * its sync method, which client and server have to agree on. WINEPREFIX is
 * empty if \a wineserver is not running.
 */
QProcessEnvironment wineEnvironment(qint64 wineserver)
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    for (const QString &name : {QStringLiteral("WINEPREFIX"), QStringLiteral("WINEESYNC"), QStringLiteral("WINEFSYNC")}) {
        env.insert(name, environmentValue(wineserver, name));
    }
    env.insert(QStringLiteral("WINEDEBUG"), QStringLiteral("-all"));
    return env;
}

/**
 * d2rreg running \a arguments in the wineprefix of \a wineserver.
 */
QProcess *tokenToolProcess(qint64 wineserver, const QStringList &arguments, QObject *parent)
{
    auto *process = new QProcess(parent);
    process->setProgram(wineBinary(wineserver, QStringLiteral("wine")));
    process->setArguments(QStringList{tokenToolPath()} + arguments);
    process->setProcessEnvironment(wineEnvironment(wineserver));
    return process;
}

void killProcessTree(qint64 pid)
{
    // A game that is still starting forks while the processes are being
    // collected, so every process found is stopped first and /proc is read
    // again until nothing new turns up. Nothing escapes the SIGKILL that way.
    // Besides the descendants of pid, this takes every process carrying the
    // same profile marker: wineserver detaches from the tree, and a game
    // adopted after a restart is not the root of what was launched.
    const QString marker = environmentValue(pid, ProfileMarker);
    QList<qint64> processes{pid};
    for (qsizetype stopped = 0; stopped < processes.size();) {
        for (; stopped < processes.size(); ++stopped) {
            ::kill(static_cast<pid_t>(processes.at(stopped)), SIGSTOP);
        }
        const auto children = processChildren();
        for (qsizetype i = 0; i < processes.size(); ++i) {
            for (const qint64 child : children.values(processes.at(i))) {
                if (!processes.contains(child)) {
                    processes << child;
                }
            }
        }
        if (marker.isEmpty()) {
            continue;
        }
        for (const qint64 process : processIds()) {
            if (!processes.contains(process) && environmentValue(process, ProfileMarker) == marker) {
                processes << process;
            }
        }
    }

    qCInfo(LOG_GAME) << "killing" << pid << "and" << processes.size() - 1 << "related processes";
    for (const qint64 process : std::as_const(processes)) {
        ::kill(static_cast<pid_t>(process), SIGKILL);
    }
}
}

LinuxGameLauncher::LinuxGameLauncher(QObject *parent)
    : GameLauncher(parent)
    , m_network(new QNetworkAccessManager(this))
{
    m_network->setTransferTimeout(QNetworkRequest::DefaultTransferTimeout);
    if (KWinWindowPositions::isSupported()) {
        m_kwinWindowPositions = new KWinWindowPositions(this);
    }
}

void LinuxGameLauncher::launch(Profile *profile)
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
        launchSteam(profile, *arguments);
        return;
    }

    const auto executable = gameExecutable(profile);
    if (!executable) {
        Q_EMIT failed(profile, executable.error());
        return;
    }
    if (umuRun().isEmpty()) {
        Q_EMIT failed(profile, i18nc("@info", "Could not find umu-run. Is umu-launcher installed?"));
        return;
    }
    if (D2RLoaderConfig::self()->wineprefixPath().isEmpty()) {
        Q_EMIT failed(profile, i18nc("@info", "No wineprefix location is configured. Set it in the application settings."));
        return;
    }
    if (!QDir().mkpath(profile->wineprefix())) {
        Q_EMIT failed(profile, i18nc("@info", "Could not create the wineprefix \"%1\".", profile->wineprefix()));
        return;
    }

    // d2rreg is needed by every umu launch to rename the window, so it is
    // fetched up front, where a failure can still be reported as one.
    fetchTokenTool(profile, [this, profile = QPointer(profile), executable = *executable, arguments = *arguments] {
        if (!profile) {
            return;
        }
        if (profile->authMethod() == AuthMethodModel::Token) {
            createPrefix(profile, [this, profile, executable, arguments] {
                runTokenTool(profile, [this, profile, executable, arguments] {
                    if (profile) {
                        launchUmu(profile, executable, arguments);
                    }
                });
            });
            return;
        }
        launchUmu(profile, executable, arguments);
    });
}

void LinuxGameLauncher::terminate(qint64 pid)
{
    if (m_terminating.contains(pid)) {
        return;
    }

    // A game that is still starting may have no wineserver yet, and wineboot
    // would start one of its own.
    const QString marker = environmentValue(pid, ProfileMarker);
    const qint64 wineserver = marker.isEmpty() ? 0 : wineserverProcess(marker);
    const QProcessEnvironment env = wineEnvironment(wineserver);
    if (env.value(QStringLiteral("WINEPREFIX")).isEmpty()) {
        killProcessTree(pid);
        return;
    }
    m_terminating.insert(pid);

    // wineboot sends WM_QUERYENDSESSION and WM_ENDSESSION to every window, then
    // terminates what is left. For a window that does not answer it waits on
    // the user, so wineserver -k takes over once EndSessionTimeout has passed.
    auto *process = new QProcess(this);
    process->setProgram(wineBinary(wineserver, QStringLiteral("wine")));
    process->setArguments({QStringLiteral("wineboot"), QStringLiteral("--end-session"), QStringLiteral("--kill"), QStringLiteral("--shutdown")});
    process->setProcessEnvironment(env);

    const auto escalated = std::make_shared<bool>(false);
    const auto escalate = [this, pid, marker, escalated] {
        if (*escalated) {
            return;
        }
        *escalated = true;
        if (environmentValue(pid, ProfileMarker) != marker) {
            qCInfo(LOG_GAME) << "pid" << pid << "ended with its session";
            return;
        }
        killWineserver(pid, marker);
    };
    connect(process, &QProcess::errorOccurred, this, [process, escalate](QProcess::ProcessError error) {
        if (error != QProcess::FailedToStart) {
            return;
        }
        qCWarning(LOG_GAME) << "could not run wineboot --end-session:" << process->errorString();
        process->deleteLater();
        escalate();
    });
    // umu-run and pressure-vessel exit shortly after the game's wine processes.
    connect(process, &QProcess::finished, this, [this, process, escalate](int exitCode) {
        process->deleteLater();
        if (exitCode != 0) {
            qCWarning(LOG_GAME) << "wineboot --end-session exited with" << exitCode;
        }
        QTimer::singleShot(ShutdownGrace, this, escalate);
    });
    QTimer::singleShot(EndSessionTimeout, this, escalate);

    qCInfo(LOG_GAME) << "ending the session of pid" << pid << "in" << env.value(QStringLiteral("WINEPREFIX"));
    process->start();
}

void LinuxGameLauncher::killWineserver(qint64 pid, const QString &marker)
{
    // wineserver -k ends every process in the wineprefix through wine and saves
    // the registry, which a SIGKILL would lose.
    const qint64 wineserver = wineserverProcess(marker);
    const QProcessEnvironment env = wineEnvironment(wineserver);
    const QString prefix = env.value(QStringLiteral("WINEPREFIX"));
    if (prefix.isEmpty()) {
        killProcessTree(pid);
        return;
    }

    auto *process = new QProcess(this);
    process->setProgram(wineBinary(wineserver, QStringLiteral("wineserver")));
    process->setArguments({QStringLiteral("-k")});
    process->setProcessEnvironment(env);

    // The marker check keeps a reused pid from being killed.
    const auto killLeftovers = [pid, marker] {
        if (environmentValue(pid, ProfileMarker) == marker) {
            qCWarning(LOG_GAME) << "pid" << pid << "outlived wineserver -k";
            killProcessTree(pid);
        }
    };
    connect(process, &QProcess::errorOccurred, this, [process, killLeftovers](QProcess::ProcessError error) {
        if (error != QProcess::FailedToStart) {
            return;
        }
        qCWarning(LOG_GAME) << "could not run wineserver -k:" << process->errorString();
        process->deleteLater();
        killLeftovers();
    });
    connect(process, &QProcess::finished, this, [this, process, killLeftovers](int exitCode) {
        process->deleteLater();
        if (exitCode != 0) {
            qCWarning(LOG_GAME) << "wineserver -k exited with" << exitCode;
        }
        QTimer::singleShot(ShutdownGrace, this, killLeftovers);
    });

    qCInfo(LOG_GAME) << "terminating" << pid << "with" << process->program() << "-k in" << prefix;
    process->start();
}

void LinuxGameLauncher::adopt(const QList<Profile *> &profiles)
{
    const QList<qint64> pids = processIds();
    for (const qint64 pid : pids) {
        if (!isGameProcess(pid)) {
            continue;
        }

        const QString marker = environmentValue(pid, ProfileMarker);
        if (marker.isEmpty()) {
            continue;
        }
        for (Profile *profile : profiles) {
            if (profile->normalizedName() == marker) {
                qCInfo(LOG_GAME) << "found" << profile->profileName() << "still running as pid" << pid;
                Q_EMIT launched(profile, pid);
                watch(pid);
                if (!m_kwinWindowPositions && profile->rememberWindowPosition()) {
                    watchWindowPosition(profile);
                }
                break;
            }
        }
    }
}

void LinuxGameLauncher::launchSteam(Profile *profile, const QStringList &arguments)
{
    // Inside the Flatpak the host's Steam is only reachable through its URL handler.
    if (QFileInfo::exists(QStringLiteral("/.flatpak-info"))) {
        const QUrl url(QStringLiteral("steam://run/%1//%2/").arg(SteamAppId, arguments.join(u' ')));
        qCInfo(LOG_GAME) << "launching" << profile->profileName() << "through" << url;
        if (!QDesktopServices::openUrl(url)) {
            Q_EMIT failed(profile, i18nc("@info", "Could not start Steam."));
            return;
        }
        Q_EMIT launched(profile, 0);
        return;
    }

    const QString steam = QStandardPaths::findExecutable(QStringLiteral("steam"));
    if (steam.isEmpty()) {
        Q_EMIT failed(profile, i18nc("@info", "Could not find steam. Is Steam installed?"));
        return;
    }

    const QStringList steamArguments = QStringList{QStringLiteral("-applaunch"), SteamAppId} + arguments;
    qCInfo(LOG_GAME) << "launching" << profile->profileName() << "through Steam:" << steam << steamArguments;
    if (!QProcess::startDetached(steam, steamArguments)) {
        Q_EMIT failed(profile, i18nc("@info", "Could not start Steam."));
        return;
    }
    Q_EMIT launched(profile, 0);
}

void LinuxGameLauncher::launchUmu(Profile *profile, const QString &executable, const QStringList &arguments)
{
    QStringList command = QStringList{umuRun(), executable} + arguments;
    const QString gamemode = QStandardPaths::findExecutable(QStringLiteral("gamemoderun"));
    if (gamemode.isEmpty()) {
        qCDebug(LOG_GAME) << "gamemoderun not found, starting without it";
    } else {
        command.prepend(gamemode);
    }

    // stdout and stderr open the log separately, so both append to keep
    // either from overwriting the other.
    const QString log = QDir(profile->wineprefix()).filePath(QStringLiteral("umu.log"));
    QFile::remove(log);

    QProcess process;
    process.setProgram(command.takeFirst());
    process.setArguments(command);
    process.setProcessEnvironment(environment(profile));
    process.setWorkingDirectory(QFileInfo(executable).absolutePath());
    process.setStandardOutputFile(log, QIODevice::Append);
    process.setStandardErrorFile(log, QIODevice::Append);

    qCInfo(LOG_GAME) << "launching" << profile->profileName() << "in" << profile->wineprefix() << ":" << process.program() << redacted(command);

    qint64 pid = 0;
    if (!process.startDetached(&pid)) {
        Q_EMIT failed(profile, i18nc("@info", "Could not start %1: %2", process.program(), process.errorString()));
        return;
    }

    qCInfo(LOG_GAME) << "started" << profile->profileName() << "as pid" << pid << "- output goes to" << log;
    Q_EMIT launched(profile, pid);
    watch(pid);
    renameWindow(profile, pid, RenameAttempts);
}

void LinuxGameLauncher::fetchTokenTool(Profile *profile, const std::function<void()> &next)
{
    if (QFileInfo::exists(tokenToolPath())) {
        next();
        return;
    }

    qCInfo(LOG_GAME) << "downloading d2rreg" << TokenToolVersion << "from" << TokenToolUrl;
    QNetworkReply *reply = m_network->get(QNetworkRequest(TokenToolUrl));
    connect(reply, &QNetworkReply::finished, this, [this, reply, profile = QPointer(profile), next] {
        reply->deleteLater();
        if (!profile) {
            return;
        }

        if (reply->error() != QNetworkReply::NoError) {
            Q_EMIT failed(profile, i18nc("@info", "Could not download d2rreg.exe: %1", reply->errorString()));
            return;
        }

        QSaveFile file(tokenToolPath());
        if (!QDir().mkpath(QFileInfo(tokenToolPath()).absolutePath()) || !file.open(QIODevice::WriteOnly) || file.write(reply->readAll()) < 0
            || !file.commit()) {
            Q_EMIT failed(profile, i18nc("@info", "Could not save d2rreg.exe to \"%1\": %2", tokenToolPath(), file.errorString()));
            return;
        }

        qCInfo(LOG_GAME) << "saved d2rreg to" << tokenToolPath();
        next();
    });
}

void LinuxGameLauncher::createPrefix(Profile *profile, const std::function<void()> &next)
{
    // d2rreg runs with runinprefix, which skips Proton's prefix setup and fails
    // on a prefix Proton has not set up yet. Proton writes the version file
    // last, once the prefix is complete.
    const QString version = QDir(profile->wineprefix()).filePath(QStringLiteral("version"));
    if (QFileInfo::exists(version)) {
        next();
        return;
    }

    auto *process = new QProcess(this);
    process->setProgram(umuRun());
    process->setArguments({QStringLiteral("createprefix")});
    process->setProcessEnvironment(environment(profile));

    connect(process, &QProcess::errorOccurred, this, [this, process, profile = QPointer(profile)](QProcess::ProcessError error) {
        if (error != QProcess::FailedToStart) {
            return;
        }
        process->deleteLater();
        if (profile) {
            Q_EMIT failed(profile, i18nc("@info", "Could not run umu-run: %1", process->errorString()));
        }
    });
    connect(process, &QProcess::finished, this, [this, process, profile = QPointer(profile), next, version](int exitCode) {
        process->deleteLater();
        if (!profile) {
            return;
        }

        // umu-run exits with 1 even when the prefix was created, as there is
        // nothing to run.
        if (!QFileInfo::exists(version)) {
            qCWarning(LOG_GAME) << "umu-run createprefix exited with" << exitCode << QString::fromLocal8Bit(process->readAllStandardError()).trimmed();
            Q_EMIT failed(profile, i18nc("@info", "Could not create the wineprefix \"%1\".", profile->wineprefix()));
            return;
        }

        qCInfo(LOG_GAME) << "created the wineprefix" << profile->wineprefix();
        next();
    });

    qCInfo(LOG_GAME) << "creating the wineprefix" << profile->wineprefix() << "for" << profile->profileName();
    process->start();
}

void LinuxGameLauncher::runTokenTool(Profile *profile, const std::function<void()> &next)
{
    // Same Proton as the game itself, so the prefix is never touched by two
    // different versions.
    QProcessEnvironment env = environment(profile);
    env.insert(QStringLiteral("PROTON_VERB"), QStringLiteral("runinprefix"));

    auto *process = new QProcess(this);
    process->setProgram(umuRun());
    process->setArguments({tokenToolPath(), QStringLiteral("--update-token"), profile->token()});
    process->setProcessEnvironment(env);

    connect(process, &QProcess::errorOccurred, this, [this, process, profile = QPointer(profile)](QProcess::ProcessError error) {
        // Every other error is followed by finished(), which reports it.
        if (error != QProcess::FailedToStart) {
            return;
        }
        process->deleteLater();
        if (profile) {
            Q_EMIT failed(profile, i18nc("@info", "Could not run d2rreg.exe: %1", process->errorString()));
        }
    });
    connect(process, &QProcess::finished, this, [this, process, profile = QPointer(profile), next](int exitCode, QProcess::ExitStatus status) {
        process->deleteLater();
        if (!profile) {
            return;
        }

        if (status != QProcess::NormalExit || exitCode != 0) {
            const QString output = QString::fromLocal8Bit(process->readAllStandardError()).trimmed();
            qCWarning(LOG_GAME) << "d2rreg exited with" << exitCode << output;
            Q_EMIT failed(profile, i18nc("@info", "Could not write the token into the wineprefix (d2rreg.exe exited with %1).", exitCode));
            return;
        }

        qCDebug(LOG_GAME) << "token written for" << profile->profileName();
        next();
    });

    qCInfo(LOG_GAME) << "writing the token for" << profile->profileName() << "into" << profile->wineprefix();
    process->start();
}

void LinuxGameLauncher::renameWindow(Profile *profile, qint64 pid, int attemptsLeft)
{
    const QString title = profile->windowTitle();

    // Stop once the game is gone.
    const auto retry = [this, profile = QPointer(profile), pid, attemptsLeft, title] {
        if (!profile || ::kill(static_cast<pid_t>(pid), 0) != 0) {
            return;
        }
        if (attemptsLeft <= 1) {
            qCWarning(LOG_GAME) << "gave up renaming the window of pid" << pid << "to" << title;
            return;
        }
        QTimer::singleShot(RenameInterval, this, [this, profile, pid, attemptsLeft] {
            if (profile) {
                renameWindow(profile, pid, attemptsLeft - 1);
            }
        });
    };

    // d2rreg runs next to the game's wineserver, which shows up a moment after
    // umu-run starts.
    const qint64 wineserver = wineserverProcess(profile->normalizedName());
    const QProcessEnvironment env = wineEnvironment(wineserver);
    if (env.value(QStringLiteral("WINEPREFIX")).isEmpty()) {
        retry();
        return;
    }

    QProcess *process = tokenToolProcess(wineserver, {QStringLiteral("--rename-window"), title}, this);

    connect(process, &QProcess::errorOccurred, this, [process](QProcess::ProcessError error) {
        if (error == QProcess::FailedToStart) {
            qCWarning(LOG_GAME) << "could not run d2rreg to rename the window:" << process->errorString();
            process->deleteLater();
        }
    });
    connect(process, &QProcess::finished, this, [this, process, profile = QPointer(profile), pid, title, retry](int exitCode, QProcess::ExitStatus status) {
        process->deleteLater();
        if (status == QProcess::NormalExit && exitCode == 0) {
            qCInfo(LOG_GAME) << "renamed the window of pid" << pid << "to" << title;
            // On Plasma the KWin script takes over once the window carries its title.
            if (profile && !m_kwinWindowPositions && profile->rememberWindowPosition()) {
                restoreWindowPosition(profile);
            }
            return;
        }
        // d2rreg exits with 1 until the window exists.
        retry();
    });

    process->start();
}

void LinuxGameLauncher::restoreWindowPosition(Profile *profile)
{
    const std::optional<QPoint> position = profile->windowPosition();
    if (!position) {
        watchWindowPosition(profile);
        return;
    }

    const QStringList arguments{QStringLiteral("--move-window"), profile->windowTitle(), QString::number(position->x()), QString::number(position->y())};
    QProcess *process = tokenToolProcess(wineserverProcess(profile->normalizedName()), arguments, this);

    connect(process, &QProcess::errorOccurred, this, [process](QProcess::ProcessError error) {
        if (error == QProcess::FailedToStart) {
            qCWarning(LOG_GAME) << "could not run d2rreg to move the window:" << process->errorString();
            process->deleteLater();
        }
    });
    connect(process, &QProcess::finished, this, [this, process, profile = QPointer(profile)](int exitCode) {
        process->deleteLater();
        const QString output = QString::fromLocal8Bit(process->readAllStandardOutput() + process->readAllStandardError()).trimmed();
        qCInfo(LOG_GAME) << "d2rreg --move-window exited with" << exitCode << output;
        if (profile) {
            watchWindowPosition(profile);
        }
    });

    process->start();
}

void LinuxGameLauncher::watchWindowPosition(Profile *profile)
{
    const QString title = profile->windowTitle();
    QProcess *process = tokenToolProcess(wineserverProcess(profile->normalizedName()), {QStringLiteral("--watch-window"), title}, this);

    // d2rreg prints "x y" whenever the window settles somewhere new, and exits
    // once the window is gone.
    connect(process, &QProcess::readyReadStandardOutput, this, [process, profile = QPointer(profile)] {
        while (process->canReadLine()) {
            const QList<QByteArray> fields = process->readLine().trimmed().split(' ');
            bool isX = false;
            bool isY = false;
            const QPoint position(fields.value(0).toInt(&isX), fields.value(1).toInt(&isY));
            if (profile && fields.size() == 2 && isX && isY) {
                profile->setWindowPosition(position);
            }
        }
    });
    connect(process, &QProcess::errorOccurred, this, [process](QProcess::ProcessError error) {
        if (error == QProcess::FailedToStart) {
            qCWarning(LOG_GAME) << "could not run d2rreg to follow the window:" << process->errorString();
            process->deleteLater();
        }
    });
    connect(process, &QProcess::finished, this, [process, title](int exitCode) {
        process->deleteLater();
        qCDebug(LOG_GAME) << "stopped following the window" << title << "- d2rreg exited with" << exitCode;
    });

    qCDebug(LOG_GAME) << "following the window" << title;
    process->start();
}

void LinuxGameLauncher::watch(qint64 pid)
{
    const int fd = static_cast<int>(::syscall(SYS_pidfd_open, static_cast<pid_t>(pid), 0));
    if (fd < 0) {
        // Most likely the process is already gone.
        qCWarning(LOG_GAME) << "cannot watch pid" << pid << ":" << strerror(errno);
        Q_EMIT finished(pid);
        return;
    }

    // A pidfd turns readable once the process exits.
    auto *notifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
    connect(notifier, &QSocketNotifier::activated, this, [this, notifier, fd, pid] {
        notifier->setEnabled(false);
        notifier->deleteLater();
        ::close(fd);
        qCInfo(LOG_GAME) << "pid" << pid << "exited";
        m_terminating.remove(pid);
        Q_EMIT finished(pid);
    });
}

QProcessEnvironment LinuxGameLauncher::environment(const Profile *profile) const
{
    const QString prefix = profile->wineprefix();
    QString protonPath = profile->protonPath();
    if (protonPath.isEmpty()) {
        protonPath = D2RLoaderConfig::self()->protonPath();
    }
    if (protonPath.isEmpty()) {
        protonPath = DefaultProtonPath;
    }

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert(QStringLiteral("__GL_SHADER_DISK_CACHE"), QStringLiteral("1"));
    env.insert(QStringLiteral("__GL_SHADER_DISK_CACHE_PATH"), prefix);
    env.insert(QStringLiteral("STAGING_SHARED_MEMORY"), QStringLiteral("1"));
    env.insert(QStringLiteral("DXVK_STATE_CACHE_PATH"), prefix);
    env.insert(QStringLiteral("DXVK_LOG_LEVEL"), QStringLiteral("error"));
    env.insert(QStringLiteral("DXVK_NVAPIHACK"), QStringLiteral("0"));
    env.insert(QStringLiteral("DXVK_ENABLE_NVAPI"), QStringLiteral("1"));
    env.insert(QStringLiteral("PROTON_DXVK_D3D8"), QStringLiteral("1"));
    env.insert(QStringLiteral("UMU_LOG"), QStringLiteral("0"));
    env.insert(QStringLiteral("WINEDEBUG"), QStringLiteral("-all"));
    env.insert(QStringLiteral("WINE_LARGE_ADDRESS_AWARE"), QStringLiteral("1"));
    env.insert(QStringLiteral("WINEARCH"), QStringLiteral("win64"));
    env.insert(QStringLiteral("WINEESYNC"), QStringLiteral("1"));
    env.insert(QStringLiteral("WINEFSYNC"), QStringLiteral("1"));
    env.insert(QStringLiteral("WINEDLLOVERRIDES"), QStringLiteral("winemenubuilder="));
    env.insert(QStringLiteral("PROTONPATH"), protonPath);

    const QStringList lines = profile->environmentVariables().split(u'\n');
    for (const QString &line : lines) {
        const QString variable = line.trimmed();
        const qsizetype separator = variable.indexOf(u'=');
        if (separator > 0) {
            env.insert(variable.left(separator).trimmed(), variable.mid(separator + 1).trimmed());
        } else if (!variable.isEmpty()) {
            qCWarning(LOG_GAME) << "ignoring environment line without NAME=value:" << variable;
        }
    }

    // Stopping and adopting the game rely on these, so they cannot be overridden.
    env.insert(QStringLiteral("WINEPREFIX"), prefix);
    env.insert(ProfileMarker, profile->normalizedName());
    return env;
}
