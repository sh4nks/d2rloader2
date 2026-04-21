pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

FormCard.FormCardPage {
    id: root

    title: "Account Settings"

    FormCard.FormHeader {
        title: "Defaults"
    }

    FormCard.FormCard {
        FormCard.FormComboBoxDelegate {
            text: "Default Region"
            description: "The region selected by default for new accounts."
            model: ["Europe", "Americas", "Asia"]
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormTextFieldDelegate {
            label: "Default Parameters"
            placeholderText: "-w -txt"
        }
    }

    FormCard.FormHeader {
        title: "Automation"
    }

    FormCard.FormCard {
        FormCard.FormCheckDelegate {
            text: "Multi-Client Support"
            description: "Allow launching multiple game instances simultaneously."
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormCheckDelegate {
            text: "Auto-Login"
            description: "Automatically log in to the last used account on startup."
        }
    }
}
