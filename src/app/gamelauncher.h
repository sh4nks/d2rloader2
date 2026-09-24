#pragma once

#include "../accounts/profile.h"

#include <QObject>
#include <QStringList>
#include <expected>

/**
 * Starts and stops Diablo II: Resurrected for a profile. Each platform
 * provides its own subclass through create().
 *
 * launch() is asynchronous, because some auth methods need work done before
 * the game can start (token auth writes the token into the registry first).
 * It ends in either launched() or failed(). A launched process is watched
 * until it exits, which is reported through finished().
 */
class GameLauncher : public QObject
{
    Q_OBJECT

public:
    /**
     * Returns the launcher for the running platform, or nullptr if there is
     * none yet.
     */
    static GameLauncher *create(QObject *parent = nullptr);

    virtual void launch(Profile *profile) = 0;

    /**
     * Ends the game started as \a pid, including everything it spawned. The
     * game is asked to close first where the platform allows.
     */
    virtual void terminate(qint64 pid) = 0;

    /**
     * Finds games of \a profiles still running from an earlier session and
     * reports each through launched(), then follows it like a new one.
     */
    virtual void adopt(const QList<Profile *> &profiles) = 0;

Q_SIGNALS:
    /**
     * \a pid is 0 when the game was handed to another launcher (Steam) and
     * cannot be followed; no finished() will come for it.
     */
    void launched(Profile *profile, qint64 pid);
    void failed(Profile *profile, const QString &error);
    void finished(qint64 pid);

protected:
    using QObject::QObject;

    /**
     * The D2R.exe in the game directory of \a profile, or in the configured
     * one when the profile has none, or why it cannot be used.
     */
    static std::expected<QString, QString> gameExecutable(const Profile *profile);

    /**
     * The arguments every platform passes to D2R.exe: the realm, the
     * credentials of the auth method and the profile's own parameters.
     */
    static std::expected<QStringList, QString> gameArguments(const Profile *profile);

    /**
     * \a arguments with the credentials masked, for logging.
     */
    static QStringList redacted(QStringList arguments);
};
