#include <qobject.h>
#pragma once

#include <QObject>

#include <string>

enum Region {
    Europe,
    Americas,
    Asia
};

enum AuthMethod {
    Token, Password
};

struct Account {
    std::string name;
    std::string email;
    AuthMethod auth_method;
    Region region;
    std::string token;
    std::string token_protected;
    std::string password;
    std::string game_params;
    std::string game_settings;
};

class Profile final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(AuthMethod authMethod READ authMethod WRITE setAuthMethod NOTIFY authMethodChanged)
    Q_PROPERTY(Region region READ region WRITE setRegion NOTIFY regionChanged)
    Q_PROPERTY(QString launchParameters READ launchParameters WRITE setLaunchParameters NOTIFY launchParametersChanged)

public:
    explicit Profile(const QString &name, const AuthMethod &authMethod, const Region &region, const QString &launchParameters, QObject *parent = nullptr);

    QString name() const;
    void setName(const QString &name);

    AuthMethod authMethod() const;
    void setAuthMethod(const AuthMethod &authMethod);

    Region region() const;
    void setRegion(const Region &region);

    QString launchParameters() const;
    void setLaunchParameters(const QString &launchParameters);

Q_SIGNALS:
    void nameChanged();
    void authMethodChanged();
    void regionChanged();
    void launchParametersChanged();

private:
    QString m_name;
    AuthMethod m_authMethod;
    Region m_region;
    QString m_launchParameters;
};
