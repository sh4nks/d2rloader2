#include "gamemanager.h"
#include "../accounts/launchsequencemanager.h"
#include "../accounts/profile.h"
#include "../accounts/profilemanager.h"
#include "../core/logging.h"
#include "d2rloaderconfig.h"
#include "gamelauncher.h"

#include <KLocalizedString>
#include <QRandomGenerator>

GameManager::GameManager(QObject *parent)
    : QObject(parent)
    , m_launcher(GameLauncher::create(this))
{
    m_sequenceTimer.setSingleShot(true);
    connect(&m_sequenceTimer, &QTimer::timeout, this, &GameManager::launchNextInSequence);
    connect(&ProfileManager::instance(), &ProfileManager::profileRemoved, this, [this](Profile *profile) {
        if (profile == m_sequenceProfile) {
            sequenceStepDone(false);
        }
    });

    if (!m_launcher) {
        return;
    }
    connect(m_launcher, &GameLauncher::launched, this, &GameManager::onLaunched);
    connect(m_launcher, &GameLauncher::failed, this, &GameManager::onFailed);
    connect(m_launcher, &GameLauncher::finished, this, &GameManager::onFinished);

    // The accounts are loaded before QML creates this singleton.
    m_launcher->adopt(ProfileManager::instance().profiles());
}

void GameManager::start(Profile *profile)
{
    if (!profile || profile->status() == ProfileState::Running || profile->status() == ProfileState::Starting) {
        return;
    }

    if (!m_launcher) {
        Q_EMIT launchFailed(profile->profileName(), i18nc("@info", "Launching the game is not supported on this platform yet."));
        return;
    }

    profile->setStatus(ProfileState::Starting);
    m_launcher->launch(profile);
}

void GameManager::stop(Profile *profile)
{
    for (auto it = m_running.cbegin(); it != m_running.cend(); ++it) {
        if (it.value() == profile) {
            m_launcher->terminate(it.key());
            return;
        }
    }
}

void GameManager::toggle(Profile *profile)
{
    if (!profile) {
        return;
    }

    switch (profile->status()) {
    case ProfileState::Running:
        stop(profile);
        break;
    case ProfileState::Starting:
        break;
    case ProfileState::Stopped:
    case ProfileState::None:
        start(profile);
        break;
    }
}

void GameManager::startSequence(int row)
{
    if (m_sequenceRunning) {
        return;
    }

    const LaunchSequenceManager &sequences = LaunchSequenceManager::instance();
    if (!m_launcher) {
        Q_EMIT launchFailed(sequences.name(row), i18nc("@info", "Launching the game is not supported on this platform yet."));
        return;
    }

    const QList<Profile *> profiles = sequences.profiles(row);
    if (profiles.isEmpty()) {
        return;
    }

    qCInfo(LOG_GAME) << "starting the launch sequence" << sequences.name(row) << "with" << profiles.count() << "accounts";
    for (Profile *profile : profiles) {
        m_sequence.append(profile);
    }
    setSequenceRunning(true);
    launchNextInSequence();
}

void GameManager::cancelSequence()
{
    if (!m_sequenceRunning) {
        return;
    }

    qCInfo(LOG_GAME) << "launch sequence ended with" << m_sequence.count() << "accounts left";
    m_sequenceTimer.stop();
    m_sequence.clear();
    m_sequenceProfile.clear();
    setSequenceRunning(false);
}

bool GameManager::sequenceRunning() const
{
    return m_sequenceRunning;
}

void GameManager::launchNextInSequence()
{
    while (!m_sequence.isEmpty()) {
        const QPointer<Profile> profile = m_sequence.takeFirst();
        if (!profile || profile->status() == ProfileState::Running || profile->status() == ProfileState::Starting) {
            continue;
        }

        m_sequenceProfile = profile;
        start(profile);
        return;
    }

    cancelSequence();
}

void GameManager::sequenceStepDone(bool started)
{
    m_sequenceProfile.clear();

    if (m_sequence.isEmpty() || (!started && D2RLoaderConfig::self()->sequenceStopOnFailure())) {
        cancelSequence();
        return;
    }

    // A failed account has no game to wait for.
    int delay = 0;
    if (started) {
        delay = D2RLoaderConfig::self()->sequenceDelay() * 1000;
        if (D2RLoaderConfig::self()->sequenceRandomizeDelay()) {
            delay += QRandomGenerator::global()->bounded(delay / 2 + 1);
        }
    }
    m_sequenceTimer.start(delay);
}

void GameManager::setSequenceRunning(bool running)
{
    if (m_sequenceRunning != running) {
        m_sequenceRunning = running;
        Q_EMIT sequenceRunningChanged();
    }
}

void GameManager::onLaunched(Profile *profile, qint64 pid)
{
    if (pid == 0) {
        profile->setStatus(ProfileState::Stopped);
    } else {
        m_running.insert(pid, profile);
        profile->setStatus(ProfileState::Running);
    }

    if (profile == m_sequenceProfile) {
        sequenceStepDone(true);
    }
}

void GameManager::onFailed(Profile *profile, const QString &error)
{
    qCWarning(LOG_GAME) << "could not launch" << profile->profileName() << ":" << error;
    profile->setStatus(ProfileState::Stopped);
    Q_EMIT launchFailed(profile->profileName(), error);

    if (profile == m_sequenceProfile) {
        sequenceStepDone(false);
    }
}

void GameManager::onFinished(qint64 pid)
{
    const QPointer<Profile> profile = m_running.take(pid);
    if (profile) {
        profile->setStatus(ProfileState::Stopped);
    }
}
