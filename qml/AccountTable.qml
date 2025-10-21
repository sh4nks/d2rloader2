pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import Qt.labs.qmlmodels

TableView {
    id: tableView
    anchors.fill: parent
    columnSpacing: 1
    rowSpacing: 1
    clip: true
    boundsBehavior: Flickable.StopAtBounds

    columnWidthProvider: function (column) {
        return tableView.model ? tableView.width / 5 : 0;
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
                "authMethod": "black",
                "region": "Europe",
                "launchParameters": "-w",
                "actions": "Start/Stop/Running"
            },
            {
                "account": "dog",
                "authMethod": "black",
                "region": "Europe",
                "launchParameters": "-w",
                "actions": "Start/Stop/Running"
            },
            {
                "account": "sheep",
                "authMethod": "black",
                "region": "Europe",
                "launchParameters": "-w",
                "actions": "Start/Stop/Running"
            },
            {
                "account": "sheep",
                "authMethod": "black",
                "region": "Europe",
                "launchParameters": "-w",
                "actions": "Start/Stop/Running"
            }
        ]
    }

    delegate: TableViewDelegate {}
    Text {
        text: tableView.model.display
        anchors.centerIn: parent
    }
}
