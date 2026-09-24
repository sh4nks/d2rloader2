pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Dialogs
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.ki18n
import com.someblocks.d2rloader.accounts as Accounts
import com.someblocks.d2rloader.core

FormCard.FormCardPage {
    id: root

    title: root.isNew ? KI18n.i18nc("@title", "Add Account") : KI18n.i18nc("@title", "Edit Account")

    property Accounts.Profile profile

    property bool isNew: true

    property bool showAdvanced: false

    // Set once rather than bound, so clearing the field does not collapse it.
    Component.onCompleted: {
        root.showAdvanced = root.profile.environmentVariables.length > 0;
        Accounts.LootFilters.refresh();
    }

    // Closing the settings window mid-edit destroys this page without going
    // through Save or Cancel. Anything still outstanding is thrown away;
    // committed and cancelled drafts are no longer outstanding, so this is a
    // no-op for them.
    Component.onDestruction: Accounts.ProfileManager.discard(root.profile)

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

    function copyCurrentSettings() {
        const error = Accounts.GameSettings.copyCurrent(root.profile, true);
        if (error.length > 0) {
            copyMessage.type = Kirigami.MessageType.Error;
            copyMessage.text = error;
            return;
        }
        root.profile.gameSettings = Accounts.GameSettingsType.Profile;
        copyMessage.type = Kirigami.MessageType.Positive;
        copyMessage.text = KI18n.i18nc("@info", "Copied the game's current settings to this account.");
    }

    Kirigami.PromptDialog {
        id: overwritePrompt

        title: KI18n.i18nc("@title:window", "Overwrite Settings")
        subtitle: KI18n.i18nc("@info", "This account already has its own settings. Replace them with the game's current settings?")
        standardButtons: Kirigami.Dialog.Cancel
        showCloseButton: false

        customFooterActions: [
            Kirigami.Action {
                text: KI18n.i18nc("@action:button", "Overwrite")
                icon.name: "document-replace"
                onTriggered: {
                    root.copyCurrentSettings();
                    overwritePrompt.close();
                }
            }
        ]
    }

    function lootFilterOptions(current) {
        const options = [
            {
                name: KI18n.i18nc("@item loot filter", "Game Default"),
                value: ""
            }
        ].concat(Accounts.LootFilters.names.map(name => ({
                    name: name,
                    value: name
                })));
        if (current.length > 0 && !Accounts.LootFilters.names.includes(current)) {
            options.push({
                name: KI18n.i18nc("@item loot filter that is no longer in the library", "%1 (missing)", current),
                value: current
            });
        }
        return options;
    }

    function importLootFilter(name, overwrite) {
        const error = Accounts.LootFilters.importFromGame(root.profile, name, overwrite);
        if (error.length > 0) {
            lootFilterMessage.type = Kirigami.MessageType.Error;
            lootFilterMessage.text = error;
            return;
        }
        root.profile.lootFilter = name;
        lootFilterMessage.type = Kirigami.MessageType.Positive;
        lootFilterMessage.text = KI18n.i18nc("@info", "Added \"%1\" to the library and chose it for this account.", name);
    }

    function requestLootFilterImport(name) {
        if (Accounts.LootFilters.names.includes(name)) {
            lootFilterOverwritePrompt.filterName = name;
            lootFilterOverwritePrompt.open();
        } else {
            root.importLootFilter(name, false);
        }
    }

    Kirigami.PromptDialog {
        id: lootFilterOverwritePrompt

        property string filterName

        title: KI18n.i18nc("@title:window", "Overwrite Loot Filter")
        subtitle: KI18n.i18nc("@info", "The library already has a loot filter named \"%1\". Replace it with the one from the game?", lootFilterOverwritePrompt.filterName)
        standardButtons: Kirigami.Dialog.Cancel
        showCloseButton: false

        customFooterActions: [
            Kirigami.Action {
                text: KI18n.i18nc("@action:button", "Overwrite")
                icon.name: "document-replace"
                onTriggered: {
                    root.importLootFilter(lootFilterOverwritePrompt.filterName, true);
                    lootFilterOverwritePrompt.close();
                }
            }
        ]
    }

    QQC2.Menu {
        id: gameFilterMenu

        property var filters: []

        Instantiator {
            model: gameFilterMenu.filters
            delegate: QQC2.MenuItem {
                id: filterItem
                required property string modelData
                text: filterItem.modelData
                onTriggered: root.requestLootFilterImport(filterItem.modelData)
            }
            onObjectAdded: (index, object) => gameFilterMenu.insertItem(index, object as QQC2.MenuItem)
            onObjectRemoved: (index, object) => gameFilterMenu.removeItem(object as QQC2.MenuItem)
        }
    }

    FileDialog {
        id: customSettingsDialog
        title: KI18n.i18nc("@title:window", "Select Custom Settings.json")
        nameFilters: ["JSON files (*.json)", "All files (*)"]
        onAccepted: root.profile.gameSettingsPath = root.urlToPath(selectedFile)
    }

    FolderDialog {
        id: gamePathDialog
        title: KI18n.i18nc("@title:window", "Select Game Directory")
        onAccepted: root.profile.gamePath = root.urlToPath(selectedFolder)
    }

    FolderDialog {
        id: protonPathDialog
        title: KI18n.i18nc("@title:window", "Select Proton Runtime Directory")
        onAccepted: root.profile.protonPath = root.urlToPath(selectedFolder)
    }

    FormCard.FormHeader {
        title: KI18n.i18nc("@title:group", "Account Configuration")
    }

    FormCard.FormCard {
        FormCard.FormTextFieldDelegate {
            id: nameField
            label: KI18n.i18nc("@label", "Account Name")
            description: KI18n.i18nc("@info:label", "A unique identifier for this account.")
            text: root.profile.profileName
            placeholderText: KI18n.i18nc("@info:placeholder", "e.g. MyAccount")
            onEditingFinished: root.profile.profileName = text
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormComboBoxDelegate {
            id: authField
            text: KI18n.i18nc("@label", "Authentication Method")
            description: KI18n.i18nc("@info:label", "The method used to log in to Battle.net.")
            model: Accounts.AuthMethodModel

            textRole: "name"
            valueRole: "value"

            Component.onCompleted: {
                authField.currentIndex = authField.indexOfValue(root.profile.authMethod);
            }

            onActivated: root.profile.authMethod = currentValue
        }

        FormCard.FormDelegateSeparator {
            visible: tokenField.visible
        }

        FormCard.FormTextFieldDelegate {
            id: tokenField
            label: KI18n.i18nc("@label", "Authentication Token")
            description: KI18n.i18nc("@info:label", "The Battle.net login token.")
            visible: root.profile.authMethod === Accounts.AuthMethodModel.Token
            text: root.profile.token
            placeholderText: KI18n.i18nc("@info:placeholder", "Enter your login token...")
            onEditingFinished: root.profile.token = text
        }

        // --- Password Conditional Fields ---
        FormCard.FormDelegateSeparator {
            visible: emailField.visible
        }

        FormCard.FormTextFieldDelegate {
            id: emailField
            label: KI18n.i18nc("@label", "Battle.net Email Address")
            description: KI18n.i18nc("@info:label", "Your Battle.net account email.")
            visible: root.profile.authMethod === Accounts.AuthMethodModel.Password
            text: root.profile.email
            placeholderText: KI18n.i18nc("@info:placeholder", "example@email.com")
            onEditingFinished: root.profile.email = text
        }

        FormCard.FormDelegateSeparator {
            visible: passwordField.visible
        }

        FormCard.FormPasswordFieldDelegate {
            id: passwordField
            label: KI18n.i18nc("@label", "Battle.net Password")
            visible: root.profile.authMethod === Accounts.AuthMethodModel.Password
            text: root.profile.password
            onEditingFinished: root.profile.password = text
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormComboBoxDelegate {
            id: regionField
            text: KI18n.i18nc("@label", "Region")
            description: KI18n.i18nc("@info:label", "The game server region for this account.")
            model: Accounts.RegionModel

            textRole: "name"
            valueRole: "value"

            Component.onCompleted: {
                regionField.currentIndex = regionField.indexOfValue(root.profile.region);
            }

            onActivated: root.profile.region = currentValue
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormTextFieldDelegate {
            id: paramsField
            label: KI18n.i18nc("@label", "Launch Parameters")
            description: KI18n.i18nc("@info:label", "Command line arguments passed to the game executable.")
            text: root.profile.gameParameters
            placeholderText: KI18n.i18nc("@info:placeholder", "-w -txt")
            onEditingFinished: root.profile.gameParameters = text
        }
    }

    FormCard.FormHeader {
        title: KI18n.i18nc("@title:group", "Compatibility")
    }

    FormCard.FormCard {
        FormCard.AbstractFormDelegate {
            id: gamePathDelegate
            // Steam starts the game from its own installation.
            enabled: root.profile.authMethod !== Accounts.AuthMethodModel.Steam
            contentItem: ColumnLayout {
                spacing: Kirigami.Units.smallSpacing
                QQC2.Label {
                    text: KI18n.i18nc("@label", "Game Directory")
                }
                QQC2.Label {
                    text: gamePathDelegate.enabled ? KI18n.i18nc("@info:label", "A different Diablo II: Resurrected installation for this account. Leave empty to use the one from the application settings.") : KI18n.i18nc("@info:label", "Steam accounts start the game from Steam's own installation.")
                    font: Kirigami.Theme.smallFont
                    opacity: 0.7
                    wrapMode: Text.Wrap
                    Layout.fillWidth: true
                }
                RowLayout {
                    QQC2.TextField {
                        id: gamePathField
                        Layout.fillWidth: true
                        text: root.profile.gamePath
                        placeholderText: D2RLoaderConfig.gamePath || KI18n.i18nc("@info:placeholder", "Path to Diablo II Resurrected...")
                        onEditingFinished: root.profile.gamePath = text
                    }
                    QQC2.Button {
                        icon.name: "folder-open"
                        onClicked: gamePathDialog.open()
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.text: KI18n.i18nc("@info:tooltip", "Browse...")
                    }
                }
            }
        }

        FormCard.FormDelegateSeparator {}

        FormCard.AbstractFormDelegate {
            contentItem: ColumnLayout {
                spacing: Kirigami.Units.smallSpacing
                QQC2.Label {
                    text: KI18n.i18nc("@label", "Proton Runtime")
                }
                QQC2.Label {
                    text: KI18n.i18nc("@info:label", "The directory containing the Proton/Wine compatibility layer for this account.")
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
                        placeholderText: KI18n.i18nc("@info:placeholder", "e.g. GE-Proton or UMU-Latest")
                        onEditingFinished: root.profile.protonPath = text
                    }
                    QQC2.Button {
                        icon.name: "folder-open"
                        onClicked: protonPathDialog.open()
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.text: KI18n.i18nc("@info:tooltip", "Browse...")
                    }
                }
            }
        }
    }

    FormCard.FormHeader {
        title: KI18n.i18nc("@title:group", "Game Settings")
    }

    FormCard.FormCard {
        FormCard.FormComboBoxDelegate {
            id: gameSettingsField
            text: KI18n.i18nc("@label", "Settings Profile")
            description: KI18n.i18nc("@info:label", "Select which game settings (Settings.json) to use for this account.")
            model: [
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
            textRole: "name"
            valueRole: "value"
            // indexOfValue() does not register the model as a binding dependency,
            // so reading count first is what makes this re-evaluate once the model
            // is populated.
            currentIndex: gameSettingsField.count > 0 ? gameSettingsField.indexOfValue(root.profile.gameSettings) : -1

            onActivated: root.profile.gameSettings = currentValue
        }

        FormCard.FormDelegateSeparator {}

        FormCard.AbstractFormDelegate {
            id: settingsFileDelegate

            readonly property bool isCustom: root.profile.gameSettings === Accounts.GameSettingsType.Custom
            readonly property string settingsPath: {
                // The path also follows the name, email and auth method, which
                // the call cannot report as dependencies.
                root.profile.profileName;
                root.profile.email;
                root.profile.authMethod;
                return Accounts.GameSettings.settingsPath(root.profile, root.profile.gameSettings);
            }

            contentItem: ColumnLayout {
                spacing: Kirigami.Units.smallSpacing
                QQC2.Label {
                    text: KI18n.i18nc("@label", "Settings File")
                }
                QQC2.Label {
                    text: {
                        switch (root.profile.gameSettings) {
                        case Accounts.GameSettingsType.Profile:
                            return KI18n.i18nc("@info:label", "This account's own copy, put in place of the game's before each launch.");
                        case Accounts.GameSettingsType.Custom:
                            return KI18n.i18nc("@info:label", "A Settings.json of your choice, put in place of the game's before each launch.");
                        default:
                            return KI18n.i18nc("@info:label", "The game's own Settings.json for this account, left as it is.");
                        }
                    }
                    font: Kirigami.Theme.smallFont
                    opacity: 0.7
                    wrapMode: Text.Wrap
                    Layout.fillWidth: true
                }
                Kirigami.SelectableLabel {
                    visible: !settingsFileDelegate.isCustom
                    Layout.fillWidth: true
                    text: settingsFileDelegate.settingsPath || KI18n.i18nc("@info", "Could not find the Steam installation the game runs in.")
                    wrapMode: Text.WrapAnywhere
                }
                RowLayout {
                    visible: settingsFileDelegate.isCustom
                    QQC2.TextField {
                        id: customSettingsPathField
                        Layout.fillWidth: true
                        text: root.profile.gameSettingsPath
                        placeholderText: KI18n.i18nc("@info:placeholder", "Path to Settings.json...")
                        onEditingFinished: root.profile.gameSettingsPath = text
                    }
                    QQC2.Button {
                        icon.name: "document-open"
                        onClicked: {
                            customSettingsDialog.currentFolder = Accounts.GameSettings.profileSettingsFolder();
                            customSettingsDialog.open();
                        }
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.text: KI18n.i18nc("@info:tooltip", "Browse...")
                    }
                }
            }
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormButtonDelegate {
            text: KI18n.i18nc("@action:button", "Copy Current Settings")
            description: KI18n.i18nc("@info:label", "Copy the Settings.json the game currently uses for this account to its account specific settings.")
            icon.name: "edit-copy"
            enabled: nameField.text.length > 0
            onClicked: {
                // The name decides where the copy goes, and the field may not
                // have committed it yet.
                root.profile.profileName = nameField.text;
                if (Accounts.GameSettings.profileSettingsExist(root.profile)) {
                    overwritePrompt.open();
                } else {
                    root.copyCurrentSettings();
                }
            }
        }

        Kirigami.InlineMessage {
            id: copyMessage
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.smallSpacing
            visible: copyMessage.text.length > 0
        }
    }

    FormCard.FormHeader {
        title: KI18n.i18nc("@title:group", "Game Window")
    }

    FormCard.FormCard {
        FormCard.FormSwitchDelegate {
            // Steam's game window keeps the game's own title, so it cannot be told apart.
            enabled: root.profile.authMethod !== Accounts.AuthMethodModel.Steam
            text: KI18n.i18nc("@option:check", "Remember Window Position")
            description: enabled ? KI18n.i18nc("@info:label", "Move the game window back to where it was when this account's game last closed. Only in windowed mode.") : KI18n.i18nc("@info:label", "Steam accounts keep the game's own window title, so their window cannot be told apart.")
            checked: root.profile.rememberWindowPosition
            onToggled: root.profile.rememberWindowPosition = checked
        }
    }

    FormCard.FormHeader {
        title: KI18n.i18nc("@title:group", "Loot Filter")
    }

    FormCard.FormCard {
        FormCard.FormComboBoxDelegate {
            id: lootFilterField
            text: KI18n.i18nc("@label", "Loot Filter")
            description: KI18n.i18nc("@info:label", "Given to every character of this account before each launch. Changes made to it in game are replaced on the next launch unless imported.")
            model: root.lootFilterOptions(root.profile.lootFilter)
            textRole: "name"
            valueRole: "value"
            // indexOfValue() does not register the model as a binding dependency,
            // so reading count first is what makes this re-evaluate once the model
            // is populated.
            currentIndex: lootFilterField.count > 0 ? lootFilterField.indexOfValue(root.profile.lootFilter) : -1

            onActivated: root.profile.lootFilter = currentValue
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormButtonDelegate {
            text: KI18n.i18nc("@action:button", "Import From Game...")
            description: KI18n.i18nc("@info:label", "Add a loot filter made in game for this account to the library.")
            icon.name: "document-import"
            enabled: nameField.text.length > 0
            onClicked: {
                // The name decides which wineprefix is looked in, and the field
                // may not have committed it yet.
                root.profile.profileName = nameField.text;
                gameFilterMenu.filters = Accounts.LootFilters.gameFilters(root.profile);
                if (gameFilterMenu.filters.length === 0) {
                    lootFilterMessage.type = Kirigami.MessageType.Information;
                    lootFilterMessage.text = KI18n.i18nc("@info", "The game has no loot filters for this account yet.");
                    return;
                }
                gameFilterMenu.popup();
            }
        }

        Kirigami.InlineMessage {
            id: lootFilterMessage
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.smallSpacing
            visible: lootFilterMessage.text.length > 0
        }
    }

    FormCard.FormCard {
        Layout.topMargin: Kirigami.Units.gridUnit

        FormCard.FormButtonDelegate {
            text: KI18n.i18nc("@action:button", "Advanced")
            description: KI18n.i18nc("@info:label", "Extra environment variables for the game.")
            icon.name: "configure"
            trailingLogo.direction: root.showAdvanced ? Qt.UpArrow : Qt.DownArrow
            onClicked: root.showAdvanced = !root.showAdvanced
        }

        FormCard.FormDelegateSeparator {
            visible: root.showAdvanced
        }

        FormCard.FormTextAreaDelegate {
            id: environmentField
            visible: root.showAdvanced
            // Steam starts the game with its own environment.
            enabled: root.profile.authMethod !== Accounts.AuthMethodModel.Steam
            label: KI18n.i18nc("@label", "Environment Variables")
            description: enabled ? KI18n.i18nc("@info:label", "One NAME=value per line, set for this account's game on top of the defaults. DRI_PRIME=1 runs it on the other GPU.") : KI18n.i18nc("@info:label", "Steam accounts take their environment from the launch options in Steam.")
            placeholderText: "DRI_PRIME=1\nDXVK_HUD=fps"
            text: root.profile.environmentVariables
            onEditingFinished: root.profile.environmentVariables = text
            status: Kirigami.MessageType.Warning
            statusMessage: {
                const ignored = text.split("\n").map(line => line.trim()).filter(line => line.length > 0 && line.indexOf("=") <= 0);
                return ignored.length > 0 ? KI18n.i18nc("@info", "Ignored, not NAME=value: %1", ignored.join(", ")) : "";
            }
        }
    }

    FormCard.FormHeader {
        title: KI18n.i18nc("@title:group", "Actions")
    }

    FormCard.FormCard {
        FormCard.FormButtonDelegate {
            id: saveButton
            text: root.isNew ? KI18n.i18nc("@action:button", "Create Account") : KI18n.i18nc("@action:button", "Save Changes")
            icon.name: root.isNew ? "list-add" : "document-save"
            highlighted: true
            onClicked: {
                Accounts.ProfileManager.commit(root.profile);
                (root.QQC2.ApplicationWindow.window as Kirigami.ApplicationWindow).pageStack.layers.pop();
            }
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormButtonDelegate {
            id: cancelButton
            text: KI18n.i18nc("@action:button", "Cancel")
            icon.name: "dialog-cancel"
            onClicked: {
                Accounts.ProfileManager.discard(root.profile);
                (root.QQC2.ApplicationWindow.window as Kirigami.ApplicationWindow).pageStack.layers.pop();
            }
        }
    }
}
