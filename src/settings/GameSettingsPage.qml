pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Dialogs
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.ki18n
import com.someblocks.d2rloader.accounts as Accounts

FormCard.FormCardPage {
    id: root

    title: KI18n.i18nc("@title", "Game Settings Assignment")

    // Files may have been put into the settings folder by hand.
    Component.onCompleted: Accounts.GameSettings.refresh()

    readonly property var settingsTypes: [
        {
            name: KI18n.i18nc("@item game settings", "Default"),
            value: Accounts.GameSettingsType.None
        },
        {
            name: KI18n.i18nc("@item game settings", "Account Specific"),
            value: Accounts.GameSettingsType.Profile
        },
        {
            name: KI18n.i18nc("@item game settings", "Custom"),
            value: Accounts.GameSettingsType.Custom
        }
    ]

    function urlToPath(url) {
        if (!url)
            return "";
        let path = url.toString();
        if (path.startsWith("file://")) {
            path = path.substring(7);
        }
        return decodeURIComponent(path);
    }

    /**
     * Copies the game's current settings of the accounts in the given rows,
     * asking first when that would replace settings they already have.
     */
    function requestCopy(rows) {
        const existing = rows.map(row => Accounts.ProfileManager.getProfile(row)).filter(profile => Accounts.GameSettings.profileSettingsExist(profile)).map(profile => profile.profileName);
        if (existing.length === 0) {
            root.copyCurrentSettings(rows);
            return;
        }
        overwritePrompt.rows = rows;
        overwritePrompt.accountNames = existing;
        overwritePrompt.open();
    }

    function copyCurrentSettings(rows) {
        const failures = [];
        for (const row of rows) {
            const profile = Accounts.ProfileManager.getProfile(row);
            const error = Accounts.GameSettings.copyCurrent(profile, true);
            if (error.length > 0) {
                failures.push(KI18n.i18nc("@info account name and why copying its settings failed", "%1: %2", profile.profileName, error));
                continue;
            }
            Accounts.ProfileManager.setData(Accounts.ProfileManager.index(row, 0), Accounts.GameSettingsType.Profile, Accounts.ProfileManager.GameSettingsRole);
        }

        if (failures.length > 0) {
            message.type = Kirigami.MessageType.Error;
            message.text = failures.join("\n");
        } else {
            message.type = Kirigami.MessageType.Positive;
            message.text = KI18n.i18ncp("@info", "Copied the game's current settings for %1 account.", "Copied the game's current settings for %1 accounts.", rows.length);
        }
    }

    function accountsUsing(name) {
        const names = [];
        for (let row = 0; row < Accounts.ProfileManager.rowCount(); ++row) {
            const profile = Accounts.ProfileManager.getProfile(row);
            // Which file an account uses also follows these, which uses()
            // cannot report as dependencies.
            profile.profileName;
            profile.gameSettings;
            profile.gameSettingsPath;
            if (Accounts.GameSettings.uses(profile, name)) {
                names.push(profile.profileName);
            }
        }
        return names;
    }

    function removeFile(name) {
        // uses() cannot tell once the file is gone.
        const rows = [];
        for (let row = 0; row < Accounts.ProfileManager.rowCount(); ++row) {
            if (Accounts.GameSettings.uses(Accounts.ProfileManager.getProfile(row), name)) {
                rows.push(row);
            }
        }

        const error = Accounts.GameSettings.remove(name);
        if (error.length > 0) {
            message.type = Kirigami.MessageType.Error;
            message.text = error;
            return;
        }
        for (const row of rows) {
            Accounts.ProfileManager.setData(Accounts.ProfileManager.index(row, 0), Accounts.GameSettingsType.None, Accounts.ProfileManager.GameSettingsRole);
        }
        message.type = Kirigami.MessageType.Positive;
        message.text = KI18n.i18nc("@info", "Removed \"%1\".", name);
    }

    FileDialog {
        id: customSettingsDialog

        property int targetRow: -1

        title: KI18n.i18nc("@title:window", "Select Custom Settings.json")
        nameFilters: ["JSON files (*.json)", "All files (*)"]
        onAccepted: Accounts.ProfileManager.setData(Accounts.ProfileManager.index(customSettingsDialog.targetRow, 0), root.urlToPath(selectedFile), Accounts.ProfileManager.GameSettingsPathRole)
    }

    Kirigami.PromptDialog {
        id: overwritePrompt

        property var rows: []
        property var accountNames: []

        title: KI18n.i18nc("@title:window", "Overwrite Settings")
        subtitle: KI18n.i18ncp("@info", "The account \"%2\" already has its own settings. Replace them with the game's current settings?", "These accounts already have their own settings: %2. Replace them with the game's current settings?", overwritePrompt.accountNames.length, overwritePrompt.accountNames.join(", "))
        standardButtons: Kirigami.Dialog.Cancel
        showCloseButton: false

        customFooterActions: [
            Kirigami.Action {
                text: KI18n.i18nc("@action:button", "Overwrite")
                icon.name: "document-replace"
                onTriggered: {
                    root.copyCurrentSettings(overwritePrompt.rows);
                    overwritePrompt.close();
                }
            }
        ]
    }

    Kirigami.PromptDialog {
        id: removePrompt

        property string fileName
        property var accountNames: []

        title: KI18n.i18nc("@title:window", "Remove Game Settings")
        subtitle: removePrompt.accountNames.length > 0 ? KI18n.i18ncp("@info", "Remove \"%2\"? The account %3 goes back to the game's own settings.", "Remove \"%2\"? The accounts %3 go back to the game's own settings.", removePrompt.accountNames.length, removePrompt.fileName, removePrompt.accountNames.join(", ")) : KI18n.i18nc("@info", "Remove \"%1\"?", removePrompt.fileName)
        standardButtons: Kirigami.Dialog.Cancel
        showCloseButton: false

        customFooterActions: [
            Kirigami.Action {
                text: KI18n.i18nc("@action:button", "Remove")
                icon.name: "edit-delete"
                onTriggered: {
                    root.removeFile(removePrompt.fileName);
                    removePrompt.close();
                }
            }
        ]
    }

    FormCard.FormHeader {
        title: KI18n.i18nc("@title:group", "Account Settings Mapping")
    }

    FormCard.FormCard {
        Layout.fillWidth: true

        Repeater {
            id: assignmentRepeater
            model: Accounts.ProfileManager

            delegate: FormCard.AbstractFormDelegate {
                id: assignmentDelegate
                required property var model
                required property int index

                readonly property bool isCustom: assignmentDelegate.model.gameSettings === Accounts.GameSettingsType.Custom
                readonly property string settingsPath: {
                    // The path also follows the name and auth method, which the
                    // call cannot report as dependencies.
                    assignmentDelegate.model.profileName;
                    assignmentDelegate.model.authMethod;
                    return Accounts.GameSettings.settingsPath(assignmentDelegate.model.profile, assignmentDelegate.model.gameSettings);
                }

                contentItem: GridLayout {
                    columns: 3
                    columnSpacing: Kirigami.Units.largeSpacing
                    rowSpacing: Kirigami.Units.smallSpacing

                    Kirigami.Icon {
                        source: "user-identity"
                        Layout.preferredWidth: Kirigami.Units.iconSizes.small
                        Layout.preferredHeight: Kirigami.Units.iconSizes.small
                    }

                    QQC2.Label {
                        text: assignmentDelegate.model.profileName
                        font.bold: true
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }

                    QQC2.ComboBox {
                        id: settingsSelector
                        model: root.settingsTypes
                        textRole: "name"
                        valueRole: "value"
                        Layout.preferredWidth: Kirigami.Units.gridUnit * 10
                        // indexOfValue() does not register the model as a binding dependency,
                        // so reading count first is what makes this re-evaluate once the model
                        // is populated.
                        currentIndex: settingsSelector.count > 0 ? settingsSelector.indexOfValue(assignmentDelegate.model.gameSettings) : -1

                        onActivated: assignmentDelegate.model.gameSettings = settingsSelector.currentValue
                    }

                    // The file in use, lined up under the account name.
                    RowLayout {
                        Layout.row: 1
                        Layout.column: 1
                        Layout.columnSpan: 2
                        Layout.fillWidth: true
                        spacing: Kirigami.Units.smallSpacing

                        QQC2.Label {
                            id: pathLabel
                            visible: !assignmentDelegate.isCustom
                            Layout.fillWidth: true
                            text: assignmentDelegate.settingsPath || KI18n.i18nc("@info", "Could not find the Steam installation the game runs in.")
                            font: Kirigami.Theme.smallFont
                            opacity: 0.7
                            elide: Text.ElideMiddle

                            HoverHandler {
                                id: pathHover
                            }
                            QQC2.ToolTip.visible: pathHover.hovered && pathLabel.truncated
                            QQC2.ToolTip.text: pathLabel.text
                        }

                        QQC2.TextField {
                            visible: assignmentDelegate.isCustom
                            Layout.fillWidth: true
                            text: assignmentDelegate.model.gameSettingsPath
                            placeholderText: KI18n.i18nc("@info:placeholder", "Path to Settings.json...")
                            onEditingFinished: assignmentDelegate.model.gameSettingsPath = text
                        }

                        QQC2.ToolButton {
                            icon.name: assignmentDelegate.isCustom ? "document-open" : "edit-copy"
                            enabled: assignmentDelegate.isCustom || assignmentDelegate.model.profileName.length > 0
                            QQC2.ToolTip.visible: hovered
                            QQC2.ToolTip.text: assignmentDelegate.isCustom ? KI18n.i18nc("@info:tooltip", "Select custom Settings.json file") : KI18n.i18nc("@info:tooltip", "Copy the game's current Settings.json to this account")
                            onClicked: {
                                if (assignmentDelegate.isCustom) {
                                    customSettingsDialog.targetRow = assignmentDelegate.index;
                                    customSettingsDialog.currentFolder = Accounts.GameSettings.profileSettingsFolder();
                                    customSettingsDialog.open();
                                } else {
                                    root.requestCopy([assignmentDelegate.index]);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    FormCard.FormHeader {
        title: KI18n.i18nc("@title:group", "Available Settings")
    }

    FormCard.FormCard {
        Repeater {
            model: Accounts.GameSettings.files

            delegate: FormCard.AbstractFormDelegate {
                id: fileDelegate
                required property string modelData

                readonly property var accountNames: root.accountsUsing(fileDelegate.modelData)

                contentItem: RowLayout {
                    spacing: Kirigami.Units.largeSpacing

                    Kirigami.Icon {
                        source: "application-json"
                        Layout.preferredWidth: Kirigami.Units.iconSizes.small
                        Layout.preferredHeight: Kirigami.Units.iconSizes.small
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 0

                        QQC2.Label {
                            text: fileDelegate.modelData
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                        QQC2.Label {
                            text: fileDelegate.accountNames.length > 0 ? KI18n.i18nc("@info", "Used by %1", fileDelegate.accountNames.join(", ")) : KI18n.i18nc("@info", "Not used by any account")
                            font: Kirigami.Theme.smallFont
                            opacity: 0.7
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                    }

                    QQC2.ToolButton {
                        icon.name: "edit-delete"
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.text: KI18n.i18nc("@info:tooltip", "Remove these settings")
                        onClicked: {
                            removePrompt.fileName = fileDelegate.modelData;
                            removePrompt.accountNames = fileDelegate.accountNames;
                            removePrompt.open();
                        }
                    }
                }
            }
        }

        FormCard.FormTextDelegate {
            visible: Accounts.GameSettings.files.length === 0
            text: KI18n.i18nc("@info", "No game settings are stored yet.")
            description: KI18n.i18nc("@info", "Copy the game's current settings to an account to store them here.")
        }
    }

    FormCard.FormHeader {
        title: KI18n.i18nc("@title:group", "Actions")
    }

    FormCard.FormCard {
        FormCard.FormButtonDelegate {
            text: KI18n.i18nc("@action:button", "Copy Current Settings to All Accounts")
            description: KI18n.i18nc("@info:label", "Copy the Settings.json the game currently uses for each account to its account specific settings.")
            icon.name: "edit-copy"
            onClicked: {
                const rows = [];
                for (let row = 0; row < Accounts.ProfileManager.rowCount(); ++row) {
                    if (Accounts.ProfileManager.getProfile(row).profileName.length > 0) {
                        rows.push(row);
                    }
                }
                root.requestCopy(rows);
            }
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormButtonDelegate {
            text: KI18n.i18nc("@action:button", "Open Settings Folder")
            description: KI18n.i18nc("@info:label", "Settings put there by hand show up the next time this page is opened.")
            icon.name: "folder-open"
            onClicked: Qt.openUrlExternally(Accounts.GameSettings.profileSettingsFolder())
        }

        Kirigami.InlineMessage {
            id: message
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.smallSpacing
            visible: message.text.length > 0
        }
    }
}
