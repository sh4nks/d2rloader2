pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import Qt.labs.qmlmodels
import org.kde.kirigami as Kirigami

ColumnLayout {
    anchors.fill: parent
    spacing: 0

    // Set Kirigami Theme properties for the entire component
    Kirigami.Theme.inherit: true
    Kirigami.Theme.colorSet: Kirigami.Theme.View

    HorizontalHeaderView {
        id: horizontalHeader
        syncView: tableView
        // Column 0 is the status indicator (no text header)
        model: ["", "Account", "Auth Method", "Region", "Launch Parameters", "Actions"]
        Layout.fillWidth: true

        delegate: Rectangle {
            required property var modelData
            required property int column

            implicitWidth: tableView.columnWidthProvider(column)
            implicitHeight: Kirigami.Units.gridUnit * 2
            color: Kirigami.Theme.alternateBackgroundColor

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: 1
                color: Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.2)
            }

            Label {
                anchors.fill: parent
                anchors.leftMargin: (column === 1 || column === 4) ? Kirigami.Units.gridUnit : Kirigami.Units.smallSpacing
                anchors.rightMargin: (column === 1 || column === 4) ? Kirigami.Units.gridUnit : Kirigami.Units.smallSpacing
                text: modelData
                font.bold: true
                color: Kirigami.Theme.textColor
                horizontalAlignment: (column === 1 || column === 4) ? Text.AlignLeft : Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }
        }
    }

    TableView {
        id: tableView
        Layout.fillWidth: true
        Layout.fillHeight: true
        columnSpacing: 0
        rowSpacing: 0
        clip: true
        boundsBehavior: Flickable.StopAtBounds

        columnWidthProvider: function (column) {
            if (column === 0) {
                return Kirigami.Units.gridUnit * 2;
            }
            if (column === 5) {
                return Kirigami.Units.gridUnit * 7;
            }
            // Distribute remaining width among other 4 columns (1, 2, 3, 4)
            return (tableView.width - (Kirigami.Units.gridUnit * 9)) / 4;
        }
        onWidthChanged: tableView.forceLayout()

        model: TableModel {
            TableModelColumn {
                display: "status"
            } // Column 0
            TableModelColumn {
                display: "account"
            }
            TableModelColumn {
                display: "authMethod"
            }
            TableModelColumn {
                display: "region"
            }
            TableModelColumn {
                display: "launchParameters"
            }
            TableModelColumn {
                display: "actions"
            }

            rows: [
                {
                    "status": "Running",
                    "account": "cow",
                    "authMethod": "Token",
                    "region": "Europe",
                    "launchParameters": "-w",
                    "actions": "Stop"
                },
                {
                    "status": "Stopped",
                    "account": "dog",
                    "authMethod": "Token",
                    "region": "Europe",
                    "launchParameters": "-w",
                    "actions": "Start"
                },
                {
                    "status": "Stopped",
                    "account": "sheep",
                    "authMethod": "Password",
                    "region": "Europe",
                    "launchParameters": "-w",
                    "actions": "Start"
                },
                {
                    "status": "Stopped",
                    "account": "goat",
                    "authMethod": "Steam",
                    "region": "Europe",
                    "launchParameters": "-w",
                    "actions": "Start"
                }
            ]
        }

        delegate: Rectangle {
            required property var display
            required property int column
            required property int row

            implicitWidth: tableView.columnWidthProvider(column)
            implicitHeight: Kirigami.Units.gridUnit * 2.5
            color: Kirigami.Theme.backgroundColor

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: 1
                color: Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.1)
            }

            // Column 0: Status Indicator (Circle)
            Rectangle {
                visible: column === 0
                anchors.centerIn: parent
                width: Kirigami.Units.gridUnit * 0.6
                height: width
                radius: width / 2
                color: display === "Running" ? Kirigami.Theme.positiveTextColor : "gray"

                // Kirigami.ToolTip {
                //     text: display
                // }
            }

            // Columns 1 & 4: Standard text display (Account, Launch Parameters)
            Label {
                visible: column === 1 || column === 4
                anchors.fill: parent
                anchors.leftMargin: Kirigami.Units.gridUnit
                anchors.rightMargin: Kirigami.Units.gridUnit
                text: display
                color: Kirigami.Theme.textColor
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }

            // Column 2: ComboBox for Auth Method
            ComboBox {
                visible: column === 2
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.leftMargin: Kirigami.Units.smallSpacing
                anchors.rightMargin: Kirigami.Units.smallSpacing
                model: ["Token", "Password", "Steam"]
                currentIndex: model.indexOf(display)
            }

            // Column 3: ComboBox for Region
            ComboBox {
                visible: column === 3
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.leftMargin: Kirigami.Units.smallSpacing
                anchors.rightMargin: Kirigami.Units.smallSpacing
                model: ["Europe", "Americas", "Asia"]
                currentIndex: model.indexOf(display)
            }

            // Column 5: Action Button
            Button {
                id: actionButton
                visible: column === 5
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                width: Kirigami.Units.gridUnit * 6
                height: parent.height - Kirigami.Units.gridUnit * 0.8

                text: parent.display // "Start" or "Stop"
                icon.name: parent.display === "Start" ? "media-playback-playing" : "media-playback-stopped"

                background: Rectangle {
                    color: actionButton.text === "Start" ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.negativeTextColor
                    radius: 4
                    opacity: actionButton.pressed ? 0.8 : (actionButton.hovered ? 0.9 : 1.0)
                }

                contentItem: RowLayout {
                    spacing: Kirigami.Units.smallSpacing
                    Item {
                        Layout.fillWidth: true
                    }
                    Kirigami.Icon {
                        source: actionButton.icon.name
                        Layout.preferredWidth: Kirigami.Units.gridUnit
                        Layout.preferredHeight: Kirigami.Units.gridUnit
                        Layout.alignment: Qt.AlignVCenter
                        color: Kirigami.Theme.highlightedTextColor
                    }
                    Label {
                        text: actionButton.text
                        Layout.alignment: Qt.AlignVCenter
                        color: Kirigami.Theme.highlightedTextColor
                        font.bold: true
                    }
                    Item {
                        Layout.fillWidth: true
                    }
                }

                onClicked: {
                    // Logic to toggle state would go here
                }
            }
        }
    }
}
