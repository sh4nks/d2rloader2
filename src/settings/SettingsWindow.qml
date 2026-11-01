pragma ComponentBehavior: Bound
pragma Singleton

import QtQuick
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.settings as KirigamiSettings
import org.kde.ki18n

import com.someblocks.d2rloader.accounts

KirigamiSettings.ConfigurationView {
    id: root

    title: KI18n.i18nc("@title:window", "Preferences")

    // The sidebar replaces the module page underneath pageStack.layers, so an
    // editor pushed as a layer would keep covering the new page.
    onConfigViewItemChanged: {
        const window = root.configViewItem as Kirigami.ApplicationWindow;
        if (!window) {
            return;
        }
        window.pageStack.currentItemChanged.connect(() => window.pageStack.layers.clear());
    }

    modules: [
        KirigamiSettings.ConfigurationModule {
            moduleId: "application"
            text: KI18n.i18nc("@title:menu", "Application")
            icon.name: "settings-configure"
            page: () => Qt.createComponent("com.someblocks.d2rloader.settings", "ApplicationSettingsPage")
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "accounts"
            text: KI18n.i18nc("@title:menu", "Accounts")
            icon.name: "system-users"
            page: () => Qt.createComponent("com.someblocks.d2rloader.settings", "AccountSettingsPage")
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "game_settings"
            text: KI18n.i18nc("@title:menu", "Game Settings")
            icon.name: "folder-games-symbolic"
            page: () => Qt.createComponent("com.someblocks.d2rloader.settings", "GameSettingsPage")
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "loot_filters"
            text: KI18n.i18nc("@title:menu", "Loot Filters")
            icon.name: "view-filter"
            page: () => Qt.createComponent("com.someblocks.d2rloader.settings", "LootFilterPage")
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "live_info"
            text: KI18n.i18nc("@title:menu", "DClone & TZ Info")
            icon.name: "internet-services"
            page: () => Qt.createComponent("com.someblocks.d2rloader.settings", "DiabloCloneSettingsPage")
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "launch_sequence"
            text: KI18n.i18nc("@title:menu", "Launch Sequence")
            icon.name: "media-playlist-play"
            page: () => Qt.createComponent("com.someblocks.d2rloader.settings", "LaunchSequencePage")
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "about"
            text: KI18n.i18nc("@title:menu", "About D2RLoader")
            icon.name: "help-about"
            category: KI18n.i18nc("@title:group", "About")
            initialProperties: () => {
                return {
                    "aboutData": About.aboutData
                };
            }
            page: () => Qt.createComponent("org.kde.kirigamiaddons.formcard", "AboutPage")
        }
    ]

    /**
     * Takes the window to \a moduleId. open() alone only activates the
     * window once it is open.
     */
    function show(moduleId) {
        if (root.configViewItem && typeof root.configViewItem.close === "function") {
            root.configViewItem.close();
        }
        root.open(moduleId);
    }

    /**
     * Opens the Accounts page with the editor for the account in \a row on
     * top, or for a new account when \a row is -1.
     */
    function openAccountEditor(row = -1) {
        const module = modules.find(module => module.moduleId === "accounts");
        // The page is created once per window, after this returns, so the
        // row is handed out a single time and later windows get a plain page.
        let pending = {
            editRow: row
        };
        module.initialProperties = () => {
            const properties = pending;
            pending = {};
            return properties;
        };
        root.show("accounts");
    }
}
