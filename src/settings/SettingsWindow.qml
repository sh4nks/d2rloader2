pragma ComponentBehavior: Bound
pragma Singleton

import QtQuick
import org.kde.kirigamiaddons.settings as KirigamiSettings

import com.someblocks.d2rloader 1.0

KirigamiSettings.ConfigurationView {
    id: root

    title: i18nc("@title:window", "Preferences")

    modules: [
        KirigamiSettings.ConfigurationModule {
            moduleId: "application"
            text: i18nc("@title:menu", "Application")
            icon.name: "settings-configure"
            page: () => Qt.createComponent("com.someblocks.d2rloader", "ApplicationSettingsPage")
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "accounts"
            text: i18nc("@title:menu", "Accounts")
            icon.name: "system-users"
            page: () => Qt.createComponent("com.someblocks.d2rloader", "AccountSettingsPage")
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "game_settings"
            text: i18nc("@title:menu", "Game Settings")
            icon.name: "folder-games-symbolic"
            page: () => Qt.createComponent("com.someblocks.d2rloader", "GameSettingsPage")
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "live_info"
            text: i18nc("@title:menu", "DClone & TZ Info")
            icon.name: "internet-services"
            page: () => Qt.createComponent("com.someblocks.d2rloader", "DiabloCloneSettingsPage")
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "launch_sequence"
            text: i18nc("@title:menu", "Launch Sequence")
            icon.name: "media-playlist-play"
            page: () => Qt.createComponent("com.someblocks.d2rloader", "LaunchSequencePage")
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "about"
            text: i18nc("@title:menu", "About D2RLoader")
            icon.name: "help-about"
            category: i18nc("@title:group", "About")
            initialProperties: () => {
                return {
                    "aboutData": About
                };
            }
            page: () => Qt.createComponent("org.kde.kirigamiaddons.formcard", "AboutPage")
        }
    ]

    /**
     * Opens a specific module with the provided initial properties.
     *
     * @param defaultModule The ID of the module to open.
     * @param initialProperties A JavaScript object containing the properties to pass to the module's page.
     */
    function openWithInitialProperties(defaultModule, initialProperties) {
        let module = null;
        for (let i = 0; i < modules.length; i++) {
            if (modules[i].moduleId === defaultModule) {
                module = modules[i];
                break;
            }
        }

        if (module) {
            module.initialProperties = () => {
                return initialProperties;
            };
        }
        root.open(defaultModule);
    }

    /**
     * Convenience method to open the account editor by navigating to the Accounts module
     * and passing the account data via initialProperties.
     *
     * @param accountData Data for the account to edit, or null for a new account.
     */
    function openAccountEditor(accountData = null) {
        let props = {
            isNew: true
        };

        if (accountData) {
            props = {
                isNew: false,
                accountName: accountData.account || "",
                authMethod: accountData.authMethod || "Token",
                region: accountData.region || "Europe",
                launchParameters: accountData.launchParameters || "-w"
            };
        }

        root.openWithInitialProperties("accounts", {
            initialAccountData: props
        });
    }
}
