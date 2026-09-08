#pragma once
#include "authmethod.h"
#include "region.h"
#include <QJsonObject>
#include <QMetaEnum>
#include <QObject>
#include <QVariantList>
#include <QVariantMap>
#include <qhashfunctions.h>
#include <qjsonobject.h>
#include <qjsonvalue.h>
#include <qobject.h>
#include <qqmlintegration.h>
#include <qtmetamacros.h>

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

class GameSettingsType : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

public:
    enum Type {
        None = 0, /* no game settings specified */
        Profile, /* profile-based game settings used */
        Custom, /* custom path to the game settings */
    };
    Q_ENUM(Type);
};

class Profile : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Profile instances must be created and managed in C++.")

    Q_PROPERTY(ProfileState::Type status READ status WRITE setStatus NOTIFY statusChanged)
    Q_PROPERTY(QString profileName READ profileName WRITE setProfileName NOTIFY profileNameChanged)
    Q_PROPERTY(AuthMethodModel::AuthMethod authMethod READ authMethod WRITE setAuthMethod NOTIFY authMethodChanged)
    Q_PROPERTY(RegionModel::Region region READ region WRITE setRegion NOTIFY regionChanged)
    Q_PROPERTY(QString email READ email WRITE setEmail NOTIFY emailChanged)
    Q_PROPERTY(QString token READ token WRITE setToken NOTIFY tokenChanged)
    Q_PROPERTY(QString password READ password WRITE setPassword NOTIFY passwordChanged)
    Q_PROPERTY(QString gameParameters READ gameParameters WRITE setGameParameters NOTIFY gameParametersChanged)
    Q_PROPERTY(QString gameSettingsPath READ gameSettingsPath WRITE setGameSettingsPath NOTIFY gameSettingsPathChanged)
    Q_PROPERTY(GameSettingsType::Type gameSettings READ gameSettings WRITE setGameSettings NOTIFY gameSettingsChanged)
    Q_PROPERTY(QString gamePath READ gamePath WRITE setGamePath NOTIFY gamePathChanged)
    Q_PROPERTY(QString protonPath READ protonPath WRITE setProtonPath NOTIFY protonPathChanged)

public:
    explicit Profile(QObject *parent = nullptr);

    int id() const;
    void setId(const int id);

    ProfileState::Type status() const;
    void setStatus(const ProfileState::Type status);

    QString profileName() const;
    void setProfileName(const QString &profileName);

    AuthMethodModel::AuthMethod authMethod() const;
    void setAuthMethod(const AuthMethodModel::AuthMethod &authMethod);

    RegionModel::Region region() const;
    void setRegion(const RegionModel::Region &region);

    QString email() const;
    void setEmail(const QString &email);

    QString token() const;
    void setToken(const QString &token);

    QString tokenProtected() const;
    void setTokenProtected(const QString &token);

    QString password() const;
    void setPassword(const QString &password);

    QString gameParameters() const;
    void setGameParameters(const QString &gameParameters);

    QString gameSettingsPath() const;
    void setGameSettingsPath(const QString &gameSettingsPath);

    QString gamePath() const;
    void setGamePath(const QString &gamePath);

    QString protonPath() const;
    void setProtonPath(const QString &protonPath);

    GameSettingsType::Type gameSettings() const;
    void setGameSettings(const GameSettingsType::Type gameSettings);

    void update(const QString &profileName, const AuthMethodModel::AuthMethod &authMethod, const RegionModel::Region &region, const QString &gameParameters);

    static Profile *create(const ProfileState::Type status,
                           const QString &profileName,
                           const AuthMethodModel::AuthMethod authMethod,
                           const RegionModel::Region region,
                           const QString &email,
                           const QString &token,
                           const QString &tokenProtected,
                           const QString &password,
                           const QString &gameParameters,
                           const GameSettingsType::Type gameSettings,
                           const QString &gameSettingsPath,
                           const QString &gamePath,
                           const QString &protonPath);

    static Profile *fromJson(QJsonObject &jsonObj);

Q_SIGNALS:
    void statusChanged(ProfileState::Type oldStatus, ProfileState::Type newStatus);
    void profileNameChanged();
    void authMethodChanged();
    void regionChanged();
    void emailChanged();
    void tokenChanged();
    void passwordChanged();
    void gameParametersChanged();
    void gameSettingsChanged();
    void gameSettingsPathChanged();
    void gamePathChanged();
    void protonPathChanged();

private:
    int m_id;
    ProfileState::Type m_status = ProfileState::Stopped;
    QString m_profileName;
    AuthMethodModel::AuthMethod m_authMethod;
    RegionModel::Region m_region;
    QString m_email;
    QString m_token;
    QString m_tokenProtected;
    QString m_password;
    QString m_gameParameters;
    GameSettingsType::Type m_gameSettings = GameSettingsType::None;
    QString m_gameSettingsPath;
    QString m_gamePath;
    QString m_protonPath;
};
