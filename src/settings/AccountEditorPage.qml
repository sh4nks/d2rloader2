pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Dialogs
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import com.someblocks.d2rloader as D2R

FormCard.FormCardPage {
    id: root

    title: root.isNew ? i18nc("@title", "Add Account") : i18nc("@title", "Edit Account")

    property D2R.Profile profile

    property bool isNew: true

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
        title: i18nc("@title:window", "Select Custom Settings.json")
        nameFilters: ["JSON files (*.json)", "All files (*)"]
        onAccepted: root.profile.gameSettingsPath = root.urlToPath(selectedFile)
    }

    FolderDialog {
        id: protonPathDialog
        title: i18nc("@title:window", "Select Proton Runtime Directory")
        onAccepted: root.profile.protonPath = root.urlToPath(selectedFolder)
    }

    FormCard.FormHeader {
        title: i18nc("@title:group", "Account Configuration")
    }

    FormCard.FormCard {
        FormCard.FormTextFieldDelegate {
            id: nameField
            label: i18nc("@label", "Account Name")
            description: i18nc("@info:label", "A unique identifier for this account.")
            text: root.profile.profileName
            placeholderText: i18nc("@info:placeholder", "e.g. MyAccount")
            onTextChanged: root.profile.profileName = text
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormComboBoxDelegate {
            id: authField
            text: i18nc("@label", "Authentication Method")
            description: i18nc("@info:label", "The method used to log in to Battle.net.")
            model: D2R.AuthMethodModel

            textRole: "name"
            valueRole: "value"

            Component.onCompleted: {
                authField.currentIndex = authField.indexOfValue(root.profile.authMethod);
            }

            onActivated: {
                console.log("Selected Name: " + currentText);
                console.log("Selected Raw Enum Value: " + currentValue);
                root.profile.authMethod = currentValue;
            }
        }

        FormCard.FormDelegateSeparator {
            visible: tokenField.visible
        }

        FormCard.FormTextFieldDelegate {
            id: tokenField
            label: i18nc("@label", "Authentication Token")
            description: i18nc("@info:label", "The Battle.net login token.")
            visible: root.profile.authMethod === D2R.AuthMethodModel.Token
            text: root.profile.token
            placeholderText: i18nc("@info:placeholder", "Enter your login token...")
            onTextChanged: root.profile.token = text
        }

        // --- Password Conditional Fields ---
        FormCard.FormDelegateSeparator {
            visible: emailField.visible
        }

        FormCard.FormTextFieldDelegate {
            id: emailField
            label: i18nc("@label", "Battle.net Email Address")
            description: i18nc("@info:label", "Your Battle.net account email.")
            visible: root.profile.authMethod === D2R.AuthMethodModel.Password
            text: root.profile.email
            placeholderText: i18nc("@info:placeholder", "example@email.com")
            onTextChanged: root.profile.email = text
        }

        FormCard.FormDelegateSeparator {
            visible: passwordField.visible
        }

        FormCard.FormPasswordFieldDelegate {
            id: passwordField
            label: i18nc("@label", "Battle.net Password")
            visible: root.profile.authMethod === D2R.AuthMethodModel.Password
            text: root.profile.password
            onTextChanged: root.profile.password = text
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormComboBoxDelegate {
            id: regionField
            text: i18nc("@label", "Region")
            description: i18nc("@info:label", "The game server region for this account.")
            model: D2R.RegionModel

            textRole: "name"
            valueRole: "value"

            Component.onCompleted: {
                regionField.currentIndex = regionField.indexOfValue(root.profile.region);
            }

            onActivated: {
                console.log("Selected Name: " + currentText);
                console.log("Selected Raw Enum Value: " + currentValue);
                root.profile.region = currentValue;
            }
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormTextFieldDelegate {
            id: paramsField
            label: i18nc("@label", "Launch Parameters")
            description: i18nc("@info:label", "Command line arguments passed to the game executable.")
            text: root.profile.gameParameters
            placeholderText: i18nc("@info:placeholder", "-w -txt")
            onTextChanged: root.profile.gameParameters = text
        }
    }

    FormCard.FormHeader {
        title: i18nc("@title:group", "Compatibility")
    }

    FormCard.FormCard {
        FormCard.AbstractFormDelegate {
            contentItem: ColumnLayout {
                spacing: Kirigami.Units.smallSpacing
                QQC2.Label {
                    text: i18nc("@label", "Proton Runtime")
                }
                QQC2.Label {
                    text: i18nc("@info:label", "The directory containing the Proton/Wine compatibility layer for this account.")
                    font: Kirigami.Theme.smallFont
                    opacity: 0.7
                    wrapMode: Text.Wrap
                    Layout.fillWidth: true
                }
                RowLayout {
                    QQC2.TextField {
                        id: protonPathField
                        Layout.fillWidth: true
                        text: root.profile.protonPath
                        placeholderText: i18nc("@info:placeholder", "e.g. GE-Proton or UMU-Latest")
                        onTextChanged: root.profile.protonPath = text
                    }
                    QQC2.Button {
                        icon.name: "folder-open"
                        onClicked: protonPathDialog.open()
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.text: i18nc("@info:tooltip", "Browse...")
                    }
                }
            }
        }
    }

    FormCard.FormHeader {
        title: i18nc("@title:group", "Game Settings")
    }

    FormCard.FormCard {
        FormCard.FormComboBoxDelegate {
            id: gameSettingsField
            text: i18nc("@label", "Settings Profile")
            description: i18nc("@info:label", "Select which game settings (Settings.json) to use for this account.")
            model: [i18nc("@item", "Default"), i18nc("@item", "Account Specific"), i18nc("@item", "Custom")]
            // currentIndex: Math.max(0, model.indexOf(root.gameSettings))
            // onCurrentIndexChanged: root.gameSettings = model[currentIndex]
        }

        FormCard.FormDelegateSeparator {
            visible: customSettingsDelegate.visible
        }

        FormCard.AbstractFormDelegate {
            id: customSettingsDelegate
            // visible: root.gameSettings === "Custom"
            visible: true
            contentItem: ColumnLayout {
                spacing: Kirigami.Units.smallSpacing
                QQC2.Label {
                    text: i18nc("@label", "Custom Settings File")
                }
                QQC2.Label {
                    text: i18nc("@info:label", "Path to a specific Settings.json file for this account.")
                    font: Kirigami.Theme.smallFont
                    opacity: 0.7
                    wrapMode: Text.Wrap
                    Layout.fillWidth: true
                }
                RowLayout {
                    QQC2.TextField {
                        id: customSettingsPathField
                        Layout.fillWidth: true
                        text: root.profile.gameSettingsPath
                        placeholderText: i18nc("@info:placeholder", "Path to Settings.json...")
                        onTextChanged: root.profile.gameSettingsPath = text
                    }
                    QQC2.Button {
                        icon.name: "document-open"
                        onClicked: customSettingsDialog.open()
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.text: i18nc("@info:tooltip", "Browse...")
                    }
                }
            }
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormButtonDelegate {
            text: i18nc("@action:button", "Copy Current Settings")
            description: i18nc("@info:label", "Copy the game's current global Settings.json to this account's profile.")
            icon.name: "edit-copy"
            onClicked: {
                // Logic to copy settings would go here
            }
        }
    }

    FormCard.FormHeader {
        title: i18nc("@title:group", "Actions")
    }

    FormCard.FormCard {
        FormCard.FormButtonDelegate {
            id: saveButton
            text: root.isNew ? i18nc("@action:button", "Create Account") : i18nc("@action:button", "Save Changes")
            icon.name: root.isNew ? "list-add" : "document-save"
            highlighted: true
            onClicked: {
                D2R.ProfileManager.save(root.profile);
                root.Kirigami.PageStack.pageStack.pop();
            }
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormButtonDelegate {
            id: cancelButton
            text: i18nc("@action:button", "Cancel")
            icon.name: "dialog-cancel"
            onClicked: root.Kirigami.PageStack.pageStack.pop()
        }
    }
}
