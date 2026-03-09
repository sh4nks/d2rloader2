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
        // The model provides the text for the header labels
        model: ["Account", "Auth Method", "Region", "Launch Parameters", "Actions"]
        Layout.fillWidth: true

        delegate: Rectangle {
            // Properties must be declared 'required' due to pragma ComponentBehavior: Bound
            required property var modelData
            required property int column

            implicitWidth: tableView.columnWidthProvider(column)
            implicitHeight: Kirigami.Units.gridUnit * 2
            color: Kirigami.Theme.alternateBackgroundColor

            Label {
                anchors.fill: parent
                anchors.margins: Kirigami.Units.smallSpacing
                text: parent.modelData
                font.bold: true
                color: Kirigami.Theme.textColor
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }
        }
    }

    TableView {
        id: tableView
        Layout.fillWidth: true
        Layout.fillHeight: true
        columnSpacing: 1
        rowSpacing: 1
        clip: true
        boundsBehavior: Flickable.StopAtBounds

        // Distribute columns evenly
        columnWidthProvider: function (column) {
            return tableView.width / 5;
        }
        onWidthChanged: tableView.forceLayout()

        model: TableModel {
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
                    "account": "cow",
                    "authMethod": "Token",
                    "region": "Europe",
                    "launchParameters": "-w",
                    "actions": "Running"
                },
                {
                    "account": "dog",
                    "authMethod": "Token",
                    "region": "Europe",
                    "launchParameters": "-w",
                    "actions": "Start"
                },
                {
                    "account": "sheep",
                    "authMethod": "Password",
                    "region": "Europe",
                    "launchParameters": "-w",
                    "actions": "Start"
                },
                {
                    "account": "goat",
                    "authMethod": "Steam",
                    "region": "Europe",
                    "launchParameters": "-w",
                    "actions": "Start"
                }
            ]
        }

        delegate: Rectangle {
            // Roles and properties must be declared 'required' due to pragma ComponentBehavior: Bound
            required property var display
            required property int column

            implicitWidth: tableView.columnWidthProvider(column)
            implicitHeight: Kirigami.Units.gridUnit * 1.5
            color: Kirigami.Theme.backgroundColor

            Label {
                anchors.fill: parent
                anchors.leftMargin: Kirigami.Units.smallSpacing
                anchors.rightMargin: Kirigami.Units.smallSpacing
                text: parent.display
                color: Kirigami.Theme.textColor
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }
        }
    }
}
