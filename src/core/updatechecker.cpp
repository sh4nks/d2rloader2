#include "updatechecker.h"
#include "d2rloaderconfig.h"
#include "logging.h"

#include <KLocalizedString>
#include <KNotification>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
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
constexpr std::chrono::hours CheckInterval(24);

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
}

UpdateChecker::UpdateChecker(QObject *parent)
    : QObject(parent)
    , m_network(new QNetworkAccessManager(this))
{
    // Without a timeout a stalled request keeps checking() set, which blocks
    // every later check.
    m_network->setTransferTimeout(QNetworkRequest::DefaultTransferTimeout);

    if (managedExternally()) {
        qCInfo(LOG_UPDATES) << "Another updater owns this installation, no release check";
        return;
    }

    // Deferred so it does not run while the singleton is still being constructed.
    QTimer::singleShot(0, this, &UpdateChecker::check);
}

bool UpdateChecker::managedExternally() const
{
    static const bool managed = installedByAnotherUpdater();
    return managed;
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
    if (last.isValid() && last.secsTo(QDateTime::currentDateTimeUtc()) < std::chrono::seconds(CheckInterval).count()) {
        qCDebug(LOG_UPDATES) << "Last release check ran at" << last << "- its result still stands";
        return;
    }

    checkNow();
}

void UpdateChecker::checkNow()
{
    if (m_checking) {
        return;
    }

    QNetworkRequest request(LatestReleaseUrl);
    request.setHeader(QNetworkRequest::UserAgentHeader, UserAgent);
    request.setRawHeader("Accept", "application/vnd.github+json");
    request.setRawHeader("X-GitHub-Api-Version", "2022-11-28");

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

    const QUrl url(releaseUrl());
    KNotificationAction *download = notification->addAction(i18nc("@action:button", "Download"));
    connect(download, &KNotificationAction::activated, this, [url] {
        QDesktopServices::openUrl(url);
    });

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
