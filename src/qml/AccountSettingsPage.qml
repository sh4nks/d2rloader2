pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

FormCard.FormCardPage {
    id: root

    title: "Accounts"

    /**
     * Property used to pass data when navigating from outside the module.
     * This mimics the pattern used in NeoChat for opening specific sub-pages.
     */
    property var initialAccountData

    onInitialAccountDataChanged: if (initialAccountData) {
        initialAccountTimer.restart();
    }

    Timer {
        id: initialAccountTimer
        interval: 10
        running: false
        onTriggered: {
            root.QQC2.ApplicationWindow.window.pageStack.layers.push(accountEditorComponent, root.initialAccountData);
            root.initialAccountData = null;
        }
    }

    FormCard.FormHeader {
        title: "Accounts"
    }

    FormCard.FormCard {
        // Mocking the account model for the UI prototype
        Repeater {
            model: 4
            delegate: FormCard.AbstractFormDelegate {
                id: accountDelegate
                required property int index

                readonly property string accountName: ["cow", "dog", "sheep", "goat"][accountDelegate.index]
                readonly property string region: "Europe"
                readonly property string authMethod: ["Token", "Token", "Password", "Steam"][accountDelegate.index]

                onClicked: root.QQC2.ApplicationWindow.window.pageStack.layers.push(accountEditorComponent, {
                    isNew: false,
                    accountName: accountDelegate.accountName,
                    authMethod: accountDelegate.authMethod,
                    region: accountDelegate.region
                })

                TapHandler {
                    onDoubleTapped: root.QQC2.ApplicationWindow.window.pageStack.layers.push(accountEditorComponent, {
                        isNew: false,
                        accountName: accountDelegate.accountName,
                        authMethod: accountDelegate.authMethod,
                        region: accountDelegate.region
                    })
                }

                contentItem: RowLayout {
                    spacing: Kirigami.Units.largeSpacing

                    Kirigami.Icon {
                        source: "user-identity"
                        Layout.preferredWidth: Kirigami.Units.iconSizes.medium
                        Layout.preferredHeight: Kirigami.Units.iconSizes.medium
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: Kirigami.Units.smallSpacing

                        QQC2.Label {
                            Layout.fillWidth: true
                            text: accountDelegate.accountName
                            font.bold: true
                            elide: Text.ElideRight
                        }

                        QQC2.Label {
                            Layout.fillWidth: true
                            text: accountDelegate.authMethod + " • " + accountDelegate.region
                            color: Kirigami.Theme.disabledTextColor
                            font: Kirigami.Theme.smallFont
                            elide: Text.ElideRight
                        }
                    }

                    // Reorder Buttons
                    RowLayout {
                        spacing: 0
                        QQC2.ToolButton {
                            icon.name: "arrow-up"
                            enabled: accountDelegate.index > 0
                            onClicked: {
                                // Logic to move account up would go here
                            }
                        }
                        QQC2.ToolButton {
                            icon.name: "arrow-down"
                            enabled: accountDelegate.index < 3
                            onClicked: {
                                // Logic to move account down would go here
                            }
                        }
                    }

                    QQC2.ToolButton {
                        icon.name: "edit-delete"
                        onClicked: {
                            // Logic to remove account would go here
                        }
                    }

                    FormCard.FormArrow {
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    }
                }
            }
        }

        FormCard.FormDelegateSeparator {
            above: addAccountDelegate
        }

        FormCard.FormButtonDelegate {
            id: addAccountDelegate
            text: "Add Account"
            icon.name: "list-add"
            onClicked: root.QQC2.ApplicationWindow.window.pageStack.layers.push(accountEditorComponent, {
                isNew: true
            })
        }
    }

    Component {
        id: accountEditorComponent
        AccountEditorPage {}
    }
}
