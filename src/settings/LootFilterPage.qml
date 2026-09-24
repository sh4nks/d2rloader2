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

    title: KI18n.i18nc("@title", "Loot Filters")

    // Files may have been put into the library folder by hand.
    Component.onCompleted: Accounts.LootFilters.refresh()

    function filterOptions(current) {
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

    function accountsUsing(name) {
        const names = [];
        for (let row = 0; row < Accounts.ProfileManager.rowCount(); ++row) {
            const profile = Accounts.ProfileManager.getProfile(row);
            if (profile.lootFilter === name) {
                names.push(profile.profileName);
            }
        }
        return names;
    }

    function showMessage(type, text) {
        message.type = type;
        message.text = text;
    }

    function showResult(error, success) {
        if (error.length > 0) {
            root.showMessage(Kirigami.MessageType.Error, error);
        } else {
            root.showMessage(Kirigami.MessageType.Positive, success);
        }
    }

    /**
     * Calls importer(overwrite), asking first when the library already has a
     * loot filter named \a name.
     */
    function requestImport(name, importer) {
        if (!Accounts.LootFilters.names.includes(name)) {
            importer(false);
            return;
        }
        overwritePrompt.filterName = name;
        overwritePrompt.importer = importer;
        overwritePrompt.open();
    }

    function openGameFilterMenu(row) {
        const profile = Accounts.ProfileManager.getProfile(row);
        const filters = Accounts.LootFilters.gameFilters(profile);
        if (filters.length === 0) {
            root.showMessage(Kirigami.MessageType.Information, KI18n.i18nc("@info", "The game has no loot filters for %1 yet.", profile.profileName));
            return;
        }
        gameFilterMenu.row = row;
        gameFilterMenu.filters = filters;
        gameFilterMenu.popup();
    }

    function importFromGame(row, name) {
        const profile = Accounts.ProfileManager.getProfile(row);
        root.requestImport(name, overwrite => {
            const error = Accounts.LootFilters.importFromGame(profile, name, overwrite);
            if (error.length === 0) {
                Accounts.ProfileManager.setData(Accounts.ProfileManager.index(row, 0), name, Accounts.ProfileManager.LootFilterRole);
            }
            root.showResult(error, KI18n.i18nc("@info", "Added \"%1\" to the library and chose it for %2.", name, profile.profileName));
        });
    }

    function removeFilter(name) {
        const error = Accounts.LootFilters.remove(name);
        if (error.length > 0) {
            root.showMessage(Kirigami.MessageType.Error, error);
            return;
        }
        for (let row = 0; row < Accounts.ProfileManager.rowCount(); ++row) {
            if (Accounts.ProfileManager.getProfile(row).lootFilter === name) {
                Accounts.ProfileManager.setData(Accounts.ProfileManager.index(row, 0), "", Accounts.ProfileManager.LootFilterRole);
            }
        }
        root.showMessage(Kirigami.MessageType.Positive, KI18n.i18nc("@info", "Removed \"%1\" from the library.", name));
    }

    FileDialog {
        id: fileDialog

        title: KI18n.i18nc("@title:window", "Select Loot Filter")
        nameFilters: ["Loot filters (*.fltr)", "All files (*)"]
        onAccepted: {
            const url = fileDialog.selectedFile;
            const name = decodeURIComponent(url.toString()).split("/").pop().replace(/\.[^.]*$/, "");
            root.requestImport(name, overwrite => {
                root.showResult(Accounts.LootFilters.importFile(url, overwrite), KI18n.i18nc("@info", "Added \"%1\" to the library.", name));
            });
        }
    }

    QQC2.Menu {
        id: gameFilterMenu

        property int row: -1
        property var filters: []

        Instantiator {
            model: gameFilterMenu.filters
            delegate: QQC2.MenuItem {
                id: filterItem
                required property string modelData
                text: filterItem.modelData
                onTriggered: root.importFromGame(gameFilterMenu.row, filterItem.modelData)
            }
            onObjectAdded: (index, object) => gameFilterMenu.insertItem(index, object as QQC2.MenuItem)
            onObjectRemoved: (index, object) => gameFilterMenu.removeItem(object as QQC2.MenuItem)
        }
    }

    Kirigami.PromptDialog {
        id: overwritePrompt

        property string filterName
        property var importer: null

        title: KI18n.i18nc("@title:window", "Overwrite Loot Filter")
        subtitle: KI18n.i18nc("@info", "The library already has a loot filter named \"%1\". Replace it?", overwritePrompt.filterName)
        standardButtons: Kirigami.Dialog.Cancel
        showCloseButton: false

        customFooterActions: [
            Kirigami.Action {
                text: KI18n.i18nc("@action:button", "Overwrite")
                icon.name: "document-replace"
                onTriggered: {
                    overwritePrompt.importer(true);
                    overwritePrompt.close();
                }
            }
        ]
    }

    Kirigami.PromptDialog {
        id: removePrompt

        property string filterName
        property var accountNames: []

        title: KI18n.i18nc("@title:window", "Remove Loot Filter")
        subtitle: removePrompt.accountNames.length > 0 ? KI18n.i18ncp("@info", "Remove \"%2\" from the library? The account %3 goes back to the game's own loot filters.", "Remove \"%2\" from the library? The accounts %3 go back to the game's own loot filters.", removePrompt.accountNames.length, removePrompt.filterName, removePrompt.accountNames.join(", ")) : KI18n.i18nc("@info", "Remove \"%1\" from the library?", removePrompt.filterName)
        standardButtons: Kirigami.Dialog.Cancel
        showCloseButton: false

        customFooterActions: [
            Kirigami.Action {
                text: KI18n.i18nc("@action:button", "Remove")
                icon.name: "edit-delete"
                onTriggered: {
                    root.removeFilter(removePrompt.filterName);
                    removePrompt.close();
                }
            }
        ]
    }

    FormCard.FormHeader {
        title: KI18n.i18nc("@title:group", "Account Loot Filters")
    }

    FormCard.FormCard {
        Layout.fillWidth: true

        Repeater {
            model: Accounts.ProfileManager

            delegate: FormCard.AbstractFormDelegate {
                id: accountDelegate
                required property var model
                required property int index

                readonly property var characters: {
                    // The characters also follow the name and auth method,
                    // which the call cannot report as dependencies.
                    accountDelegate.model.profileName;
                    accountDelegate.model.authMethod;
                    return Accounts.LootFilters.characterNames(accountDelegate.model.profile);
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
                        text: accountDelegate.model.profileName
                        font.bold: true
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }

                    QQC2.ComboBox {
                        id: filterSelector
                        model: root.filterOptions(accountDelegate.model.lootFilter)
                        textRole: "name"
                        valueRole: "value"
                        Layout.preferredWidth: Kirigami.Units.gridUnit * 10
                        // indexOfValue() does not register the model as a binding dependency,
                        // so reading count first is what makes this re-evaluate once the model
                        // is populated.
                        currentIndex: filterSelector.count > 0 ? filterSelector.indexOfValue(accountDelegate.model.lootFilter) : -1

                        onActivated: accountDelegate.model.lootFilter = filterSelector.currentValue
                    }

                    RowLayout {
                        Layout.row: 1
                        Layout.column: 1
                        Layout.columnSpan: 2
                        Layout.fillWidth: true
                        spacing: Kirigami.Units.smallSpacing

                        QQC2.Label {
                            id: charactersLabel
                            Layout.fillWidth: true
                            text: accountDelegate.characters.length > 0 ? KI18n.i18nc("@info", "Characters: %1", accountDelegate.characters.join(", ")) : KI18n.i18nc("@info", "No characters found yet.")
                            font: Kirigami.Theme.smallFont
                            opacity: 0.7
                            elide: Text.ElideRight

                            HoverHandler {
                                id: charactersHover
                            }
                            QQC2.ToolTip.visible: charactersHover.hovered && charactersLabel.truncated
                            QQC2.ToolTip.text: charactersLabel.text
                        }

                        QQC2.ToolButton {
                            icon.name: "document-import"
                            enabled: accountDelegate.model.profileName.length > 0
                            QQC2.ToolTip.visible: hovered
                            QQC2.ToolTip.text: KI18n.i18nc("@info:tooltip", "Import a loot filter from this account's game")
                            onClicked: root.openGameFilterMenu(accountDelegate.index)
                        }
                    }
                }
            }
        }
    }

    FormCard.FormHeader {
        title: KI18n.i18nc("@title:group", "Library")
    }

    FormCard.FormCard {
        Repeater {
            model: Accounts.LootFilters.names

            delegate: FormCard.AbstractFormDelegate {
                id: filterDelegate
                required property string modelData

                readonly property var accountNames: root.accountsUsing(filterDelegate.modelData)

                contentItem: RowLayout {
                    spacing: Kirigami.Units.largeSpacing

                    Kirigami.Icon {
                        source: "view-filter"
                        Layout.preferredWidth: Kirigami.Units.iconSizes.small
                        Layout.preferredHeight: Kirigami.Units.iconSizes.small
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 0

                        QQC2.Label {
                            text: filterDelegate.modelData
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                        QQC2.Label {
                            text: filterDelegate.accountNames.length > 0 ? KI18n.i18nc("@info", "Used by %1", filterDelegate.accountNames.join(", ")) : KI18n.i18nc("@info", "Not used by any account")
                            font: Kirigami.Theme.smallFont
                            opacity: 0.7
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                    }

                    QQC2.ToolButton {
                        icon.name: "edit-delete"
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.text: KI18n.i18nc("@info:tooltip", "Remove from the library")
                        onClicked: {
                            removePrompt.filterName = filterDelegate.modelData;
                            removePrompt.accountNames = filterDelegate.accountNames;
                            removePrompt.open();
                        }
                    }
                }
            }
        }

        FormCard.FormTextDelegate {
            visible: Accounts.LootFilters.names.length === 0
            text: KI18n.i18nc("@info", "The library is empty.")
            description: KI18n.i18nc("@info", "Import a loot filter from an account's game or from a file.")
        }
    }

    FormCard.FormHeader {
        title: KI18n.i18nc("@title:group", "Actions")
    }

    FormCard.FormCard {
        FormCard.FormButtonDelegate {
            text: KI18n.i18nc("@action:button", "Import Loot Filter File...")
            description: KI18n.i18nc("@info:label", "Add a .fltr file, for example one shared by another player, to the library.")
            icon.name: "document-import"
            onClicked: fileDialog.open()
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormButtonDelegate {
            text: KI18n.i18nc("@action:button", "Open Library Folder")
            description: KI18n.i18nc("@info:label", "Loot filters put there by hand show up the next time this page is opened.")
            icon.name: "folder-open"
            onClicked: Qt.openUrlExternally(Accounts.LootFilters.libraryFolder())
        }

        Kirigami.InlineMessage {
            id: message
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.smallSpacing
            visible: message.text.length > 0
        }
    }
}
