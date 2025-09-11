import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs

ApplicationWindow {
    id: mainWindow
    visible: true
    width: 800
    height: 600
    title: `${Application.name}`

    menuBar: MenuBar {
        Menu {
            title: "&File"

            MenuItem {
                text: "&Open..."
                icon.name: "document-open"
                onTriggered: fileOpenDialog.open()
            }
        }

        Menu {
            title: "&Help"

            MenuItem {
                text: "&About..."
                onTriggered: aboutDialog.show() && console.log(units)
            }
        }
    }

    Units {
        id: units
    }

    AboutDialog {
        id: aboutDialog
    }

    FileDialog {
        id: fileOpenDialog
    }
}
