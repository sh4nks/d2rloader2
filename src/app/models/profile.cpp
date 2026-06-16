#include "profile.h"
#include "authmethod.h"
#include "region.h"
#include <qobject.h>

Profile::Profile(const QString &name,
                 const AuthMethodModel::AuthMethod &authMethod,
                 const RegionModel::Region &region,
                 const QString &gameParameters,
                 QObject *parent)
    : QObject(parent)
    , m_name(name)
    , m_authMethod(authMethod)
    , m_region(region)
    , m_gameParameters(gameParameters)
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

QString Profile::name() const
{
    return m_name;
}

void Profile::setName(const QString &name)
{
    if (m_name != name) {
        m_name = name;
        Q_EMIT nameChanged();
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
