pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

FormCard.FormCardPage {
    id: root

    title: "Diablo Clone Settings"

    FormCard.FormHeader {
        title: "D2Emu Integration"
    }

    FormCard.FormCard {
        FormCard.FormTextFieldDelegate {
            label: "API Key"
            placeholderText: "Enter your D2Emu API key..."
            echoMode: TextInput.Password
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormCheckDelegate {
            text: "Enable Progress Notifications"
            description: "Receive desktop notifications when Diablo Clone progress changes."
        }
    }

    FormCard.FormHeader {
        title: "Monitoring"
    }

    FormCard.FormCard {
        FormCard.FormSpinBoxDelegate {
            label: "Update Interval (minutes)"
            from: 1
            to: 60
            value: 5
            stepSize: 1
        }
    }
}
