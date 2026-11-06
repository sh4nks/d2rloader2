#pragma once

#include <QDateTime>
#include <QObject>
#include <QString>
#include <qqmlintegration.h>

class QNetworkAccessManager;
class QNetworkReply;

/**
 * Checks GitHub for a release newer than this build.
 *
 * Only the check: nothing is downloaded or installed. Where another updater
 * owns the installation - the Flatpak's own repo, the distribution's package
 * manager for a packaged build - managedExternally is set and no request is
 * ever sent, because the download would be the wrong way to update.
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

public:
    explicit UpdateChecker(QObject *parent = nullptr);

    bool managedExternally() const;
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

Q_SIGNALS:
    void latestReleaseChanged();
    void dismissedChanged();
    void checkingChanged();
    void statusTextChanged();

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

    QNetworkAccessManager *m_network;
    QString m_error;
    bool m_checking = false;
};
