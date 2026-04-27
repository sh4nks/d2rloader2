pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

FormCard.FormCardPage {
    id: root

    title: i18nc("@title", "Live Information")

    FormCard.FormHeader {
        title: i18nc("@title:group", "D2Emu Integration")
    }

    FormCard.FormCard {
        FormCard.FormTextFieldDelegate {
            label: i18nc("@label", "Username")
            description: i18nc("@info:label", "Your D2Emu.com account username.")
            placeholderText: i18nc("@info:placeholder", "Enter your username...")
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormTextFieldDelegate {
            label: i18nc("@label", "API Key")
            description: i18nc("@info:label", "Your D2Emu.com API key for Terror Zone and Diablo Clone data.")
            placeholderText: i18nc("@info:placeholder", "Enter your API key...")
            echoMode: TextInput.Password
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormCheckDelegate {
            id: autoRefresh
            text: i18nc("@label", "Automatic Refresh")
            description: i18nc("@info:label", "Periodically fetch the latest data from D2Emu.com.")
            checked: true
        }

        FormCard.FormDelegateSeparator {
            visible: autoRefresh.checked
        }

        FormCard.FormSpinBoxDelegate {
            label: i18nc("@label", "Update Interval (minutes)")
            description: i18nc("@info:label", "How often to check for game event updates.")
            visible: autoRefresh.checked
            from: 10
            to: 60
            value: 10
            stepSize: 10
        }
    }

    FormCard.FormHeader {
        title: i18nc("@title:group", "Diablo Clone")
    }

    FormCard.FormCard {
        FormCard.FormCheckDelegate {
            text: i18nc("@label", "Enable DClone Notifications")
            description: i18nc("@info:label", "Receive desktop notifications when Diablo Clone progress changes.")
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormComboBoxDelegate {
            text: i18nc("@label", "Preferred Mode")
            description: i18nc("@info:label", "The game mode to prioritize for notifications and monitoring.")
            model: [i18nc("@item", "Softcore"), i18nc("@item", "Hardcore"), i18nc("@item", "Softcore Ladder"), i18nc("@item", "Hardcore Ladder")]
        }
    }

    FormCard.FormHeader {
        title: i18nc("@title:group", "Terror Zones")
    }

    FormCard.FormCard {
        FormCard.FormCheckDelegate {
            text: i18nc("@label", "Enable TZ Notifications")
            description: i18nc("@info:label", "Receive desktop notifications when a new Terror Zone starts.")
        }
    }
}
