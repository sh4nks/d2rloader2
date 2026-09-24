pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.ki18n
import com.someblocks.d2rloader.accounts as Accounts

FormCard.FormCardPage {
    id: root

    title: KI18n.i18nc("@title", "Accounts")

    /**
     * Property used to pass data when navigating from outside the module.
     * This mimics the pattern used in NeoChat for opening specific sub-pages.
     */
    property var accountData

    onAccountDataChanged: if (root.accountData) {
        Qt.callLater(root.showAccountEditor);
    }

    /**
     * Opens the editor for the account this page was navigated to with,
     * deferred until the page itself is on the stack.
     */
    function showAccountEditor() {
        if (!root.accountData) {
            return;
        }
        (root.QQC2.ApplicationWindow.window as Kirigami.ApplicationWindow).pageStack.layers.push(accountEditorComponent, root.accountData);
        root.accountData = null;
    }

    /**
     * Opens the editor on a copy of the account at the given index, so that
     * cancelling leaves the stored account untouched.
     */
    function editAccount(index) {
        (root.QQC2.ApplicationWindow.window as Kirigami.ApplicationWindow).pageStack.layers.push(accountEditorComponent, {
            isNew: false,
            profile: Accounts.ProfileManager.editDraft(index)
        });
    }

    /**
     * Opens the editor on a new account that is only added once it is saved.
     */
    function addAccount() {
        (root.QQC2.ApplicationWindow.window as Kirigami.ApplicationWindow).pageStack.layers.push(accountEditorComponent, {
            isNew: true,
            profile: Accounts.ProfileManager.createDraft()
        });
    }

    FormCard.FormHeader {
        title: KI18n.i18nc("@title:group", "Accounts")
    }

    FormCard.FormCard {
        // Mocking the account model for the UI prototype
        Repeater {
            model: Accounts.ProfileManager

            delegate: FormCard.AbstractFormDelegate {
                id: accountDelegate
                required property int index
                required property var profile

                onClicked: root.editAccount(accountDelegate.index)

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
                            text: accountDelegate.profile.profileName
                            font.bold: true
                            elide: Text.ElideRight
                        }

                        QQC2.Label {
                            Layout.fillWidth: true
                            text: Accounts.AuthMethodModel.getDisplayName(accountDelegate.profile.authMethod) + " • " + Accounts.RegionModel.getDisplayName(accountDelegate.profile.region)
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
                            QQC2.ToolTip.text: KI18n.i18nc("@info:tooltip", "Move Up")
                            enabled: accountDelegate.index > 0
                            onClicked: Accounts.ProfileManager.moveUp(accountDelegate.index)
                        }
                        QQC2.ToolButton {
                            icon.name: "arrow-down"
                            QQC2.ToolTip.visible: hovered && enabled
                            QQC2.ToolTip.text: KI18n.i18nc("@info:tooltip", "Move Down")
                            enabled: accountDelegate.index < Accounts.ProfileManager.rowCount() - 1
                            onClicked: Accounts.ProfileManager.moveDown(accountDelegate.index)
                        }
                    }

                    QQC2.ToolButton {
                        icon.name: "edit-copy"
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.text: KI18n.i18nc("@info:tooltip", "Clone Account")
                        onClicked: Accounts.ProfileManager.cloneProfile(accountDelegate.index)
                    }

                    QQC2.ToolButton {
                        icon.name: "edit-delete"
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.text: KI18n.i18nc("@info:tooltip", "Delete Account")
                        onClicked: {
                            deletePrompt.row = accountDelegate.index;
                            deletePrompt.accountName = accountDelegate.profile.profileName;
                            deletePrompt.open();
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
            text: KI18n.i18nc("@action:button", "Add Account")
            icon.name: "list-add"
            onClicked: root.addAccount()
        }
    }

    Kirigami.PromptDialog {
        id: deletePrompt

        property int row: -1
        property string accountName

        title: KI18n.i18nc("@title:window", "Delete Account")
        subtitle: KI18n.i18nc("@info", "Delete the account \"%1\"? This cannot be undone.", deletePrompt.accountName)
        standardButtons: Kirigami.Dialog.Cancel
        showCloseButton: false

        customFooterActions: [
            Kirigami.Action {
                text: KI18n.i18nc("@action:button", "Delete")
                icon.name: "edit-delete"
                onTriggered: {
                    Accounts.ProfileManager.removeProfileAt(deletePrompt.row);
                    deletePrompt.close();
                }
            }
        ]
    }

    Component {
        id: accountEditorComponent
        AccountEditorPage {}
    }
}
