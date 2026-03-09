import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.tableview as Tables
import BookTableModel
import Qt.labs.qmlmodels

ApplicationWindow {
    id: mainWindow
    visible: true
    width: 800
    height: 600
    title: `${Application.name}`

    menuBar: MenuBar {
        Menu {
            title: "&Settings"
            MenuItem {
                text: "&Settings"
                icon.name: "settings-configure"
                onTriggered: fileOpenDialog.open()
            }
            MenuItem {
                text: "&Load Settings..."
                icon.name: "document-open"
                onTriggered: fileOpenDialog.open()
            }
            MenuItem {
                text: "&Save Settings..."
                icon.name: "document-save"
                onTriggered: fileOpenDialog.open()
            }
            MenuSeparator {}
            MenuItem {
                text: "&Exit"
                icon.name: "application-exit"
                onTriggered: fileOpenDialog.open()
            }
        }

        Menu {
            title: "&Account"

            MenuItem {
                text: "&Add Account"
                icon.name: "list-add-user"
                onTriggered: fileOpenDialog.open()
            }

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

    AccountTable {
        id: accountTable
    }

    AboutDialog {
        id: aboutDialog
    }

    FileDialog {
        id: fileOpenDialog
    }
}
