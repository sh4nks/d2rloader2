#pragma once
#include "authmethod.h"
#include "region.h"
#include <QJsonObject>
#include <QMetaEnum>
#include <QObject>
#include <QPoint>
#include <QVariantList>
#include <QVariantMap>
#include <qhashfunctions.h>
#include <qjsonobject.h>
#include <qjsonvalue.h>
#include <qobject.h>
#include <qqmlintegration.h>
#include <qtmetamacros.h>

#include <optional>

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
        Starting,
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
    Q_PROPERTY(QString environmentVariables READ environmentVariables WRITE setEnvironmentVariables NOTIFY environmentVariablesChanged)
    Q_PROPERTY(QString lootFilter READ lootFilter WRITE setLootFilter NOTIFY lootFilterChanged)
    Q_PROPERTY(bool rememberWindowPosition READ rememberWindowPosition WRITE setRememberWindowPosition NOTIFY rememberWindowPositionChanged)

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

    /**
     * Extra environment for the game, one NAME=value per line.
     */
    QString environmentVariables() const;
    void setEnvironmentVariables(const QString &environmentVariables);

    /**
     * The name of the library loot filter given to every character of this
     * account, empty to leave the game's loot filters alone.
     */
    QString lootFilter() const;
    void setLootFilter(const QString &lootFilter);

    GameSettingsType::Type gameSettings() const;
    void setGameSettings(const GameSettingsType::Type gameSettings);

    /**
     * Whether the game window is moved back to where it was when the game of
     * this account last closed.
     */
    bool rememberWindowPosition() const;
    void setRememberWindowPosition(bool rememberWindowPosition);

    /**
     * Where the game window of this account was last seen, empty until it
     * was seen once.
     */
    std::optional<QPoint> windowPosition() const;
    void setWindowPosition(const QPoint &windowPosition);

    /**
     * The title the game window of this account is renamed to, in the format
     * of the original D2RLoader: "Account (server address)".
     */
    QString windowTitle() const;

    /**
     * The profile name reduced to lowercase ASCII words joined by "-",
     * matching the original D2RLoader so that its per-account wineprefixes
     * are reused. Falls back to the email, then to the id, when nothing of
     * the name survives. Never empty.
     */
    QString normalizedName() const;

    /**
     * The wineprefix the game of this profile runs in when started through
     * umu: normalizedName() under the configured wineprefixPath.
     */
    QString wineprefix() const;

    /**
     * Copies every persisted field, including the id, from \a other.
     * The runtime status and the window position are left alone: both come
     * from the running game, which an editor's draft must not overwrite.
     */
    void copyFrom(const Profile *other);

    QJsonObject toJson() const;
    static Profile *fromJson(const QJsonObject &json, QObject *parent = nullptr);

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
    void environmentVariablesChanged();
    void lootFilterChanged();
    void rememberWindowPositionChanged();
    void windowPositionChanged();

private:
    int m_id = 0; /* 0 means "not stored yet", see ProfileManager::commit() */
    ProfileState::Type m_status = ProfileState::Stopped;
    QString m_profileName;
    AuthMethodModel::AuthMethod m_authMethod = AuthMethodModel::Password;
    RegionModel::Region m_region = RegionModel::Europe;
    QString m_email;
    QString m_token;
    QString m_password;
    QString m_gameParameters;
    GameSettingsType::Type m_gameSettings = GameSettingsType::None;
    QString m_gameSettingsPath;
    QString m_gamePath;
    QString m_protonPath;
    QString m_environmentVariables;
    QString m_lootFilter;
    bool m_rememberWindowPosition = true;
    std::optional<QPoint> m_windowPosition;
};
