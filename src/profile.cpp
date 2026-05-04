#include "profile.h"
#include <qobject.h>

Profile::Profile(const QString &name, const AuthMethod &authMethod, const Region &region, const QString &launchParameters, QObject *parent)
    : QObject(parent)
    , m_name(name)
    , m_authMethod(authMethod)
    , m_region(region)
    , m_launchParameters(launchParameters)
{
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

AuthMethod Profile::authMethod() const
{
    return m_authMethod;
}

void Profile::setAuthMethod(const AuthMethod &authMethod)
{
    if (m_authMethod != authMethod) {
        m_authMethod = authMethod;
        Q_EMIT authMethodChanged();
    }
}

Region Profile::region() const
{
    return m_region;
}

void Profile::setRegion(const Region &region)
{
    if (m_region != region) {
        m_region = region;
        Q_EMIT regionChanged();
    }
}

QString Profile::launchParameters() const
{
    return m_launchParameters;
}

void Profile::setLaunchParameters(const QString &launchParameters)
{
    if (m_launchParameters != launchParameters) {
        m_launchParameters = launchParameters;
        Q_EMIT launchParametersChanged();
    }
}
