#pragma once
#include "authmethod.h"
#include "region.h"
#include <QMetaEnum>
#include <QObject>
#include <QVariantList>
#include <QVariantMap>
#include <qhashfunctions.h>
#include <qjsonvalue.h>
#include <qobject.h>
#include <qqmlintegration.h>
#include <qtmetamacros.h>

/**
 * @class ChatBarType
 *
 * This class is designed to define the ChatBarType enumeration.
 */
class ProfileState : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

public:
    enum Type {
        Running = 0,
        Stopped,
        None,
    };
    Q_ENUM(Type);
};

class Profile final : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Profile instances must be created and managed in C++.")

    Q_PROPERTY(ProfileState::Type status READ status WRITE setStatus NOTIFY statusChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(AuthMethodModel::AuthMethod authMethod READ authMethod WRITE setAuthMethod NOTIFY authMethodChanged)
    Q_PROPERTY(RegionModel::Region region READ region WRITE setRegion NOTIFY regionChanged)
    Q_PROPERTY(QString gameParameters READ gameParameters WRITE setGameParameters NOTIFY gameParametersChanged)

public:
    explicit Profile(const QString &name,
                     const AuthMethodModel::AuthMethod &authMethod,
                     const RegionModel::Region &region,
                     const QString &gameParameters,
                     QObject *parent = nullptr);

    ProfileState::Type status() const;
    void setStatus(const ProfileState::Type status);

    QString name() const;
    void setName(const QString &name);

    AuthMethodModel::AuthMethod authMethod() const;
    void setAuthMethod(const AuthMethodModel::AuthMethod &authMethod);

    RegionModel::Region region() const;
    void setRegion(const RegionModel::Region &region);

    QString gameParameters() const;
    void setGameParameters(const QString &gameParameters);

Q_SIGNALS:
    void nameChanged();
    void authMethodChanged();
    void regionChanged();
    void gameParametersChanged();
    void statusChanged(ProfileState::Type oldStatus, ProfileState::Type newStatus);

private:
    ProfileState::Type m_status = ProfileState::Stopped;
    QString m_name;
    QString m_email;
    AuthMethodModel::AuthMethod m_authMethod;
    RegionModel::Region m_region;
    QString m_token;
    QString m_tokenProtected;
    QString m_password;
    QString m_gameParameters;
    QString m_gameSettings;
    QString m_gamePath;
    QString m_protonPath;
};
