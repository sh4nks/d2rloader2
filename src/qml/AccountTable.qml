pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import Qt.labs.qmlmodels
import org.kde.kirigami as Kirigami

Kirigami.Card {
    id: root
    padding: 0
    signal settingsClicked
    signal addAccountClicked
    signal editAccountClicked(var accountData)

    Layout.fillWidth: true
    Layout.fillHeight: true
    Layout.margins: 0

    header: Item {
        implicitHeight: headerLayout.implicitHeight + Kirigami.Units.smallSpacing * 2
        RowLayout {
            id: headerLayout
            anchors.fill: parent
            anchors.leftMargin: Kirigami.Units.smallSpacing * 2
            anchors.rightMargin: Kirigami.Units.smallSpacing * 2
            anchors.topMargin: Kirigami.Units.smallSpacing * 2

            Kirigami.Heading {
                text: i18nc("@title", "Accounts")
                level: 2
                Layout.alignment: Qt.AlignVCenter
            }

            Item {
                Layout.fillWidth: true
            }

            Button {
                text: i18nc("@action:button", "Add Account")
                icon.name: "list-add"
                Kirigami.Theme.colorSet: Kirigami.Theme.Button
                Kirigami.Theme.inherit: false
                onClicked: root.addAccountClicked()
            }

            Button {
                icon.name: "settings-configure"
                onClicked: root.settingsClicked()
            }
        }
    }

    contentItem: ColumnLayout {
        spacing: 0
        Layout.margins: 0

        // Set Kirigami Theme properties for the table area
        Kirigami.Theme.inherit: true
        Kirigami.Theme.colorSet: Kirigami.Theme.View

        HorizontalHeaderView {
            id: horizontalHeader
            syncView: tableView
            // Column 0 is the status indicator (no text header)
            model: ["", i18nc("@title:column", "Account"), i18nc("@title:column", "Auth Method"), i18nc("@title:column", "Region"), i18nc("@title:column", "Launch Parameters"), i18nc("@title:column", "Actions")]
            Layout.fillWidth: true

            delegate: Rectangle {
                id: headerDelegate
                required property var modelData
                required property int column

                implicitWidth: tableView.columnWidthProvider(headerDelegate.column)
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
                    anchors.leftMargin: (headerDelegate.column === 1 || headerDelegate.column === 4) ? Kirigami.Units.gridUnit : Kirigami.Units.smallSpacing
                    anchors.rightMargin: (headerDelegate.column === 1 || headerDelegate.column === 4) ? Kirigami.Units.gridUnit : Kirigami.Units.smallSpacing
                    text: headerDelegate.modelData ?? ""
                    font.bold: true
                    color: Kirigami.Theme.textColor
                    horizontalAlignment: (headerDelegate.column === 1 || headerDelegate.column === 4) ? Text.AlignLeft : Text.AlignHCenter
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
                id: tableModel
                TableModelColumn {
                    display: "status"
                }
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
                id: cellDelegate
                required property var display
                required property int column
                required property int row

                implicitWidth: tableView.columnWidthProvider(cellDelegate.column)
                implicitHeight: Kirigami.Units.gridUnit * 2.5
                color: Kirigami.Theme.backgroundColor

                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    height: 1
                    color: Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.1)
                }

                TapHandler {
                    onDoubleTapped: {
                        let rowData = tableModel.rows[cellDelegate.row];
                        root.editAccountClicked(rowData);
                    }
                }

                // Column 0: Status Indicator (Circle)
                Rectangle {
                    visible: cellDelegate.column === 0
                    anchors.centerIn: parent
                    width: Kirigami.Units.gridUnit * 0.6
                    height: width
                    radius: width / 2
                    color: cellDelegate.display === "Running" ? Kirigami.Theme.positiveTextColor : "gray"
                }

                // Columns 1 & 4: Standard text display (Account, Launch Parameters)
                Label {
                    visible: cellDelegate.column === 1 || cellDelegate.column === 4
                    anchors.fill: parent
                    anchors.topMargin: Kirigami.Units.gridUnit * 0.4
                    anchors.bottomMargin: Kirigami.Units.gridUnit * 0.4
                    anchors.leftMargin: Kirigami.Units.gridUnit
                    anchors.rightMargin: Kirigami.Units.gridUnit
                    text: cellDelegate.display ?? ""
                    color: Kirigami.Theme.textColor
                    font.family: cellDelegate.column === 4 ? "monospace" : ""
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }

                // Column 2: ComboBox for Auth Method
                ComboBox {
                    id: authComboBox
                    Kirigami.Theme.colorSet: Kirigami.Theme.Button
                    Kirigami.Theme.inherit: false
                    visible: cellDelegate.column === 2
                    anchors.fill: parent
                    anchors.topMargin: Kirigami.Units.gridUnit * 0.4
                    anchors.bottomMargin: Kirigami.Units.gridUnit * 0.4
                    anchors.leftMargin: Kirigami.Units.smallSpacing
                    anchors.rightMargin: Kirigami.Units.smallSpacing
                    model: [i18nc("@item:incombobox", "Token"), i18nc("@item:incombobox", "Password"), i18nc("@item:incombobox", "Steam")]
                    currentIndex: authComboBox.model.indexOf(cellDelegate.display)
                }

                // Column 3: ComboBox for Region
                ComboBox {
                    id: regionComboBox
                    Kirigami.Theme.colorSet: Kirigami.Theme.Button
                    Kirigami.Theme.inherit: false
                    visible: cellDelegate.column === 3
                    anchors.fill: parent
                    anchors.topMargin: Kirigami.Units.gridUnit * 0.4
                    anchors.bottomMargin: Kirigami.Units.gridUnit * 0.4
                    anchors.leftMargin: Kirigami.Units.smallSpacing
                    anchors.rightMargin: Kirigami.Units.smallSpacing
                    model: [i18nc("@item:incombobox", "Europe"), i18nc("@item:incombobox", "Americas"), i18nc("@item:incombobox", "Asia")]
                    currentIndex: regionComboBox.model.indexOf(cellDelegate.display)
                }

                // Column 5: Action Button
                Button {
                    id: actionButton
                    Kirigami.Theme.colorSet: Kirigami.Theme.Button
                    Kirigami.Theme.inherit: false
                    visible: cellDelegate.column === 5
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.topMargin: Kirigami.Units.gridUnit * 0.4
                    anchors.bottomMargin: Kirigami.Units.gridUnit * 0.4
                    width: Kirigami.Units.gridUnit * 6

                    text: cellDelegate.display ?? ""
                    icon.name: cellDelegate.display === "Start" ? "media-playback-playing" : "media-playback-stopped"

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
                            source: actionButton.icon.name ?? ""
                            Layout.preferredWidth: Kirigami.Units.gridUnit * 0.8
                            Layout.preferredHeight: Kirigami.Units.gridUnit * 0.8
                            Layout.alignment: Qt.AlignVCenter
                            color: Kirigami.Theme.highlightedTextColor
                        }
                        Label {
                            text: actionButton.text ?? ""
                            Layout.alignment: Qt.AlignVCenter
                            color: Kirigami.Theme.highlightedTextColor
                            font.bold: true
                        }
                        Item {
                            Layout.fillWidth: true
                        }
                    }

                    onClicked: {
                        // Logic to toggle state
                    }
                }
            }
        }
    }
}
