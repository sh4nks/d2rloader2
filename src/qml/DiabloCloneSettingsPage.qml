pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

FormCard.FormCardPage {
    id: root

    title: "Diablo Clone and Terror Zone Information"

    FormCard.FormHeader {
        title: "D2Emu Integration"
    }

    FormCard.FormCard {
        FormCard.FormTextFieldDelegate {
            label: "Username"
            description: "Your D2Emu.com account username."
            placeholderText: "Enter your username..."
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormTextFieldDelegate {
            label: "API Key"
            description: "Your D2Emu.com API key for Terror Zone and Diablo Clone data."
            placeholderText: "Enter your API key..."
            echoMode: TextInput.Password
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormCheckDelegate {
            id: autoRefresh
            text: "Automatic Refresh"
            description: "Periodically fetch the latest data from D2Emu.com."
            checked: true
        }

        FormCard.FormDelegateSeparator {
            visible: autoRefresh.checked
        }

        FormCard.FormSpinBoxDelegate {
            label: "Update Interval (minutes)"
            description: "How often to check for game event updates."
            visible: autoRefresh.checked
            from: 10
            to: 60
            value: 10
            stepSize: 10
        }
    }

    FormCard.FormHeader {
        title: "Diablo Clone"
    }

    FormCard.FormCard {
        FormCard.FormCheckDelegate {
            text: "Enable DClone Notifications"
            description: "Receive desktop notifications when Diablo Clone progress changes."
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormComboBoxDelegate {
            text: "Preferred Mode"
            description: "The game mode to prioritize for notifications and monitoring."
            model: ["Softcore", "Hardcore", "Softcore Ladder", "Hardcore Ladder"]
        }
    }

    FormCard.FormHeader {
        title: "Terror Zones"
    }

    FormCard.FormCard {
        FormCard.FormCheckDelegate {
            text: "Enable TZ Notifications"
            description: "Receive desktop notifications when a new Terror Zone starts."
        }
    }
}
