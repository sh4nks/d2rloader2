pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

FormCard.FormCardPage {
    id: root

    title: "Game Settings Assignment"

    FormCard.FormHeader {
        title: "Account Settings Mapping"
    }

    FormCard.FormCard {
        Layout.fillWidth: true

        // Using a custom delegate to show a table-like list of accounts and their settings
        Repeater {
            id: assignmentRepeater
            model: 4 // Mocking the account list for the UI prototype

            delegate: FormCard.AbstractFormDelegate {
                id: assignmentDelegate
                required property int index

                readonly property string accountName: ["BooBoo", "MFer", "Filler1", "Mule"][assignmentDelegate.index]

                contentItem: RowLayout {
                    spacing: Kirigami.Units.largeSpacing

                    Kirigami.Icon {
                        source: "user-identity"
                        Layout.preferredWidth: Kirigami.Units.iconSizes.small
                        Layout.preferredHeight: Kirigami.Units.iconSizes.small
                    }

                    QQC2.Label {
                        text: assignmentDelegate.accountName
                        font.bold: true
                        Layout.fillWidth: true
                    }

                    QQC2.ComboBox {
                        id: settingsSelector
                        model: ["Default", "Account Specific", "Custom"]
                        Layout.preferredWidth: Kirigami.Units.gridUnit * 10

                        onCurrentIndexChanged: {
                            // Logic to update the account's settings type would go here
                        }
                    }

                    QQC2.Button {
                        icon.name: "edit-copy"
                        flat: true
                        visible: settingsSelector.currentText !== "Custom"
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.text: "Copy current global Settings.json to this account"
                        onClicked: {
                            // Logic to copy settings
                        }
                    }

                    QQC2.Button {
                        icon.name: "document-open"
                        flat: true
                        visible: settingsSelector.currentText === "Custom"
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.text: "Select custom Settings.json file"
                        onClicked: {
                            // Logic to open file dialog
                        }
                    }
                }
            }
        }
    }

    FormCard.FormHeader {
        title: "Actions"
    }

    FormCard.FormCard {
        FormCard.FormButtonDelegate {
            text: "Apply to All Accounts"
            description: "Set the currently selected settings profile for all accounts in the list."
            icon.name: "dialog-ok-apply"
            onClicked: {
                // Logic for bulk assignment
            }
        }
    }
}
