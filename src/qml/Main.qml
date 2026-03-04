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

    // AccountTable {
    //     id: accountTable
    // }

    Tables.KTableView {
        id: bookTable
        model: bookTableModel
        anchors.fill: parent
        anchors.leftMargin: 0
        anchors.rightMargin: 0

        interactive: false
        clip: true
        alternatingRows: true

        sortOrder: Qt.AscendingOrder
        sortRole: BookRoles.YearRole

        onWidthChanged: Qt.callLater(table.forceLayout)

        onColumnClicked: function (index, headerComponent) {
            console.log("bookTable: ", bookTable)
            console.log("bookTable.table: ", )
            console.log(index, headerComponent)
            if (bookTable.sortRole !== headerComponent.role) {
                bookTable.sortRole = index;
                bookTable.sortOrder = Qt.AscendingOrder;
            } else {
                bookTable.sortOrder = bookTable.sortOrder === Qt.AscendingOrder ? Qt.DescendingOrder : Qt.AscendingOrder;
            }

            bookTable.model.sort(bookTable.sortRole, bookTable.sortOrder);

            // After sorting we need update selection
            __resetSelection();
        }

        function getColumnWidth() {
            return bookTable.width / 4
        }

        function __resetSelection() {
            // NOTE: Making a forced copy of the list
            let selectedIndexes = Array(...bookTable.selectionModel.selectedIndexes);

            let currentRow = bookTable.selectionModel.currentIndex.row;
            let currentColumn = bookTable.selectionModel.currentIndex.column;

            bookTable.selectionModel.clear();
            for (let i in selectedIndexes) {
                bookTable.selectionModel.select(selectedIndexes[i], ItemSelectionModel.Select);
            }

            bookTable.selectionModel.setCurrentIndex(bookTable.model.index(currentRow, currentColumn), ItemSelectionModel.Select);
        }

        headerComponents: [
            Tables.HeaderComponent {
                width: bookTable.width / 4
                title: "Book"
                textRole: "title"
                resizable: true
                role: BookRoles.TitleRole
            },
            Tables.HeaderComponent {
                width: bookTable.width / 4
                title: "Author"
                textRole: "author"
                resizable: true
                role: BookRoles.AuthorRole

                leading: Kirigami.Icon {
                    source: "social"
                    implicitWidth: bookTable.compact ? Kirigami.Units.iconSizes.small : Kirigami.Units.iconSizes.medium
                    implicitHeight: implicitWidth
                }
            },
            Tables.HeaderComponent {
                width: bookTable.width / 4
                title: "Year"
                textRole: "year"
                resizable: true
                role: BookRoles.YearRole
            },
            Tables.HeaderComponent {
                width: bookTable.width / 4
                title: "Rating"
                textRole: "rating"
                resizable: true
                role: BookRoles.RatingRole

                leading: Kirigami.Icon {
                    source: "star-shape"
                    implicitWidth: bookTable.compact ? Kirigami.Units.iconSizes.small : Kirigami.Units.iconSizes.medium
                    implicitHeight: implicitWidth
                }
            }
        ]
    }

    AboutDialog {
        id: aboutDialog
    }

    FileDialog {
        id: fileOpenDialog
    }
}
