pragma Singleton

import QtQml

/**
 * The logging category for QML. Pass it to console.log() and friends so the
 * message reaches the "Application Log" tab:
 *
 *     console.warn(Log.category, "something went wrong");
 *
 * A console.log() without it goes to stderr only. The category has to be
 * reached through a property - a singleton whose root object is the
 * LoggingCategory itself is not recognised by console.log() in every scope.
 */
QtObject {
    readonly property LoggingCategory category: LoggingCategory {
        name: "d2rloader.qml"
    }
}
