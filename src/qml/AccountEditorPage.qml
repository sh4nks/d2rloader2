pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Dialogs
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

FormCard.FormCardPage {
    id: root

    title: root.isNew ? "Add Account" : "Edit Account"

    property bool isNew: true
    property string accountName: ""
    property string authMethod: "Token"
    property string region: "Europe"
    property string launchParameters: "-w"
    property string gameSettings: "Default"
    property string customSettingsPath: ""
    property string protonPath: ""

    // Authentication data
    property string authToken: ""
    property string email: ""
    property string password: ""

    // Helper function to convert URL to local path
    function urlToPath(url) {
        if (!url)
            return "";
        let path = url.toString();
        if (path.startsWith("file://")) {
            path = path.substring(7);
        }
        return decodeURIComponent(path);
    }

    FileDialog {
        id: customSettingsDialog
        title: "Select Custom Settings.json"
        nameFilters: ["JSON files (*.json)", "All files (*)"]
        onAccepted: root.customSettingsPath = root.urlToPath(selectedFile)
    }

    FolderDialog {
        id: protonPathDialog
        title: "Select Proton Runtime Directory"
        onAccepted: root.protonPath = root.urlToPath(selectedFolder)
    }

    FormCard.FormHeader {
        title: "Account Configuration"
    }

    FormCard.FormCard {
        FormCard.FormTextFieldDelegate {
            id: nameField
            label: "Account Name"
            description: "A unique identifier for this account."
            text: root.accountName
            placeholderText: "e.g. MyAccount"
            onTextChanged: root.accountName = text
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormComboBoxDelegate {
            id: authField
            text: "Authentication Method"
            description: "The method used to log in to Battle.net."
            model: ["Token", "Password", "Steam"]
            currentIndex: Math.max(0, model.indexOf(root.authMethod))
            onCurrentIndexChanged: root.authMethod = model[currentIndex]
        }

        // --- Token Conditional Field ---
        FormCard.FormDelegateSeparator {
            visible: tokenField.visible
        }

        FormCard.FormTextFieldDelegate {
            id: tokenField
            label: "Authentication Token"
            description: "The Battle.net login token."
            visible: root.authMethod === "Token"
            text: root.authToken
            placeholderText: "Enter your login token..."
            onTextChanged: root.authToken = text
        }

        // --- Password Conditional Fields ---
        FormCard.FormDelegateSeparator {
            visible: emailField.visible
        }

        FormCard.FormTextFieldDelegate {
            id: emailField
            label: "Email Address"
            description: "Your Battle.net account email."
            visible: root.authMethod === "Password"
            text: root.email
            placeholderText: "example@email.com"
            onTextChanged: root.email = text
        }

        FormCard.FormDelegateSeparator {
            visible: passwordField.visible
        }

        FormCard.FormTextFieldDelegate {
            id: passwordField
            label: "Password"
            description: "Your Battle.net account password."
            visible: root.authMethod === "Password"
            echoMode: TextInput.Password
            text: root.password
            placeholderText: "••••••••"
            onTextChanged: root.password = text
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormComboBoxDelegate {
            id: regionField
            text: "Region"
            description: "The game server region for this account."
            model: ["Europe", "Americas", "Asia"]
            currentIndex: Math.max(0, model.indexOf(root.region))
            onCurrentIndexChanged: root.region = model[currentIndex]
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormTextFieldDelegate {
            id: paramsField
            label: "Launch Parameters"
            description: "Command line arguments passed to the game executable."
            text: root.launchParameters
            placeholderText: "-w -txt"
            onTextChanged: root.launchParameters = text
        }
    }

    FormCard.FormHeader {
        title: "Compatibility"
    }

    FormCard.FormCard {
        FormCard.AbstractFormDelegate {
            contentItem: ColumnLayout {
                spacing: Kirigami.Units.smallSpacing
                QQC2.Label {
                    text: "Proton Runtime"
                }
                QQC2.Label {
                    text: "The directory containing the Proton/Wine compatibility layer for this account."
                    font: Kirigami.Theme.smallFont
                    opacity: 0.7
                    wrapMode: Text.Wrap
                    Layout.fillWidth: true
                }
                RowLayout {
                    QQC2.TextField {
                        id: protonPathField
                        Layout.fillWidth: true
                        text: root.protonPath
                        placeholderText: "e.g. GE-Proton or UMU-Latest"
                        onTextChanged: root.protonPath = text
                    }
                    QQC2.Button {
                        icon.name: "folder-open"
                        onClicked: protonPathDialog.open()
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.text: "Browse..."
                    }
                }
            }
        }
    }

    FormCard.FormHeader {
        title: "Game Settings"
    }

    FormCard.FormCard {
        FormCard.FormComboBoxDelegate {
            id: gameSettingsField
            text: "Settings Profile"
            description: "Select which game settings (Settings.json) to use for this account."
            model: ["Default", "Account Specific", "Custom"]
            currentIndex: Math.max(0, model.indexOf(root.gameSettings))
            onCurrentIndexChanged: root.gameSettings = model[currentIndex]
        }

        FormCard.FormDelegateSeparator {
            visible: customSettingsDelegate.visible
        }

        FormCard.AbstractFormDelegate {
            id: customSettingsDelegate
            visible: root.gameSettings === "Custom"
            contentItem: ColumnLayout {
                spacing: Kirigami.Units.smallSpacing
                QQC2.Label {
                    text: "Custom Settings File"
                }
                QQC2.Label {
                    text: "Path to a specific Settings.json file for this account."
                    font: Kirigami.Theme.smallFont
                    opacity: 0.7
                    wrapMode: Text.Wrap
                    Layout.fillWidth: true
                }
                RowLayout {
                    QQC2.TextField {
                        id: customSettingsPathField
                        Layout.fillWidth: true
                        text: root.customSettingsPath
                        placeholderText: "Path to Settings.json..."
                        onTextChanged: root.customSettingsPath = text
                    }
                    QQC2.Button {
                        icon.name: "document-open"
                        onClicked: customSettingsDialog.open()
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.text: "Browse..."
                    }
                }
            }
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormButtonDelegate {
            text: "Copy Current Settings"
            description: "Copy the game's current global Settings.json to this account's profile."
            icon.name: "edit-copy"
            onClicked: {
                // Logic to copy settings would go here
            }
        }
    }

    FormCard.FormHeader {
        title: "Actions"
    }

    FormCard.FormCard {
        FormCard.FormButtonDelegate {
            id: saveButton
            text: root.isNew ? "Create Account" : "Save Changes"
            icon.name: root.isNew ? "list-add" : "document-save"
            highlighted: true
            onClicked: {
                // Logic to persist the account would go here
                root.Kirigami.PageStack.pageStack.pop();
            }
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormButtonDelegate {
            id: cancelButton
            text: "Cancel"
            icon.name: "dialog-cancel"
            onClicked: root.Kirigami.PageStack.pageStack.pop()
        }
    }
}
