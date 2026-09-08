pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Window
import org.kde.kirigami as Kirigami

import com.someblocks.d2rloader.accounts as Accounts
import com.someblocks.d2rloader.settings as Settings

Kirigami.ApplicationWindow {
    id: mainWindow
    visible: true
    width: 800
    height: 800
    // title: `${Application.name}`

    menuBar: MenuBar {
        Menu {
            title: i18nc("@title:menu", "&Settings")
            MenuItem {
                text: i18nc("@action:inmenu", "&Settings")
                icon.name: "settings-configure"
                onTriggered: Settings.SettingsWindow.open()
            }
            MenuItem {
                text: i18nc("@action:inmenu", "&Save Settings...")
                icon.name: "document-save"
                onTriggered: fileOpenDialog.open()
            }
            MenuSeparator {}
            MenuItem {
                text: i18nc("@action:inmenu", "&Load Account Settings...")
                icon.name: "document-open"
                onTriggered: fileOpenDialog.open()
            }
            MenuItem {
                text: i18nc("@action:inmenu", "&Save Account Settings...")
                icon.name: "username-copy"
                onTriggered: fileOpenDialog.open()
            }
            MenuSeparator {}
            MenuItem {
                text: i18nc("@action:inmenu", "&Exit")
                icon.name: "application-exit"
                onTriggered: Qt.quit()
            }
        }
        Menu {
            title: i18nc("@title:menu", "&Help")

            MenuItem {
                text: i18nc("@action:inmenu", "&Open README")
                icon.name: "help-contents"
                onTriggered: aboutDialog.show()
            }
            MenuSeparator {}
            MenuItem {
                text: i18nc("@action:inmenu", "&About...")
                icon.name: "help-about"
                onTriggered: aboutDialog.show()
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Fixed Upper Section: Accounts Table
        Accounts.AccountTable {
            id: accountTable
            modelData: Accounts.ProfileManager
            Layout.topMargin: Kirigami.Units.smallSpacing * 6
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
        }

        // TabBar beneath the accounts table
        TabBar {
            id: tabBar
            Layout.fillWidth: true
            currentIndex: stackLayout.currentIndex

            TabButton {
                implicitHeight: Kirigami.Units.gridUnit * 2
                text: i18nc("@title:tab", "Terror Zones")
                icon.name: "view-calendar-day"
            }
            TabButton {
                implicitHeight: Kirigami.Units.gridUnit * 2
                text: i18nc("@title:tab", "Diablo Clone")
                icon.name: "view-media-artist"
            }
            TabButton {
                implicitHeight: Kirigami.Units.gridUnit * 2
                text: i18nc("@title:tab", "Application Log")
                icon.name: "utilities-log-viewer"
            }
        }

        // Content Area for Tabs
        StackLayout {
            id: stackLayout
            Layout.fillWidth: true
            Layout.preferredHeight: Kirigami.Units.gridUnit * 5
            currentIndex: tabBar.currentIndex

            TerrorZones {
                id: terrorZonesTab
            }

            DiabloClone {
                id: diabloCloneTab
            }

            ApplicationLog {
                id: applicationLogTab
            }
        }
    }

    AboutDialog {
        id: aboutDialog
    }

    FileDialog {
        id: fileOpenDialog
    }
}
