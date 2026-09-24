pragma ComponentBehavior: Bound

import QtQuick
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.ki18n
import com.someblocks.d2rloader.core

FormCard.FormCardPage {
    id: root

    title: KI18n.i18nc("@title", "Live Information")

    FormCard.FormHeader {
        title: KI18n.i18nc("@title:group", "D2Emu Integration")
    }

    FormCard.FormCard {
        FormCard.FormComboBoxDelegate {
            id: providerField
            text: KI18n.i18nc("@label", "Data Provider")
            description: KI18n.i18nc("@info:label", "Where Terror Zone and Diablo Clone data is fetched from.")
            model: [KI18n.i18nc("@item:inlistbox", "D2RLoader Service"), KI18n.i18nc("@item:inlistbox", "d2emu.com directly")]
            currentIndex: D2RLoaderConfig.useD2RInfo ? 0 : 1
            onActivated: {
                D2RLoaderConfig.useD2RInfo = currentIndex === 0;
                D2RLoaderConfig.save();
            }
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormTextFieldDelegate {
            label: KI18n.i18nc("@label", "Username")
            description: KI18n.i18nc("@info:label", "Your D2Emu.com account username.")
            placeholderText: KI18n.i18nc("@info:placeholder", "Enter your username...")
            enabled: !D2RLoaderConfig.useD2RInfo
            text: D2RLoaderConfig.d2emuUser
            onEditingFinished: {
                D2RLoaderConfig.d2emuUser = text;
                D2RLoaderConfig.save();
            }
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormTextFieldDelegate {
            label: KI18n.i18nc("@label", "API Key")
            description: KI18n.i18nc("@info:label", "Your D2Emu.com API key for Terror Zone and Diablo Clone data.")
            placeholderText: KI18n.i18nc("@info:placeholder", "Enter your API key...")
            echoMode: TextInput.Password
            enabled: !D2RLoaderConfig.useD2RInfo
            text: D2RLoaderConfig.d2emuToken
            onEditingFinished: {
                D2RLoaderConfig.d2emuToken = text;
                D2RLoaderConfig.save();
            }
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormCheckDelegate {
            id: autoRefresh
            text: KI18n.i18nc("@label", "Automatic Refresh")
            description: KI18n.i18nc("@info:label", "Periodically fetch the latest data from D2Emu.com.")
            checked: D2RLoaderConfig.autoRefresh
            onToggled: {
                D2RLoaderConfig.autoRefresh = checked;
                D2RLoaderConfig.save();
            }
        }

        FormCard.FormDelegateSeparator {
            visible: autoRefresh.checked
        }

        FormCard.FormSpinBoxDelegate {
            label: KI18n.i18nc("@label", "Update Interval (minutes)")
            description: KI18n.i18nc("@info:label", "How often to check for game event updates.")
            visible: autoRefresh.checked
            from: 10
            to: 60
            stepSize: 10
            value: D2RLoaderConfig.refreshInterval
            onValueChanged: if (value !== D2RLoaderConfig.refreshInterval) {
                D2RLoaderConfig.refreshInterval = value;
                D2RLoaderConfig.save();
            }
        }
    }

    FormCard.FormHeader {
        title: KI18n.i18nc("@title:group", "Diablo Clone")
    }

    FormCard.FormCard {
        FormCard.FormCheckDelegate {
            id: dcloneNotifications
            text: KI18n.i18nc("@label", "Enable DClone Notifications")
            description: KI18n.i18nc("@info:label", "Receive desktop notifications when Diablo Clone progress rises on a listed realm.")
            checked: D2RLoaderConfig.dcloneNotifications
            onToggled: {
                D2RLoaderConfig.dcloneNotifications = checked;
                D2RLoaderConfig.save();
            }
        }

        FormCard.FormDelegateSeparator {
            visible: dcloneNotifications.checked
        }

        FormCard.FormSpinBoxDelegate {
            label: KI18n.i18nc("@label", "Notification Threshold")
            description: KI18n.i18nc("@info:label", "Only notify once a realm reaches this progress out of 6.")
            visible: dcloneNotifications.checked
            from: 1
            to: 6
            value: D2RLoaderConfig.dcloneThreshold
            onValueChanged: if (value !== D2RLoaderConfig.dcloneThreshold) {
                D2RLoaderConfig.dcloneThreshold = value;
                D2RLoaderConfig.save();
            }
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormComboBoxDelegate {
            id: modeField
            text: KI18n.i18nc("@label", "Preferred Mode")
            description: KI18n.i18nc("@info:label", "Which realms the Diablo Clone tab lists, and the mode to prioritize for notifications.")
            model: [KI18n.i18nc("@item", "Softcore"), KI18n.i18nc("@item", "Hardcore"), KI18n.i18nc("@item", "Softcore Ladder"), KI18n.i18nc("@item", "Hardcore Ladder")]
            currentIndex: D2RLoaderConfig.dcloneMode
            onActivated: {
                D2RLoaderConfig.dcloneMode = currentIndex;
                D2RLoaderConfig.save();
            }
        }
    }

    FormCard.FormHeader {
        title: KI18n.i18nc("@title:group", "Terror Zones")
    }

    FormCard.FormCard {
        FormCard.FormCheckDelegate {
            text: KI18n.i18nc("@label", "Enable TZ Notifications")
            description: KI18n.i18nc("@info:label", "Receive desktop notifications when a new Terror Zone starts and when the next one is predicted.")
            checked: D2RLoaderConfig.tzNotifications
            onToggled: {
                D2RLoaderConfig.tzNotifications = checked;
                D2RLoaderConfig.save();
            }
        }
    }
}
