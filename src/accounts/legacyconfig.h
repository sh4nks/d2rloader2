#pragma once

#include "profile.h"

#include <QJsonArray>
#include <QList>
#include <QObject>
#include <QString>

/**
 * One-time import of the files of the previous D2RLoader (the Python one),
 * which kept settings.json and accounts.json in %APPDATA%\d2rloader on
 * Windows and in $XDG_CONFIG_HOME/d2rloader on Linux.
 */
namespace LegacyConfig
{
struct Result {
    /** The converted accounts, parented to the object given to migrate(). */
    QList<Profile *> profiles;
    /** Whether profiles is what the accounts file should hold from now on. */
    bool accountsConverted = false;
    /** What was done, for the user. Empty when nothing was. */
    QString notice;
};

/**
 * Imports the settings this D2RLoader has no value for yet, and the accounts
 * unless this D2RLoader already has an accounts file of its own. Runs once:
 * legacyConfigImported in the configuration records that it did.
 */
Result migrate(QObject *parent);

/**
 * Converts accounts in the format of the previous D2RLoader. Game settings
 * files in \a gameSettingsFolder, its game_settings folder, are expected to
 * have been copied to this D2RLoader's folder; pass an empty string to keep
 * their paths as they are.
 */
QList<Profile *> convertAccounts(const QJsonArray &accounts, const QString &gameSettingsFolder, QObject *parent);
}
