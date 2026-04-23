pragma ComponentBehavior: Bound

import QtQuick
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.settings as KirigamiSettings

KirigamiSettings.ConfigurationView {
    id: root

    title: "Preferences"

    modules: [
        KirigamiSettings.ConfigurationModule {
            moduleId: "application"
            text: "Application"
            icon.name: "settings-configure"
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
