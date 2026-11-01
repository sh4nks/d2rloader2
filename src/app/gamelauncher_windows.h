#pragma once

#include "gamelauncher.h"

#include <QHash>
#include <QPointer>
#include <QSet>
#include <QStringList>

/**
 * Runs D2R.exe directly. The game refuses to start while another instance
 * holds its "Check For Other Instances" event, so that handle is closed in
 * every game after it starts, which lets several accounts play at once.
 * Steam accounts are handed to the Steam client instead.
 */
class WindowsGameLauncher : public GameLauncher
{
    Q_OBJECT

public:
    explicit WindowsGameLauncher(QObject *parent = nullptr);
    ~WindowsGameLauncher() override;

    void launch(Profile *profile) override;
    void terminate(qint64 pid) override;
    void adopt(const QList<Profile *> &profiles) override;

private:
    /**
     * Writes the token of a token account and starts the game. Every token
     * account reads its token from the same registry value, so the launch is
     * held while the game of another token account has not read its own yet.
     */
    void startGame(Profile *profile, const QString &executable, const QStringList &arguments);
    void launchGame(Profile *profile, const QString &executable, const QStringList &arguments);

    /**
     * Called once the game started as \a pid has read its token, that is
     * once its window exists or it is gone: starts the next held launch.
     */
    void tokenRead(qint64 pid);

    /**
     * Closes the single instance event of the game started as \a pid,
     * retrying while the game has not created it yet.
     */
    void releaseInstanceGuard(qint64 pid, int attemptsLeft);

    /**
     * Renames the window of the game started as \a pid to
     * Profile::windowTitle(), retrying while the window does not exist yet,
     * then follows it.
     */
    void renameWindow(Profile *profile, qint64 pid, int attemptsLeft);

    /**
     * Moves the game window \a window (an HWND) of \a profile, running as
     * \a pid, back to its saved position and saves every position it settles
     * at until it is gone.
     */
    void followWindow(Profile *profile, qint64 pid, void *window);

    /**
     * Ends \a pid without asking.
     */
    void kill(qint64 pid);

    /**
     * Emits finished() once \a pid exits.
     */
    void watch(qint64 pid);

    bool isRunning(qint64 pid) const;

    struct HeldLaunch {
        QPointer<Profile> profile;
        QString executable;
        QStringList arguments;
    };

    QHash<qint64, void *> m_processes; /* game pid -> process handle, while the game is watched */
    QSet<qint64> m_terminating;
    qint64 m_tokenGamePid = 0; /* the game of a token account that has not shown its window yet */
    QList<HeldLaunch> m_heldLaunches;
};
