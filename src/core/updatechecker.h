#pragma once

#include <QByteArray>
#include <QDateTime>
#include <QObject>
#include <QString>
#include <QUrl>
#include <qqmlintegration.h>

#include <optional>

class QJsonObject;
class QNetworkAccessManager;
class QNetworkReply;
class VerifiedDownload;

/**
 * Checks GitHub for a release newer than this build.
 *
 * Where another updater owns the installation - the Flatpak's own repo, the
 * distribution's package manager for a packaged build - managedExternally is
 * set and no request is ever sent, because the download would be the wrong
 * way to update.
 *
 * A copy the Windows installer put in place can also install the release:
 * installUpdate() downloads the release's installer, runs it and quits. Every
 * other copy only gets the notice and the link to the release page.
 *
 * The result is kept in the configuration, so the notice is right from the
 * first frame after a restart, and the response's ETag goes back out with the
 * next check so an unchanged release answers 304 instead of being transferred
 * again. GitHub does not count a 304 against its rate limit.
 */
class UpdateChecker : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(bool managedExternally READ managedExternally CONSTANT)
    Q_PROPERTY(QString latestVersion READ latestVersion NOTIFY latestReleaseChanged)
    Q_PROPERTY(QString releaseUrl READ releaseUrl NOTIFY latestReleaseChanged)
    Q_PROPERTY(bool updateAvailable READ updateAvailable NOTIFY latestReleaseChanged)
    Q_PROPERTY(bool dismissed READ dismissed NOTIFY dismissedChanged)
    Q_PROPERTY(bool checking READ checking NOTIFY checkingChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(bool canInstall READ canInstall CONSTANT)
    Q_PROPERTY(bool installing READ installing NOTIFY installingChanged)
    Q_PROPERTY(QString installStatus READ installStatus NOTIFY installStatusChanged)

public:
    struct Installer {
        QString version;
        QUrl url;
        QByteArray sha256;
    };

    explicit UpdateChecker(QObject *parent = nullptr);

    /**
     * The Windows installer of a release as the GitHub API describes it, or
     * nothing when the release has none that can be verified: no such asset,
     * no SHA-256 digest for it, or no HTTPS address.
     */
    static std::optional<Installer> windowsInstaller(const QJsonObject &release);

    /**
     * Whether the executable runs from where the Windows installer put it.
     */
    static bool runsFromInstalledCopy();

    bool managedExternally() const;
    bool canInstall() const;
    bool installing() const;

    /**
     * The progress of installUpdate(), or why it failed. Empty before it ran
     * and after it was cancelled.
     */
    QString installStatus() const;
    QString latestVersion() const;
    QString releaseUrl() const;
    bool updateAvailable() const;
    bool dismissed() const;
    bool checking() const;

    /**
     * One line for the settings page: what the last check found, or why it
     * could not find out.
     */
    QString statusText() const;

    /**
     * The version this build reports, as the release tags spell it.
     */
    static QString currentVersion();

    /**
     * Asks GitHub, unless another updater owns the installation, the setting
     * is off, or the last check is too recent to repeat.
     */
    Q_INVOKABLE void check();

    /**
     * The check the settings page runs: neither the interval nor the setting
     * is consulted, because the user asked for it directly.
     */
    Q_INVOKABLE void checkNow();

    /**
     * Stops the notice for the release the last check found. A later release
     * is announced again.
     */
    Q_INVOKABLE void dismiss();

    /**
     * Downloads the installer of the newest release, checks it against the
     * digest GitHub reports for it, starts it and quits, so it can replace
     * the files. The installer starts D2RLoader again when it is done.
     */
    Q_INVOKABLE void installUpdate();
    Q_INVOKABLE void cancelInstall();

Q_SIGNALS:
    void latestReleaseChanged();
    void dismissedChanged();
    void checkingChanged();
    void statusTextChanged();

    /**
     * A check has ended and statusText says how.
     */
    void checkFinished();
    void installingChanged();
    void installStatusChanged();

    /**
     * A desktop notification the window has to show itself. Only emitted on
     * Windows, where KNotification has no popup backend.
     */
    void notificationRequested(const QString &title, const QString &text);

private:
    void handle(QNetworkReply *reply);
    void notify(const QString &version);
    void setError(const QString &error);
    void setChecking(bool checking);
    void download(QNetworkReply *reply);
    void launch(const QString &installer);
    void failInstall(const QString &error);
    void setInstalling(bool installing);
    void setInstallStatus(const QString &status);

    QNetworkAccessManager *m_network;
    QString m_error;
    bool m_checking = false;
    QNetworkReply *m_installerLookup = nullptr;
    VerifiedDownload *m_download = nullptr;
    QString m_installStatus;
    bool m_installing = false;
    bool m_installCancelled = false;
};
