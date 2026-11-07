#pragma once

#include "../accounts/profile.h"

#include <QHash>
#include <QObject>
#include <QPointer>
#include <QTimer>
#include <qqmlintegration.h>

class GameLauncher;

/**
 * Starts and stops the game for profiles and keeps their status in step:
 * Starting while the launcher prepares, Running while the game process is
 * alive, Stopped otherwise.
 */
class GameManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(bool sequenceRunning READ sequenceRunning NOTIFY sequenceRunningChanged)
    Q_PROPERTY(QString launchProgress READ launchProgress NOTIFY launchProgressChanged)

public:
    explicit GameManager(QObject *parent = nullptr);

    Q_INVOKABLE void start(Profile *profile);
    Q_INVOKABLE void stop(Profile *profile);

    /**
     * Starts \a profile if it is stopped and stops it if it is running.
     */
    Q_INVOKABLE void toggle(Profile *profile);

    /**
     * Starts the accounts of the launch sequence in \a row one after another,
     * waiting for each to be launched plus the configured delay. Accounts that
     * are already running are skipped.
     */
    Q_INVOKABLE void startSequence(int row);

    /**
     * Stops starting further accounts of the running sequence. An account that
     * is starting already is left to finish.
     */
    Q_INVOKABLE void cancelSequence();

    bool sequenceRunning() const;

    /**
     * What the launcher is doing for the account that is starting, for steps
     * that take a while such as downloading Proton. Empty otherwise.
     */
    QString launchProgress() const;

Q_SIGNALS:
    void launchFailed(const QString &profileName, const QString &error);
    void sequenceRunningChanged();
    void launchProgressChanged();

private:
    void onLaunched(Profile *profile, qint64 pid);
    void onFailed(Profile *profile, const QString &error);
    void onFinished(qint64 pid);
    void onProgress(Profile *profile, const QString &message);

    void launchNextInSequence();
    void sequenceStepDone(bool started);
    void setSequenceRunning(bool running);
    void setLaunchProgress(const QString &progress);

    GameLauncher *m_launcher = nullptr;
    QHash<qint64, QPointer<Profile>> m_running; /* game pid -> profile */
    QString m_launchProgress;
    QPointer<Profile> m_progressProfile; /* account launchProgress is about */

    QList<QPointer<Profile>> m_sequence; /* accounts of the sequence still to start */
    QPointer<Profile> m_sequenceProfile; /* account of the sequence that is starting */
    QTimer m_sequenceTimer;
    bool m_sequenceRunning = false;
};
