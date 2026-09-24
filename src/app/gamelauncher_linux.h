#pragma once

#include "gamelauncher.h"

#include <QProcessEnvironment>
#include <QSet>
#include <functional>

class KWinWindowPositions;
class QNetworkAccessManager;

/**
 * Runs D2R.exe through umu-run, in a wineprefix of its own per account so
 * several accounts can play at once. Steam accounts are handed to the Steam
 * client instead.
 */
class LinuxGameLauncher : public GameLauncher
{
    Q_OBJECT

public:
    explicit LinuxGameLauncher(QObject *parent = nullptr);

    void launch(Profile *profile) override;
    void terminate(qint64 pid) override;
    void adopt(const QList<Profile *> &profiles) override;

private:
    void launchSteam(Profile *profile, const QStringList &arguments);
    void launchUmu(Profile *profile, const QString &executable, const QStringList &arguments);

    /**
     * Downloads d2rreg.exe unless it is there already, then calls \a next.
     */
    void fetchTokenTool(Profile *profile, const std::function<void()> &next);

    /**
     * Sets up the profile's wineprefix through Proton unless that was done
     * already, then calls \a next.
     */
    void createPrefix(Profile *profile, const std::function<void()> &next);

    /**
     * Writes the profile's token into the registry of its wineprefix with
     * d2rreg.exe, then calls \a next.
     */
    void runTokenTool(Profile *profile, const std::function<void()> &next);

    /**
     * Renames the window of the game started as \a pid to
     * Profile::windowTitle() with d2rreg.exe, retrying while the window does
     * not exist yet, then restores its position.
     */
    void renameWindow(Profile *profile, qint64 pid, int attemptsLeft);

    /**
     * Moves the game window of \a profile to its saved position with
     * d2rreg.exe, then follows it. On Plasma the KWin script does this.
     */
    void restoreWindowPosition(Profile *profile);

    /**
     * Saves every position the game window of \a profile settles at with
     * d2rreg.exe, until the window is gone.
     */
    void watchWindowPosition(Profile *profile);

    /**
     * Ends the wineprefix of the game started as \a pid with wineserver -k,
     * and whatever outlives it with SIGKILL.
     */
    void killWineserver(qint64 pid, const QString &marker);

    /**
     * Emits finished() once \a pid exits.
     */
    void watch(qint64 pid);

    QProcessEnvironment environment(const Profile *profile) const;

    QNetworkAccessManager *m_network = nullptr;
    KWinWindowPositions *m_kwinWindowPositions = nullptr;
    QSet<qint64> m_terminating;
};
