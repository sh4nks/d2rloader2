pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

FormCard.FormCardPage {
    id: root

    title: i18nc("@title", "Accounts")

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
        title: i18nc("@title:group", "Accounts")
    }

    FormCard.FormCard {
        // Mocking the account model for the UI prototype
        Repeater {
            model: 4
            delegate: FormCard.AbstractFormDelegate {
                id: accountDelegate
                required property int index

                readonly property string accountName: [i18nc("@info", "cow"), i18nc("@info", "dog"), i18nc("@info", "sheep"), i18nc("@info", "goat")][accountDelegate.index]
                readonly property string region: i18nc("@info", "Europe")
                readonly property string authMethod: [i18nc("@info", "Token"), i18nc("@info", "Token"), i18nc("@info", "Password"), i18nc("@info", "Steam")][accountDelegate.index]

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
                            QQC2.ToolTip.visible: hovered && enabled
                            QQC2.ToolTip.text: i18nc("@info:tooltip", "Move Up")
                            enabled: accountDelegate.index > 0
                            onClicked: {
                                // Logic to move account up would go here
                            }
                        }
                        QQC2.ToolButton {
                            icon.name: "arrow-down"
                            QQC2.ToolTip.visible: hovered && enabled
                            QQC2.ToolTip.text: i18nc("@info:tooltip", "Move Down")
                            enabled: accountDelegate.index < 3
                            onClicked: {
                                // Logic to move account down would go here
                            }
                        }
                    }

                    QQC2.ToolButton {
                        icon.name: "edit-copy"
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.text: i18nc("@info:tooltip", "Clone Account")
                        onClicked: {
                            // Logic to clone account would go here
                        }
                    }

                    QQC2.ToolButton {
                        icon.name: "edit-delete"
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.text: i18nc("@info:tooltip", "Delete Account")
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
            text: i18nc("@action:button", "Add Account")
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
