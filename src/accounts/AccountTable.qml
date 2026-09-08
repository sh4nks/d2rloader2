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
    signal editAccountClicked(int rowIndex)
    required property var modelData

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
            Layout.fillWidth: true

            delegate: Rectangle {
                id: headerDelegate
                required property string display
                required property int column

                // Text columns are left aligned and share the cells' wider padding.
                readonly property bool isTextColumn: headerDelegate.column === ProfileColumn.Name || headerDelegate.column === ProfileColumn.GameParameters

                implicitWidth: tableView.columnWidthProvider(headerDelegate.column)
                implicitHeight: Kirigami.Units.gridUnit * 2
                color: Kirigami.Theme.alternateBackgroundColor

                Kirigami.Separator {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                }

                Label {
                    anchors.fill: parent
                    anchors.leftMargin: headerDelegate.isTextColumn ? Kirigami.Units.gridUnit : Kirigami.Units.smallSpacing
                    anchors.rightMargin: headerDelegate.isTextColumn ? Kirigami.Units.gridUnit : Kirigami.Units.smallSpacing
                    text: headerDelegate.display
                    font.bold: true
                    color: Kirigami.Theme.textColor
                    horizontalAlignment: headerDelegate.isTextColumn ? Text.AlignLeft : Text.AlignHCenter
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

            readonly property real statusColumnWidth: Kirigami.Units.gridUnit * 2
            readonly property real actionsColumnWidth: Kirigami.Units.gridUnit * 7

            columnWidthProvider: function (column) {
                if (column === ProfileColumn.Status) {
                    return tableView.statusColumnWidth;
                }
                if (column === ProfileColumn.Actions) {
                    return tableView.actionsColumnWidth;
                }
                // The remaining columns share whatever is left over.
                return (tableView.width - tableView.statusColumnWidth - tableView.actionsColumnWidth) / (ProfileColumn.Count - 2);
            }
            onWidthChanged: tableView.forceLayout()

            TapHandler {
                acceptedButtons: Qt.LeftButton | Qt.RightButton
                onTapped: (eventPoint, button) => {
                    if (button === Qt.LeftButton) {
                        return;
                    }
                    if (tableView.hoveredRow > -1) {
                        contextMenu.currentRowData = root.modelData.getProfile(tableView.hoveredRow);
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
                    column: ProfileColumn.Status
                    delegate: CellDelegateStatus {
                        required property var model
                        required property int row

                        highlighted: row === tableView.hoveredRow
                        running: model.status === ProfileState.Running
                    }
                }

                // Column 1: Account
                DelegateChoice {
                    column: ProfileColumn.Name
                    delegate: CellDelegateLabel {
                        required property var model
                        required property int row

                        highlighted: row === tableView.hoveredRow
                        text: model.profileName ?? ""
                    }
                }

                // Column 2: Auth Method
                DelegateChoice {
                    column: ProfileColumn.AuthMethod
                    delegate: CellDelegateComboBox {
                        required property var model
                        required property int row

                        highlighted: row === tableView.hoveredRow
                        options: AuthMethodModel
                        value: model.authMethod
                        onValueSelected: newValue => model.authMethod = newValue
                    }
                }

                // Column 3: Region
                DelegateChoice {
                    column: ProfileColumn.Region
                    delegate: CellDelegateComboBox {
                        required property var model
                        required property int row

                        highlighted: row === tableView.hoveredRow
                        options: RegionModel
                        value: model.region
                        onValueSelected: newValue => model.region = newValue
                    }
                }

                // Column 4: Game Parameters
                DelegateChoice {
                    column: ProfileColumn.GameParameters
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
                    column: ProfileColumn.Actions
                    delegate: CellDelegateAction {
                        required property var model
                        required property int row

                        highlighted: row === tableView.hoveredRow
                        running: model.status === ProfileState.Running
                        onTriggered: {
                            // Logic to toggle state
                        }
                    }
                }
            }
        }
    }
}
