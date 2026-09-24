pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Window
import org.kde.kirigami as Kirigami
import org.kde.ki18n

import com.someblocks.d2rloader.accounts as Accounts
import com.someblocks.d2rloader.settings as Settings
import com.someblocks.d2rloader.core

Kirigami.ApplicationWindow {
    id: mainWindow
    visible: true
    width: 800
    height: 800
    // title: `${Application.name}`

    Shortcut {
        sequence: "Ctrl+M"
        onActivated: {
            D2RLoaderConfig.showMenuBar = !D2RLoaderConfig.showMenuBar;
            D2RLoaderConfig.save();
        }
    }

    menuBar: MenuBar {
        visible: D2RLoaderConfig.showMenuBar

        Menu {
            title: KI18n.i18nc("@title:menu", "&Settings")
            MenuItem {
                text: KI18n.i18nc("@action:inmenu", "&Settings")
                icon.name: "settings-configure"
                onTriggered: Settings.SettingsWindow.open()
            }
            MenuSeparator {}
            MenuItem {
                text: KI18n.i18nc("@action:inmenu", "&Load Account Settings...")
                icon.name: "document-open"
                onTriggered: importDialog.open()
            }
            MenuItem {
                text: KI18n.i18nc("@action:inmenu", "&Save Account Settings...")
                icon.name: "username-copy"
                onTriggered: exportDialog.open()
            }
            MenuSeparator {}
            MenuItem {
                text: KI18n.i18nc("@action:inmenu", "&Exit")
                icon.name: "application-exit"
                onTriggered: Qt.quit()
            }
        }
        Menu {
            title: KI18n.i18nc("@title:menu", "&Help")

            MenuItem {
                text: KI18n.i18nc("@action:inmenu", "&Open README")
                icon.name: "help-contents"
                onTriggered: aboutDialog.show()
            }
            MenuSeparator {}
            MenuItem {
                text: KI18n.i18nc("@action:inmenu", "&About...")
                icon.name: "help-about"
                onTriggered: aboutDialog.show()
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        // Kirigami's contentItem fills the whole window and only makes room for header, not menuBar
        anchors.topMargin: mainWindow.menuBar.visible ? mainWindow.menuBar.height : 0
        spacing: 0

        Kirigami.InlineMessage {
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.smallSpacing
            type: Kirigami.MessageType.Error
            text: Accounts.ProfileManager.loadError
            visible: text.length > 0
        }

        Kirigami.InlineMessage {
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.smallSpacing
            type: Kirigami.MessageType.Error
            text: Accounts.LaunchSequenceManager.loadError
            visible: text.length > 0
        }

        // Fixed Upper Section: Accounts Table
        Accounts.AccountTable {
            id: accountTable
            modelData: Accounts.ProfileManager
            Layout.topMargin: 0
            Layout.fillWidth: true
            Layout.fillHeight: true
            onSettingsClicked: Settings.SettingsWindow.open()
            onAddAccountClicked: {
                Accounts.ProfileManager.selectedIndex;
                Settings.SettingsWindow.openAccountEditor();
            }
            onEditAccountClicked: rowIndex => {
                Settings.SettingsWindow.openAccountEditor(rowIndex);
            }
            onStartStopClicked: rowIndex => {
                GameManager.toggle(Accounts.ProfileManager.getProfile(rowIndex));
            }
            sequenceRunning: GameManager.sequenceRunning
            onLaunchSequenceClicked: sequenceIndex => GameManager.startSequence(sequenceIndex)
            onCancelSequenceClicked: GameManager.cancelSequence()
        }

        Connections {
            target: GameManager

            function onLaunchFailed(profileName: string, error: string) {
                mainWindow.showPassiveNotification(KI18n.i18nc("@info %1 is the account name, %2 the reason", "Could not start %1: %2", profileName, error), "long");
            }
        }

        // TabBar beneath the accounts table
        TabBar {
            id: tabBar
            Layout.fillWidth: true

            TabButton {
                implicitHeight: Kirigami.Units.gridUnit * 2
                text: KI18n.i18nc("@title:tab", "Terror Zones")
                icon.name: "view-calendar-day"
            }
            TabButton {
                implicitHeight: Kirigami.Units.gridUnit * 2
                text: KI18n.i18nc("@title:tab", "Diablo Clone")
                icon.name: "view-media-artist"
            }
            TabButton {
                implicitHeight: Kirigami.Units.gridUnit * 2
                text: KI18n.i18nc("@title:tab", "Application Log")
                icon.name: "utilities-log-viewer"
            }
        }

        // Content Area for Tabs. Not a StackLayout: hidden Kirigami cards
        // neither lay out nor report their height, so every tab stays visible
        // to size the area by the tallest one and only the current one shows.
        Item {
            Layout.fillWidth: true
            implicitHeight: Math.max(terrorZonesTab.implicitHeight, diabloCloneTab.implicitHeight, applicationLogTab.implicitHeight)

            TerrorZones {
                id: terrorZonesTab
                anchors.fill: parent
                opacity: tabBar.currentIndex === 0 ? 1 : 0
                enabled: tabBar.currentIndex === 0
            }

            DiabloClone {
                id: diabloCloneTab
                anchors.fill: parent
                opacity: tabBar.currentIndex === 1 ? 1 : 0
                enabled: tabBar.currentIndex === 1
            }

            ApplicationLog {
                id: applicationLogTab
                anchors.fill: parent
                opacity: tabBar.currentIndex === 2 ? 1 : 0
                enabled: tabBar.currentIndex === 2
            }
        }
    }

    AboutDialog {
        id: aboutDialog
    }

    FileDialog {
        id: importDialog
        title: KI18n.i18nc("@title:window", "Load Account Settings")
        nameFilters: [KI18n.i18nc("@item", "JSON files (*.json)"), KI18n.i18nc("@item", "All files (*)")]
        onAccepted: {
            importModePrompt.file = importDialog.selectedFile;
            importModePrompt.open();
        }
    }

    FileDialog {
        id: exportDialog
        title: KI18n.i18nc("@title:window", "Save Account Settings")
        fileMode: FileDialog.SaveFile
        defaultSuffix: "json"
        nameFilters: [KI18n.i18nc("@item", "JSON files (*.json)"), KI18n.i18nc("@item", "All files (*)")]
        onAccepted: {
            const error = Accounts.ProfileManager.exportAccounts(exportDialog.selectedFile);
            if (error.length > 0) {
                mainWindow.showPassiveNotification(KI18n.i18nc("@info %1 is the reason", "Could not save the accounts: %1", error), "long");
            } else {
                mainWindow.showPassiveNotification(KI18n.i18ncp("@info", "Saved %1 account.", "Saved %1 accounts.", Accounts.ProfileManager.rowCount()));
            }
        }
    }

    Kirigami.PromptDialog {
        id: importModePrompt

        property url file

        title: KI18n.i18nc("@title:window", "Load Account Settings")
        subtitle: KI18n.i18nc("@info", "Add the accounts in this file to the existing ones, or replace the existing accounts with them?")
        standardButtons: Kirigami.Dialog.Cancel
        showCloseButton: false

        customFooterActions: [
            Kirigami.Action {
                text: KI18n.i18nc("@action:button", "Add")
                icon.name: "list-add"
                onTriggered: mainWindow.importAccounts(importModePrompt.file, false)
            },
            Kirigami.Action {
                text: KI18n.i18nc("@action:button", "Replace")
                icon.name: "document-replace"
                onTriggered: mainWindow.importAccounts(importModePrompt.file, true)
            }
        ]
    }

    function importAccounts(file: url, replace: bool) {
        importModePrompt.close();
        const previousCount = replace ? 0 : Accounts.ProfileManager.rowCount();
        const error = Accounts.ProfileManager.importAccounts(file, replace);
        if (error.length > 0) {
            mainWindow.showPassiveNotification(KI18n.i18nc("@info %1 is the reason", "Could not load the accounts: %1", error), "long");
            return;
        }
        mainWindow.showPassiveNotification(KI18n.i18ncp("@info", "Loaded %1 account.", "Loaded %1 accounts.", Accounts.ProfileManager.rowCount() - previousCount));
    }
}
