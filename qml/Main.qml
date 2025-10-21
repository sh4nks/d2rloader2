import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import org.kde.kirigami as Kirigami
import BookTableModel
import Qt.labs.qmlmodels

import "ktableview" as D2R

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

    D2R.MyTableView {
        id: bookTable
        model: __exampleModel

        interactive: false
        clip: true
        alternatingRows: false

        sortOrder: Qt.AscendingOrder
        sortRole: BookRoles.YearRole

        onColumnClicked: function (index, headerComponent) {
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
            HeaderComponent {
                width: 200
                title: "Book"
                textRole: "title"
                role: BookRoles.TitleRole
            },
            HeaderComponent {
                width: 200
                title: "Author"
                textRole: "author"
                role: BookRoles.AuthorRole

                leading: Kirigami.Icon {
                    source: "social"
                    implicitWidth: bookTable.compact ? Kirigami.Units.iconSizes.small : Kirigami.Units.iconSizes.medium
                    implicitHeight: implicitWidth
                }
            },
            HeaderComponent {
                width: 100
                title: "Year"
                textRole: "year"
                role: BookRoles.YearRole
            },
            HeaderComponent {
                width: 100
                title: "Rating"
                textRole: "rating"
                role: BookRoles.RatingRole

                leading: Kirigami.Icon {
                    source: "star-shape"
                    implicitWidth: bookTable.compact ? Kirigami.Units.iconSizes.small : Kirigami.Units.iconSizes.medium
                    implicitHeight: implicitWidth
                }
            }
        ]
    }

    /*
    TableView {
        id: tableView
        anchors.fill: parent
        columnSpacing: 1
        rowSpacing: 1
        boundsBehavior: Flickable.StopAtBounds

        columnWidthProvider: function (column) {
            return tableView.model ? tableView.width / 5 : 0;
        }

        model: TableModel {
            TableModelColumn {
                display: "checked"
            }
            TableModelColumn {
                display: "amount"
            }
            TableModelColumn {
                display: "fruitType"
            }
            TableModelColumn {
                display: "fruitName"
            }
            TableModelColumn {
                display: "fruitPrice"
            }

            // Each row is one type of fruit that can be ordered
            rows: [
                {
                    // Each property is one cell/column.
                    checked: false,
                    amount: 1,
                    fruitType: "Apple",
                    fruitName: "Granny Smith",
                    fruitPrice: 1.50
                },
                {
                    checked: true,
                    amount: 4,
                    fruitType: "Orange",
                    fruitName: "Navel",
                    fruitPrice: 2.50
                },
                {
                    checked: false,
                    amount: 1,
                    fruitType: "Banana",
                    fruitName: "Cavendish",
                    fruitPrice: 3.50
                }
            ]
        }
        onWidthChanged: tableView.forceLayout()
        delegate: Rectangle {
            implicitWidth: tableView.columnWidthProvider(column)
            implicitHeight: 50
            border.width: 1

            Text {
                text: display
                anchors.centerIn: parent
            }
        }
        Row {
            id: columnsHeader
            y: tableView.contentY
            z: 2
            Repeater {
                model: tableView.columns > 0 ? tableView.columns : 1
                Rectangle {
                    width: tableView.columnWidthProvider(modelData)
                    height: 60
                    clip: true

                    Label {
                        id: headerText
                        width: parent.width
                        color: SystemTheme.palette.windowText.color
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        text: tableView.model ? tableView.model.headerData(modelData, Qt.Horizontal) : 0
                        elide: Text.ElideRight
                        clip: true
                    }
                }
            }
        }
    }
    */
    AboutDialog {
        id: aboutDialog
    }

    FileDialog {
        id: fileOpenDialog
    }

    TableModel {
        id: __exampleModel
        TableModelColumn {
            display: "title"
        }
        TableModelColumn {
            display: "author"
        }
        TableModelColumn {
            display: "year"
        }
        TableModelColumn {
            display: "rating"
        }

        rows: [
            {
                title: "Harry Potter and the Philosopher's Stone",
                author: "J.K. Rowling",
                year: 1997,
                rating: 4.5
            },
            {
                title: "Harry Potter and the Philosopher's Stone",
                author: "J.K. Rowling",
                year: 1997,
                rating: 4.5
            },
            {
                title: "Harry Potter and the Philosopher's Stone",
                author: "J.K. Rowling",
                year: 1997,
                rating: 4.5
            },
            {
                title: "Harry Potter and the Philosopher's Stone",
                author: "J.K. Rowling",
                year: 1997,
                rating: 4.5
            },
        ]
    }
}
