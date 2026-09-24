#pragma once

#include "profile.h"

#include <QObject>
#include <QStringList>
#include <QUrl>
#include <expected>
#include <qqmlintegration.h>

/**
 * The library of loot filters (.fltr) the accounts choose from. Before
 * launching, the chosen filter is copied into the game's folder and given to
 * every character of the account through the game's lootfilter.json.
 */
class LootFilters : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QStringList names READ names NOTIFY namesChanged)

public:
    explicit LootFilters(QObject *parent = nullptr);

    /**
     * The characters of \a profile, taken from the controls files the game
     * keeps for each of them.
     */
    static QStringList characters(const Profile *profile);

    /**
     * Puts the loot filter chosen for \a profile in place and assigns it to
     * all of its characters. Does nothing when none is chosen.
     */
    static std::expected<void, QString> apply(const Profile *profile);

    QStringList names() const;

    /**
     * Reads the library folder again.
     */
    Q_INVOKABLE void refresh();

    /**
     * The library folder. It is created if needed, so it can be opened.
     */
    Q_INVOKABLE QUrl libraryFolder() const;

    Q_INVOKABLE QStringList characterNames(Profile *profile) const;

    /**
     * The loot filters in the game's folder of \a profile.
     */
    Q_INVOKABLE QStringList gameFilters(Profile *profile) const;

    /**
     * Copies the loot filter \a name from the game's folder of \a profile to
     * the library. Returns why that failed, or an empty string.
     */
    Q_INVOKABLE QString importFromGame(Profile *profile, const QString &name, bool overwrite);

    /**
     * Copies the loot filter file \a url to the library. Returns why that
     * failed, or an empty string.
     */
    Q_INVOKABLE QString importFile(const QUrl &url, bool overwrite);

    Q_INVOKABLE QString remove(const QString &name);

Q_SIGNALS:
    void namesChanged();

private:
    QString store(const QString &name, QByteArray data, bool overwrite);

    QStringList m_names;
};
