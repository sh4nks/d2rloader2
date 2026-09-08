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
    signal editAccountClicked(int rowIndex)
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
                onTriggered: root.editAccountClicked(contextMenu.currentRowIndex)
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

            // Cells live in their own components and cannot reach the view, so
            // the hovered row is tracked here for all of them at once.
            HoverHandler {
                id: rowHoverHandler
                onPointChanged: {
                    const position = tableView.mapToItem(tableView.contentItem, rowHoverHandler.point.position);
                    tableView.hoveredRow = tableView.cellAtPosition(position, true).y;
                }
                onHoveredChanged: if (!rowHoverHandler.hovered) {
                    tableView.hoveredRow = -1;
                }
            }

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
                    root.editAccountClicked(tableView.hoveredRow);
                }
            }

            model: root.modelData

            delegate: DelegateChooser {
                // Column 0: Status Indicator
                DelegateChoice {
                    column: 0
                    delegate: CellDelegateStatus {
                        required property var model
                        required property int row

                        highlighted: row === tableView.hoveredRow
                        running: model.status === D2R.ProfileState.Running
                    }
                }

                // Column 1: Account
                DelegateChoice {
                    column: 1
                    delegate: CellDelegateLabel {
                        required property var model
                        required property int row

                        highlighted: row === tableView.hoveredRow
                        text: model.profileName ?? ""
                    }
                }

                // Column 2: Auth Method
                DelegateChoice {
                    column: 2
                    delegate: CellDelegateComboBox {
                        required property var model
                        required property int row

                        highlighted: row === tableView.hoveredRow
                        options: D2R.AuthMethodModel
                        value: model.authMethod
                        onValueSelected: newValue => model.authMethod = newValue
                    }
                }

                // Column 3: Region
                DelegateChoice {
                    column: 3
                    delegate: CellDelegateComboBox {
                        required property var model
                        required property int row

                        highlighted: row === tableView.hoveredRow
                        options: D2R.RegionModel
                        value: model.region
                        onValueSelected: newValue => model.region = newValue
                    }
                }

                // Column 4: Game Parameters
                DelegateChoice {
                    column: 4
                    delegate: CellDelegateLabel {
                        required property var model
                        required property int row

                        highlighted: row === tableView.hoveredRow
                        monospace: true
                        text: model.gameParameters ?? ""
                    }
                }

                // Column 5: Action Button
                DelegateChoice {
                    column: 5
                    delegate: CellDelegateAction {
                        required property var model
                        required property int row

                        highlighted: row === tableView.hoveredRow
                        running: model.status === D2R.ProfileState.Running
                        onTriggered: {
                            // Logic to toggle state
                        }
                    }
                }
            }
        }
    }
}
