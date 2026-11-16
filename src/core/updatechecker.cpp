#include "updatechecker.h"
#include "d2rloaderconfig.h"
#include "logging.h"
#include "verifieddownload.h"

#include <KLocalizedString>
#include <KNotification>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QRegularExpression>
#include <QSettings>
#include <QStandardPaths>
#include <QTimer>
#include <QUrl>
#include <QVersionNumber>

#include <chrono>

#include <d2rloader-version.h>

namespace
{
/**
 * The releases/latest endpoint reports the newest release that is neither a
 * draft nor a pre-release, which is what the check should offer.
 */
const QUrl LatestReleaseUrl = QUrl(QStringLiteral("https://api.github.com/repos/sh4nks/d2rloader2/releases/latest"));

/**
 * The GitHub API answers 403 to a request without a user agent.
 */
const QString UserAgent = QStringLiteral("D2RLoader (https://github.com/sh4nks/d2rloader2)");

/**
 * How long a check's result stands before the startup check asks again.
 */
constexpr std::chrono::hours CheckInterval(1);

QVersionNumber versionOf(const QString &tag)
{
    return QVersionNumber::fromString(tag.startsWith(QLatin1Char('v')) ? tag.sliced(1) : tag);
}

/**
 * Whether another updater owns this installation. The Flatpak is updated from
 * its repo and a packaged build by the distribution's package manager, so a
 * release download would be the wrong way to update either. An AppImage, both
 * Windows builds and a build from source have no such updater.
 */
bool installedByAnotherUpdater()
{
#ifdef Q_OS_LINUX
    if (QFile::exists(QStringLiteral("/.flatpak-info"))) {
        return true;
    }
    if (qEnvironmentVariableIsSet("APPIMAGE")) {
        return false;
    }
    return QCoreApplication::applicationDirPath().startsWith(QStringLiteral("/usr/"));
#else
    return false;
#endif
}

/**
 * The command line tool of AppImageUpdate, which fetches the blocks of the
 * next release that differ from this AppImage. It is distributed as an
 * AppImage itself, so the name it is published under is looked up too, on
 * PATH and in ~/Applications, where AppImageLauncher keeps AppImages. Empty
 * outside an AppImage.
 */
QString appImageUpdateTool()
{
    if (!qEnvironmentVariableIsSet("APPIMAGE")) {
        return QString();
    }
    QStringList paths = qEnvironmentVariable("PATH").split(QDir::listSeparator(), Qt::SkipEmptyParts);
    paths << QDir::home().filePath(QStringLiteral("Applications"));
    for (const QString &name : {QStringLiteral("appimageupdatetool"), QStringLiteral("appimageupdatetool-x86_64.AppImage")}) {
        const QString tool = QStandardPaths::findExecutable(name, paths);
        if (!tool.isEmpty()) {
            return tool;
        }
    }
    return QString();
}

/**
 * The last two lines appimageupdatetool printed, without the escape sequence
 * and the carriage returns its progress line is redrawn with. The tool ends
 * a failure with a verdict like "Update failed!" and the reason is the status
 * line before it.
 */
QString updaterFailure(QString output)
{
    output.remove(QStringLiteral("\x1b[2K"));
    static const QRegularExpression lineBreak(QStringLiteral("[\r\n]"));
    const QStringList lines = output.split(lineBreak, Qt::SkipEmptyParts);
    return lines.mid(lines.size() - 2).join(QLatin1Char(' ')).simplified();
}

QNetworkRequest releaseRequest()
{
    QNetworkRequest request(LatestReleaseUrl);
    request.setHeader(QNetworkRequest::UserAgentHeader, UserAgent);
    request.setRawHeader("Accept", "application/vnd.github+json");
    request.setRawHeader("X-GitHub-Api-Version", "2022-11-28");
    return request;
}

/**
 * Where the downloaded installer is kept. It is still running when it starts
 * D2RLoader again, so the directory is emptied on the start after that one.
 */
QString updateDir()
{
    return QDir::temp().filePath(QStringLiteral("D2RLoader-update"));
}
}

UpdateChecker::UpdateChecker(QObject *parent)
    : QObject(parent)
    , m_network(new QNetworkAccessManager(this))
{
    // Without a timeout a stalled request keeps checking() set, which blocks
    // every later check.
    m_network->setTransferTimeout(QNetworkRequest::DefaultTransferTimeout);

#ifdef Q_OS_WIN
    if (canInstall()) {
        QDir(updateDir()).removeRecursively();
    }
#endif

    if (managedExternally()) {
        qCInfo(LOG_UPDATES) << "Another updater owns this installation, no release check";
        return;
    }

    // Deferred so it does not run while the singleton is still being constructed.
    QTimer::singleShot(0, this, &UpdateChecker::check);
}

std::optional<UpdateChecker::Installer> UpdateChecker::windowsInstaller(const QJsonObject &release)
{
    const QVersionNumber version = versionOf(release.value(QLatin1String("tag_name")).toString());
    if (version.isNull()) {
        return std::nullopt;
    }

    const QJsonArray assets = release.value(QLatin1String("assets")).toArray();
    for (const QJsonValue &value : assets) {
        const QJsonObject asset = value.toObject();
        if (!asset.value(QLatin1String("name")).toString().endsWith(QLatin1String("-windows-x86_64-setup.exe"))) {
            continue;
        }

        const QUrl url(asset.value(QLatin1String("browser_download_url")).toString());
        if (url.scheme() != QLatin1String("https")) {
            return std::nullopt;
        }

        // "sha256:" followed by the 64 hex digits. fromHex() skips what is
        // not a hex digit, so only the decoded length tells they all were.
        const QString digest = asset.value(QLatin1String("digest")).toString();
        const QLatin1String algorithm("sha256:");
        if (!digest.startsWith(algorithm) || digest.size() != algorithm.size() + 64) {
            return std::nullopt;
        }
        const QByteArray sha256 = QByteArray::fromHex(digest.sliced(algorithm.size()).toLatin1());
        if (sha256.size() != 32) {
            return std::nullopt;
        }

        return Installer{version.toString(), url, sha256};
    }
    return std::nullopt;
}

bool UpdateChecker::runsFromInstalledCopy()
{
#ifdef Q_OS_WIN
    const QSettings uninstall(
        QStringLiteral("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{59BA45B2-25D4-49E8-B8FC-E48986E500AB}_is1"),
        QSettings::NativeFormat);
    const QString installed = QDir::cleanPath(QDir::fromNativeSeparators(uninstall.value(QStringLiteral("InstallLocation")).toString()));
    return installed.compare(QDir::cleanPath(QCoreApplication::applicationDirPath()), Qt::CaseInsensitive) == 0;
#else
    return false;
#endif
}

bool UpdateChecker::managedExternally() const
{
    static const bool managed = installedByAnotherUpdater();
    return managed;
}

bool UpdateChecker::canInstall() const
{
#ifdef Q_OS_LINUX
    static const bool installed = !appImageUpdateTool().isEmpty();
#else
    static const bool installed = runsFromInstalledCopy();
#endif
    return installed;
}

bool UpdateChecker::installing() const
{
    return m_installing;
}

QString UpdateChecker::installStatus() const
{
    return m_installStatus;
}

QString UpdateChecker::currentVersion()
{
    return QStringLiteral(D2RLOADER_VERSION_STRING);
}

QString UpdateChecker::latestVersion() const
{
    return D2RLoaderConfig::self()->latestVersion();
}

QString UpdateChecker::releaseUrl() const
{
    return D2RLoaderConfig::self()->latestReleaseUrl();
}

bool UpdateChecker::updateAvailable() const
{
    const QVersionNumber latest = versionOf(latestVersion());
    return !latest.isNull() && latest > versionOf(currentVersion());
}

bool UpdateChecker::dismissed() const
{
    return !latestVersion().isEmpty() && D2RLoaderConfig::self()->dismissedVersion() == latestVersion();
}

void UpdateChecker::dismiss()
{
    D2RLoaderConfig::self()->setDismissedVersion(latestVersion());
    D2RLoaderConfig::self()->save();
    Q_EMIT dismissedChanged();
    Q_EMIT statusTextChanged();
}

bool UpdateChecker::checking() const
{
    return m_checking;
}

QString UpdateChecker::statusText() const
{
    if (m_checking) {
        return i18nc("@info:status", "Checking for updates...");
    }
    if (!m_error.isEmpty()) {
        return m_error;
    }
    if (updateAvailable()) {
        return i18nc("@info:status new version", "Version %1 is available.", latestVersion());
    }
    const QDateTime last = D2RLoaderConfig::self()->lastUpdateCheck();
    if (!last.isValid()) {
        return i18nc("@info:status", "This build has not been compared against the releases yet.");
    }
    return i18nc("@info:status when the last check ran", "Up to date, last checked %1.", QLocale().toString(last.toLocalTime(), QLocale::ShortFormat));
}

void UpdateChecker::check()
{
    if (managedExternally() || !D2RLoaderConfig::self()->checkForUpdates()) {
        return;
    }

    const QDateTime last = D2RLoaderConfig::self()->lastUpdateCheck();
    const qint64 interval = std::chrono::seconds(CheckInterval).count();
    const qint64 elapsed = last.secsTo(QDateTime::currentDateTimeUtc());
    if (last.isValid() && elapsed < interval) {
        const QString format = QStringLiteral("yyyy-MM-dd HH:mm:ss");
        qCDebug(LOG_UPDATES).noquote() << "Skipping the release check, the last one ran at" << last.toLocalTime().toString(format);
        qCDebug(LOG_UPDATES).noquote() << "Next release check is due at" << last.addSecs(interval).toLocalTime().toString(format) << "- in"
                                       << (interval - elapsed + 59) / 60 << "minutes";
        return;
    }

    checkNow();
}

void UpdateChecker::checkNow()
{
    if (m_checking) {
        return;
    }

    QNetworkRequest request = releaseRequest();

    const QString etag = D2RLoaderConfig::self()->updateEtag();
    if (!etag.isEmpty()) {
        request.setRawHeader("If-None-Match", etag.toUtf8());
    }

    qCInfo(LOG_UPDATES) << "Checking for a release newer than" << currentVersion();
    setError(QString());
    setChecking(true);

    QNetworkReply *reply = m_network->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        handle(reply);
        Q_EMIT checkFinished();
    });
}

void UpdateChecker::handle(QNetworkReply *reply)
{
    reply->deleteLater();
    setChecking(false);

    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    // Nothing was published since the last check, so its stored result stands.
    if (status == 304) {
        qCInfo(LOG_UPDATES) << "The newest release has not changed since the last check";
        D2RLoaderConfig::self()->setLastUpdateCheck(QDateTime::currentDateTimeUtc());
        D2RLoaderConfig::self()->save();
        Q_EMIT statusTextChanged();
        return;
    }

    if (reply->error() != QNetworkReply::NoError || status != 200) {
        qCWarning(LOG_UPDATES) << "release check failed" << status << reply->errorString();
        if (status == 403 || status == 429) {
            // What GitHub answers an unauthenticated client that asked too often.
            setError(i18nc("@info", "GitHub is rate limiting the update check. Try again later."));
        } else if (reply->error() == QNetworkReply::OperationCanceledError) {
            // The transfer timeout aborts the reply, which reports "Operation canceled".
            setError(i18nc("@info", "GitHub did not respond in time."));
        } else {
            setError(i18nc("@info", "Could not check for updates: %1", reply->errorString()));
        }
        return;
    }

    const QJsonObject release = QJsonDocument::fromJson(reply->readAll()).object();
    const QString tag = release.value(QLatin1String("tag_name")).toString();
    const QVersionNumber latest = versionOf(tag);
    if (latest.isNull()) {
        qCWarning(LOG_UPDATES) << "the newest release carries no version in its tag" << tag;
        setError(i18nc("@info", "The newest release does not report a version."));
        return;
    }

    const QString version = latest.toString();
    const bool firstSighting = version != latestVersion();

    D2RLoaderConfig::self()->setLatestVersion(version);
    D2RLoaderConfig::self()->setLatestReleaseUrl(release.value(QLatin1String("html_url")).toString());
    D2RLoaderConfig::self()->setUpdateEtag(QString::fromUtf8(reply->rawHeader("ETag")));
    D2RLoaderConfig::self()->setLastUpdateCheck(QDateTime::currentDateTimeUtc());
    D2RLoaderConfig::self()->save();

    Q_EMIT latestReleaseChanged();
    Q_EMIT dismissedChanged();
    Q_EMIT statusTextChanged();

    qCInfo(LOG_UPDATES) << "Newest release is" << version << "and this build is" << currentVersion();

    // The banner carries a release that was already reported, so the
    // notification only fires the first time a release is seen.
    if (firstSighting && updateAvailable() && !dismissed()) {
        notify(version);
    }
}

void UpdateChecker::notify(const QString &version)
{
    const QString title = i18nc("@title:window", "Update Available");
    const QString text = i18nc("@info notification text, version", "D2RLoader %1 is available.", version);
#ifdef Q_OS_WIN
    // The MSYS2 KNotifications has no popup backend and crashes on a Popup
    // event, so the tray icon shows the native toast instead.
    Q_EMIT notificationRequested(title, text);
#else
    auto *notification = new KNotification(QStringLiteral("update"));
    notification->setTitle(title);
    notification->setText(text);

    if (canInstall()) {
        KNotificationAction *install = notification->addAction(i18nc("@action:button", "Install"));
        connect(install, &KNotificationAction::activated, this, &UpdateChecker::installUpdate);
    } else {
        const QUrl url(releaseUrl());
        KNotificationAction *download = notification->addAction(i18nc("@action:button", "Download"));
        connect(download, &KNotificationAction::activated, this, [url] {
            QDesktopServices::openUrl(url);
        });
    }

    notification->sendEvent();
#endif
}

void UpdateChecker::setError(const QString &error)
{
    if (m_error == error) {
        return;
    }
    m_error = error;
    Q_EMIT statusTextChanged();
}

void UpdateChecker::setChecking(bool checking)
{
    if (m_checking == checking) {
        return;
    }
    m_checking = checking;
    Q_EMIT checkingChanged();
    Q_EMIT statusTextChanged();
}

void UpdateChecker::installUpdate()
{
    if (!canInstall() || m_installing) {
        return;
    }

    m_installCancelled = false;
    setInstalling(true);

#ifdef Q_OS_LINUX
    updateAppImage();
#else
    setInstallStatus(i18nc("@info:status", "Looking up the installer..."));

    // The stored result of the check has neither the installer's address nor
    // its digest, and a 304 would not bring them, so no ETag goes out here.
    m_installerLookup = m_network->get(releaseRequest());
    connect(m_installerLookup, &QNetworkReply::finished, this, [this] {
        download(m_installerLookup);
    });
#endif
}

void UpdateChecker::cancelInstall()
{
    m_installCancelled = true;
    if (m_installerLookup) {
        m_installerLookup->abort();
    }
    if (m_download) {
        m_download->cancel();
    }
    if (m_updater) {
        m_updater->kill();
    }
}

void UpdateChecker::download(QNetworkReply *reply)
{
    reply->deleteLater();
    m_installerLookup = nullptr;

    if (reply->error() != QNetworkReply::NoError) {
        failInstall(i18nc("@info", "Could not look up the installer: %1", reply->errorString()));
        return;
    }

    const std::optional<Installer> installer = windowsInstaller(QJsonDocument::fromJson(reply->readAll()).object());
    if (!installer) {
        failInstall(i18nc("@info", "The newest release has no installer that can be verified. Download it from the release page instead."));
        return;
    }
    if (versionOf(installer->version) <= versionOf(currentVersion())) {
        failInstall(i18nc("@info", "The newest release is not newer than this version."));
        return;
    }

    QDir dir(updateDir());
    dir.removeRecursively();
    dir.mkpath(QStringLiteral("."));
    const QString target = dir.filePath(QStringLiteral("D2RLoader-%1-windows-x86_64-setup.exe").arg(installer->version));
    const QString version = installer->version;

    qCInfo(LOG_UPDATES) << "Downloading" << installer->url.toString() << "to" << target;
    setInstallStatus(i18nc("@info:status version", "Downloading D2RLoader %1...", version));

    m_download = new VerifiedDownload(m_network, installer->url, installer->sha256, target, this);
    connect(m_download, &VerifiedDownload::progress, this, [this, version](qint64 received, qint64 total) {
        if (total > 0) {
            setInstallStatus(i18nc("@info:status version, percent", "Downloading D2RLoader %1... %2%", version, received * 100 / total));
        }
    });
    connect(m_download, &VerifiedDownload::finished, this, [this, target](const QString &error) {
        m_download->deleteLater();
        m_download = nullptr;
        if (!error.isEmpty()) {
            failInstall(i18nc("@info", "Could not download the installer: %1", error));
            return;
        }
        launch(target);
    });
}

void UpdateChecker::launch(const QString &installer)
{
    // /SILENT only shows the progress and the errors. RELAUNCH is this
    // application's own switch: d2rloader.iss starts D2RLoader again for it.
    const QStringList arguments{QStringLiteral("/SILENT"), QStringLiteral("/NORESTART"), QStringLiteral("/RELAUNCH=1")};
    if (!QProcess::startDetached(installer, arguments)) {
        failInstall(i18nc("@info", "Could not start the installer."));
        return;
    }

    qCInfo(LOG_UPDATES) << "Started" << installer << "- quitting so it can replace the files";
    setInstallStatus(i18nc("@info:status", "Installing the update..."));
    QCoreApplication::quit();
}

void UpdateChecker::updateAppImage()
{
    const QString appImage = qEnvironmentVariable("APPIMAGE");
    const QString tool = appImageUpdateTool();
    const QString version = latestVersion();

    qCInfo(LOG_UPDATES) << "Updating" << appImage << "with" << tool;
    setInstallStatus(i18nc("@info:status version", "Downloading D2RLoader %1...", version));

    m_updaterOutput.clear();
    m_updater = new QProcess(this);
    m_updater->setProgram(tool);
    // --overwrite renames the finished file over the AppImage, which keeps
    // running from the old one until it quits. --remove-old drops the .zs-old
    // backup that would otherwise be left next to it.
    m_updater->setArguments({QStringLiteral("--overwrite"), QStringLiteral("--remove-old"), appImage});
    m_updater->setProcessChannelMode(QProcess::MergedChannels);

    connect(m_updater, &QProcess::readyRead, this, [this, version] {
        const QString output = QString::fromUtf8(m_updater->readAll());
        m_updaterOutput += output;

        static const QRegularExpression progress(QStringLiteral("(\\d+)(?:\\.\\d+)?% done"));
        QRegularExpressionMatch last;
        for (QRegularExpressionMatchIterator it = progress.globalMatch(output); it.hasNext();) {
            last = it.next();
        }
        if (last.hasMatch()) {
            setInstallStatus(i18nc("@info:status version, percent", "Downloading D2RLoader %1... %2%", version, last.captured(1).toInt()));
        }
    });
    connect(m_updater, &QProcess::errorOccurred, this, [this, tool](QProcess::ProcessError error) {
        if (error != QProcess::FailedToStart) {
            return;
        }
        m_updater->deleteLater();
        m_updater = nullptr;
        failInstall(i18nc("@info path", "Could not start %1.", tool));
    });
    connect(m_updater, &QProcess::finished, this, [this, appImage, version](int exitCode, QProcess::ExitStatus status) {
        m_updater->deleteLater();
        m_updater = nullptr;
        if (status != QProcess::NormalExit || exitCode != 0) {
            if (!m_installCancelled) {
                qCWarning(LOG_UPDATES).noquote().nospace() << "appimageupdatetool exited with " << exitCode << " and printed:\n" << m_updaterOutput;
            }
            QFile::remove(appImage + QStringLiteral(".part"));
            failInstall(i18nc("@info", "Could not update the AppImage: %1", updaterFailure(m_updaterOutput)));
            return;
        }

        qCInfo(LOG_UPDATES) << "Updated" << appImage << "- starting it again and quitting";
        if (!QProcess::startDetached(appImage, {})) {
            failInstall(i18nc("@info version", "D2RLoader %1 is installed, but could not be started. Start it yourself.", version));
            return;
        }
        QCoreApplication::quit();
    });
    m_updater->start();
}

void UpdateChecker::failInstall(const QString &error)
{
    if (m_installCancelled) {
        qCInfo(LOG_UPDATES) << "Update cancelled";
        setInstallStatus(QString());
    } else {
        qCWarning(LOG_UPDATES) << "update failed:" << error;
        setInstallStatus(error);
    }
    setInstalling(false);
}

void UpdateChecker::setInstalling(bool installing)
{
    if (m_installing == installing) {
        return;
    }
    m_installing = installing;
    Q_EMIT installingChanged();
}

void UpdateChecker::setInstallStatus(const QString &status)
{
    if (m_installStatus == status) {
        return;
    }
    m_installStatus = status;
    Q_EMIT installStatusChanged();
}
