#pragma once

#include "profile.h"

#include <QObject>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <expected>
#include <qqmlintegration.h>

/**
 * Locates and moves the game's Settings.json for a profile. Each profile can
 * keep its own copy, which is put in place of the game's before launching.
 */
class GameSettings : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QStringList files READ files NOTIFY filesChanged)

public:
    explicit GameSettings(QObject *parent = nullptr);

    /**
     * The wineprefix the game of \a profile runs in: Steam's own prefix for
     * Steam accounts, otherwise the profile's prefix under wineprefixPath.
     * Empty when Steam cannot be found.
     */
    static QString gamePrefix(const Profile *profile);

    /**
     * The folder the game keeps the settings, loot filters and controls of
     * \a profile in. Empty when Steam cannot be found.
     */
    static QString savedGamesPath(const Profile *profile);

    /**
     * The Settings.json the game reads for \a profile.
     */
    static QString currentSettingsPath(const Profile *profile);

    /**
     * The account specific copy of Settings.json for \a profile.
     */
    static QString profileSettingsPath(const Profile *profile);

    /**
     * The file put in place before launching, empty when the game's own
     * settings are left alone.
     */
    static QString resolvedPath(const Profile *profile);

    /**
     * Replaces the game's Settings.json with the one chosen for \a profile.
     * The replaced file is kept as Settings.json.bak.
     */
    static std::expected<void, QString> apply(const Profile *profile);

    /**
     * The Settings.json files in the folder the account specific copies are
     * kept in, by file name.
     */
    QStringList files() const;

    /**
     * Reads the folder the account specific copies are kept in again.
     */
    Q_INVOKABLE void refresh();

    /**
     * Whether \a profile has the file \a name from files() put in place
     * before launching.
     */
    Q_INVOKABLE bool uses(Profile *profile, const QString &name) const;

    /**
     * Removes the file \a name from files(). Returns why that failed, or an
     * empty string.
     */
    Q_INVOKABLE QString remove(const QString &name);

    Q_INVOKABLE bool profileSettingsExist(Profile *profile) const;

    /**
     * The folder the account specific copies are kept in. It is created if
     * needed, so a file dialog can open there.
     */
    Q_INVOKABLE QUrl profileSettingsFolder() const;

    /**
     * The Settings.json the game ends up with when \a profile uses \a type:
     * the game's own for None, otherwise the file put in its place.
     */
    Q_INVOKABLE QString settingsPath(Profile *profile, GameSettingsType::Type type) const;

    /**
     * Copies the game's current Settings.json of \a profile to its account
     * specific copy. Returns why that failed, or an empty string.
     */
    Q_INVOKABLE QString copyCurrent(Profile *profile, bool overwrite);

Q_SIGNALS:
    void filesChanged();

private:
    QStringList m_files;
};
