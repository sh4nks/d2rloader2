#include "profile.h"
#include "authmethod.h"
#include "region.h"
#include <qhashfunctions.h>
#include <qobject.h>

Profile::Profile(QObject *parent)
    : QObject(parent)
{
}

ProfileState::Type Profile::status() const
{
    return m_status;
}

void Profile::setStatus(const ProfileState::Type status)
{
    if (status == m_status) {
        return;
    }
    const auto oldStatus = std::exchange(m_status, status);
    Q_EMIT statusChanged(oldStatus, m_status);
}

QString Profile::profileName() const
{
    return m_profileName;
}

void Profile::setProfileName(const QString &name)
{
    if (m_profileName != name) {
        m_profileName = name;
        Q_EMIT profileNameChanged();
    }
}

AuthMethodModel::AuthMethod Profile::authMethod() const
{
    return m_authMethod;
}

void Profile::setAuthMethod(const AuthMethodModel::AuthMethod &authMethod)
{
    if (m_authMethod != authMethod) {
        m_authMethod = authMethod;
        Q_EMIT authMethodChanged();
    }
}

RegionModel::Region Profile::region() const
{
    return m_region;
}

void Profile::setRegion(const RegionModel::Region &region)
{
    if (m_region != region) {
        m_region = region;
        Q_EMIT regionChanged();
    }
}

QString Profile::email() const
{
    return m_email;
}

void Profile::setEmail(const QString &email)
{
    if (m_email != email) {
        m_email = email;
        Q_EMIT emailChanged();
    }
}

QString Profile::password() const
{
    return m_password;
}

void Profile::setPassword(const QString &password)
{
    if (m_password != password) {
        m_password = password;
        Q_EMIT passwordChanged();
    }
}

QString Profile::token() const
{
    return m_token;
}

void Profile::setToken(const QString &token)
{
    if (m_token != token) {
        m_token = token;
        Q_EMIT tokenChanged();
    }
}

QString Profile::gameParameters() const
{
    return m_gameParameters;
}

void Profile::setGameParameters(const QString &gameParameters)
{
    if (m_gameParameters != gameParameters) {
        m_gameParameters = gameParameters;
        Q_EMIT gameParametersChanged();
    }
}

GameSettingsType::Type Profile::gameSettings() const
{
    return m_gameSettings;
}

void Profile::setGameSettings(const GameSettingsType::Type type)
{
    if (m_gameSettings != type) {
        m_gameSettings = type;
        Q_EMIT gameSettingsChanged();
    }
}

QString Profile::gameSettingsPath() const
{
    return m_gameSettingsPath;
}

void Profile::setGameSettingsPath(const QString &gameSettingsPath)
{
    if (m_gameSettingsPath != gameSettingsPath) {
        m_gameSettingsPath = gameSettingsPath;
        Q_EMIT gameSettingsPathChanged();
    }
}

QString Profile::gamePath() const
{
    return m_gamePath;
}

void Profile::setGamePath(const QString &gamePath)
{
    if (m_gamePath != gamePath) {
        m_gamePath = gamePath;
        Q_EMIT gamePathChanged();
    }
}

QString Profile::protonPath() const
{
    return m_protonPath;
}

void Profile::setProtonPath(const QString &protonPath)
{
    if (m_protonPath != protonPath) {
        m_protonPath = protonPath;
        Q_EMIT protonPathChanged();
    }
}

Profile *fromJson(QJsonObject &jsonObj)
{
    auto profileName = jsonObj.find(QStringLiteral("profileName")) auto profile = new Profile(nullptr);

    return nullptr;
}
