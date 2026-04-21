pragma ComponentBehavior: Bound

import QtQuick
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.settings as KirigamiSettings

KirigamiSettings.ConfigurationWindow {
    id: root

    title: "Preferences"

    // Set a reasonable default size for the settings window
    width: Kirigami.Units.gridUnit * 50
    height: Kirigami.Units.gridUnit * 35

    modules: [
        KirigamiSettings.ConfigurationModule {
            moduleId: "application"
            text: "Application"
            icon.name: "settings-configure"
            // Use the project's QML module URI to load the pages
            page: () => Qt.createComponent("org.someblocks.d2rloader", "ApplicationSettingsPage")
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "accounts"
            text: "Accounts"
            icon.name: "user-identity"
            page: () => Qt.createComponent("org.someblocks.d2rloader", "AccountSettingsPage")
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "dclone"
            text: "Diablo Clone"
            icon.name: "view-media-artist"
            page: () => Qt.createComponent("org.someblocks.d2rloader", "DiabloCloneSettingsPage")
        }
    ]
}
