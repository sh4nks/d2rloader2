#pragma once

#include <QLoggingCategory>

/**
 * The application's own logging categories. Only these reach the "Application
 * Log" tab - see the allowlist in logbuffer.cpp. Adding a category here also
 * means adding it there. A plain qDebug()/qWarning() does not show up in the
 * tab; use qCDebug(LOG_APP) and friends.
 */
Q_DECLARE_LOGGING_CATEGORY(LOG_APP)
Q_DECLARE_LOGGING_CATEGORY(LOG_PROFILES)
Q_DECLARE_LOGGING_CATEGORY(LOG_GAMEINFO)
Q_DECLARE_LOGGING_CATEGORY(LOG_GAME)

// QML logs through "d2rloader.qml", declared in Log.qml.
