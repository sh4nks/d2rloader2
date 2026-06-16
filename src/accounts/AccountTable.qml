pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import Qt.labs.qmlmodels
import org.kde.kirigami as Kirigami
import com.someblocks.d2rloader as D2R

Kirigami.Card {
    id: root
    padding: 0
    signal settingsClicked
    signal addAccountClicked
    signal editAccountClicked(var accountData)
    required property var modelData

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

            ComboBox {
                id: launchSequenceSelector
                model: [i18nc("@item:incombobox", "Default Sequence")]
                Layout.preferredWidth: Kirigami.Units.gridUnit * 10
            }

            Button {
                text: i18nc("@action:button", "Launch Sequence")
                icon.name: "media-playback-start"
                onClicked: {
                    // Logic to launch the selected sequence
                }
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

        Menu {
            id: contextMenu
            property var currentRowData
            property int currentRowIndex

            MenuItem {
                text: i18nc("@action:inmenu", "Clone")
                icon.name: "edit-copy"
                onTriggered: {
                    // Logic to clone contextMenu.currentRowData
                }
            }
            MenuItem {
                text: i18nc("@action:inmenu", "Edit")
                icon.name: "edit-entry"
                onTriggered: root.editAccountClicked(contextMenu.currentRowData)
            }
            MenuItem {
                text: i18nc("@action:inmenu", "Delete")
                icon.name: "edit-delete"
                onTriggered: {
                    // Logic to delete index contextMenu.currentRowIndex
                }
            }
            MenuSeparator {}
            MenuItem {
                text: i18nc("@action:inmenu", "Move Up")
                icon.name: "arrow-up"
                enabled: contextMenu.currentRowIndex > 0
                onTriggered: {
                    // Logic to move row up
                }
            }
            MenuItem {
                text: i18nc("@action:inmenu", "Move Down")
                icon.name: "arrow-down"
                enabled: contextMenu.currentRowIndex < root.modelData.rowCount() - 1
                onTriggered: {
                    // Logic to move row down
                }
            }
        }

        Menu {
            id: contextMenuAdd
            MenuItem {
                text: i18nc("@action:inmenu", "Add new Account")
                icon.name: "list-add"
                onTriggered: root.addAccountClicked()
            }
        }

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

            property int hoveredRow: -1

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

            TapHandler {
                acceptedButtons: Qt.LeftButton | Qt.RightButton
                onTapped: (eventPoint, button) => {
                    if (button === Qt.LeftButton) {
                        return;
                    }
                    if (tableView.hoveredRow > -1) {
                        contextMenu.currentRowData = tableView.model.data[tableView.hoveredRow];
                        contextMenu.currentRowIndex = tableView.hoveredRow;
                        contextMenu.popup();
                    } else {
                        contextMenuAdd.popup();
                    }
                }
                onDoubleTapped: (eventPoint, button) => {
                    if (button === Qt.RightButton) {
                        return;
                    }

                    if (tableView.hoveredRow === -1) {
                        return;
                    }

                    let rowData = tableView.model.data[tableView.hoveredRow];
                    root.editAccountClicked(rowData);
                }
            }

            model: root.modelData

            D2R.AuthMethodModel {
                id: authMethodModel
            }

            D2R.RegionModel {
                id: regionModel
            }

            delegate: Rectangle {
                id: cellDelegate
                required property var model
                required property int column
                required property int row

                implicitWidth: tableView.columnWidthProvider(cellDelegate.column)
                implicitHeight: Kirigami.Units.gridUnit * 2.5
                color: cellDelegate.row === tableView.hoveredRow ? Kirigami.Theme.activeBackgroundColor : Kirigami.Theme.backgroundColor

                HoverHandler {
                    onHoveredChanged: {
                        if (hovered) {
                            tableView.hoveredRow = cellDelegate.row;
                        } else if (tableView.hoveredRow === cellDelegate.row) {
                            tableView.hoveredRow = -1;
                        }
                    }
                }

                // border
                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    height: 1
                    color: Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.1)
                }

                // Column 0: Status Indicator (Circle)
                Rectangle {
                    visible: cellDelegate.column === 0
                    anchors.centerIn: parent
                    width: Kirigami.Units.gridUnit * 0.6
                    height: width
                    radius: width / 2
                    color: cellDelegate.model.status === D2R.ProfileState.Running ? Kirigami.Theme.positiveTextColor : "gray"
                }

                // Columns 1: Account
                Label {
                    visible: cellDelegate.column === 1
                    anchors.fill: parent
                    anchors.topMargin: Kirigami.Units.gridUnit * 0.4
                    anchors.bottomMargin: Kirigami.Units.gridUnit * 0.4
                    anchors.leftMargin: Kirigami.Units.gridUnit
                    anchors.rightMargin: Kirigami.Units.gridUnit
                    text: cellDelegate.model.profileName ?? ""
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
                    model: authMethodModel
                    textRole: "name"   // Displays literal string (e.g., "HighContrast")
                    valueRole: "value"

                    Component.onCompleted: {
                        authComboBox.currentIndex = authComboBox.indexOfValue(cellDelegate.model.authMethod);
                    }

                    onActivated: {
                        console.log("Selected Name: " + currentText);
                        console.log("Selected Raw Enum Value: " + currentValue);
                    }
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
                    model: regionModel
                    currentIndex: regionComboBox.indexOfValue(cellDelegate.model.region)
                    textRole: "name"
                    valueRole: "value" // Evaluates to raw numerical enum index (e.g., 3)

                    Component.onCompleted: {
                        regionComboBox.currentIndex = regionComboBox.indexOfValue(cellDelegate.model.region);
                    }

                    onActivated: {
                        console.log("Selected Name: " + currentText);
                        console.log("Selected Raw Enum Value: " + currentValue);
                    }
                }

                // Column 4: Game Parameters
                Label {
                    visible: cellDelegate.column === 4
                    anchors.fill: parent
                    anchors.topMargin: Kirigami.Units.gridUnit * 0.4
                    anchors.bottomMargin: Kirigami.Units.gridUnit * 0.4
                    anchors.leftMargin: Kirigami.Units.gridUnit
                    anchors.rightMargin: Kirigami.Units.gridUnit
                    text: cellDelegate.model.gameParameters ?? ""
                    color: Kirigami.Theme.textColor
                    font.family: cellDelegate.column === 4 ? "monospace" : ""
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
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

                    text: cellDelegate.model.status === D2R.ProfileState.Running ? "Stop" : "Start"
                    icon.name: cellDelegate.model.status === D2R.ProfileState.Running ? "media-playback-pause" : "media-playback-start"

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
