pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Window
import org.kde.kirigami as Kirigami

Kirigami.ApplicationWindow {
    id: mainWindow
    visible: true
    width: 800
    height: 800
    title: `${Application.name}`

    menuBar: MenuBar {
        Menu {
            title: "&Settings"
            MenuItem {
                text: "&Settings"
                icon.name: "settings-configure"
                onTriggered: settingsWindow.open()
            }
            MenuItem {
                text: "&Save Settings..."
                icon.name: "document-save"
                onTriggered: fileOpenDialog.open()
            }
            MenuSeparator {}
            MenuItem {
                text: "&Load Account Settings..."
                icon.name: "document-open"
                onTriggered: fileOpenDialog.open()
            }
            MenuItem {
                text: "&Save Account Settings..."
                icon.name: "username-copy"
                onTriggered: fileOpenDialog.open()
            }
            MenuSeparator {}
            MenuItem {
                text: "&Exit"
                icon.name: "application-exit"
                onTriggered: Qt.quit()
            }
        }
        Menu {
            title: "&Help"

            MenuItem {
                text: "&Open README"
                icon.name: "help-contents"
                onTriggered: aboutDialog.show()
            }
            MenuSeparator {}
            MenuItem {
                text: "&About..."
                icon.name: "help-about"
                onTriggered: aboutDialog.show()
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Fixed Upper Section: Accounts Table
        AccountTable {
            id: accountTable
            Layout.fillWidth: true
            Layout.fillHeight: true
            onSettingsClicked: settingsWindow.open()
            onAddAccountClicked: {
                settingsWindow.openAccountEditor();
            }
            onEditAccountClicked: accountData => {
                console.log("accountData", accountData);
                settingsWindow.openAccountEditor(accountData);
            }
        }

        // TabBar beneath the accounts table
        TabBar {
            id: tabBar
            Layout.fillWidth: true
            currentIndex: stackLayout.currentIndex

            TabButton {
                implicitHeight: Kirigami.Units.gridUnit * 2
                text: "Terror Zones"
                icon.name: "view-calendar-day"
            }
            TabButton {
                implicitHeight: Kirigami.Units.gridUnit * 2
                text: "Diablo Clone"
                icon.name: "view-media-artist"
            }
            TabButton {
                implicitHeight: Kirigami.Units.gridUnit * 2
                text: "Application Log"
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

    SettingsWindow {
        id: settingsWindow
    }

    AboutDialog {
        id: aboutDialog
    }

    FileDialog {
        id: fileOpenDialog
    }
}
