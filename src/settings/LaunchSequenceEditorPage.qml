pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.ki18n
import com.someblocks.d2rloader.accounts as Accounts

FormCard.FormCardPage {
    id: root

    /**
     * Row of the sequence being edited, or -1 for a new one.
     */
    property int sequenceIndex: -1
    property string sequenceName
    property var selectedIds: []

    readonly property bool isNew: root.sequenceIndex < 0

    title: root.isNew ? KI18n.i18nc("@title", "New Launch Sequence") : KI18n.i18nc("@title", "Edit Launch Sequence")

    FormCard.FormHeader {
        title: KI18n.i18nc("@title:group", "Sequence Configuration")
    }

    FormCard.FormCard {
        FormCard.FormTextFieldDelegate {
            id: nameField
            label: KI18n.i18nc("@label", "Sequence Name")
            description: KI18n.i18nc("@info:label", "A descriptive name for this launch sequence.")
            text: root.sequenceName
            placeholderText: KI18n.i18nc("@info:placeholder", "e.g. Morning Farming")
        }
    }

    FormCard.FormHeader {
        title: KI18n.i18nc("@title:group", "Select Accounts")
    }

    FormCard.FormCard {
        Repeater {
            model: Accounts.ProfileManager
            delegate: FormCard.FormCheckDelegate {
                id: accountCheck
                required property int profileId
                required property var profile

                text: accountCheck.profile.profileName
                description: Accounts.AuthMethodModel.getDisplayName(accountCheck.profile.authMethod) + " - " + Accounts.RegionModel.getDisplayName(accountCheck.profile.region)
                checked: root.selectedIds.includes(accountCheck.profileId)
                onToggled: {
                    if (accountCheck.checked) {
                        root.selectedIds = root.selectedIds.concat([accountCheck.profileId]);
                    } else {
                        root.selectedIds = root.selectedIds.filter(id => id !== accountCheck.profileId);
                    }
                }
            }
        }

        FormCard.FormTextDelegate {
            visible: !Accounts.ProfileManager.hasProfiles
            text: KI18n.i18nc("@info", "There are no accounts yet.")
        }
    }

    FormCard.FormSectionText {
        text: KI18n.i18nc("@info", "Accounts start in the order of the accounts table.")
    }

    FormCard.FormHeader {
        title: KI18n.i18nc("@title:group", "Actions")
    }

    FormCard.FormCard {
        FormCard.FormButtonDelegate {
            text: root.isNew ? KI18n.i18nc("@action:button", "Create Sequence") : KI18n.i18nc("@action:button", "Save Changes")
            icon.name: root.isNew ? "list-add" : "document-save"
            enabled: nameField.text.trim().length > 0
            onClicked: {
                Accounts.LaunchSequenceManager.save(root.sequenceIndex, nameField.text.trim(), root.selectedIds);
                (root.QQC2.ApplicationWindow.window as Kirigami.ApplicationWindow).pageStack.layers.pop();
            }
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormButtonDelegate {
            text: KI18n.i18nc("@action:button", "Cancel")
            icon.name: "dialog-cancel"
            onClicked: (root.QQC2.ApplicationWindow.window as Kirigami.ApplicationWindow).pageStack.layers.pop()
        }
    }
}
