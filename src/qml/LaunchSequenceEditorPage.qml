pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.ki18n

FormCard.FormCardPage {
    id: root

    title: root.isNew ? i18nc("@title", "New Launch Sequence") : i18nc("@title", "Edit Launch Sequence")

    property bool isNew: true
    property string sequenceName: ""
    property var selectedAccounts: []

    FormCard.FormHeader {
        title: i18nc("@title:group", "Sequence Configuration")
    }

    FormCard.FormCard {
        FormCard.FormTextFieldDelegate {
            id: nameField
            label: i18nc("@label", "Sequence Name")
            description: i18nc("@info:label", "A descriptive name for this launch sequence.")
            text: root.sequenceName
            placeholderText: i18nc("@info:placeholder", "e.g. Morning Farming")
            onTextChanged: root.sequenceName = text
        }
    }

    FormCard.FormHeader {
        title: i18nc("@title:group", "Select Accounts")
    }

    FormCard.FormCard {
        Repeater {
            model: 6 // Mocking available accounts for the UI prototype
            delegate: FormCard.FormCheckDelegate {
                id: accountCheck
                required property int index

                text: [i18nc("@info", "cow"), i18nc("@info", "dog"), i18nc("@info", "sheep"), i18nc("@info", "goat"), i18nc("@info", "BooBoo"), i18nc("@info", "MFer")][accountCheck.index]
                description: i18nc("@info", "Account ID: %1", (index + 1))

                checked: false
                onToggled: {
                    // Logic to update selectedAccounts list
                }
            }
        }
    }

    FormCard.FormHeader {
        title: KI18n.i18nc("@title:group", "Actions")
    }

    FormCard.FormCard {
        FormCard.FormButtonDelegate {
            text: root.isNew ? i18nc("@action:button", "Create Sequence") : i18nc("@action:button", "Save Changes")
            icon.name: root.isNew ? "list-add" : "document-save"
            highlighted: true
            onClicked: {
                // Logic to persist the sequence would go here
                root.QQC2.ApplicationWindow.window.pageStack.layers.pop();
            }
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormButtonDelegate {
            text: i18nc("@action:button", "Cancel")
            icon.name: "dialog-cancel"
            onClicked: root.QQC2.ApplicationWindow.window.pageStack.layers.pop()
        }
    }
}
