pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import Qt.labs.qmlmodels
import org.kde.kirigami as Kirigami
import org.kde.ki18n

Kirigami.Card {
    id: root
    padding: 0
    signal settingsClicked
    signal addAccountClicked
    signal editAccountClicked(int rowIndex)
    signal startStopClicked(int rowIndex)
    signal launchSequenceClicked(int sequenceIndex)
    signal cancelSequenceClicked
    required property var modelData
    property bool sequenceRunning: false

    header: Item {
        implicitHeight: headerLayout.implicitHeight + Kirigami.Units.smallSpacing * 2
        RowLayout {
            id: headerLayout
            anchors.fill: parent
            anchors.leftMargin: Kirigami.Units.smallSpacing * 2
            anchors.rightMargin: Kirigami.Units.smallSpacing * 2
            anchors.topMargin: Kirigami.Units.smallSpacing * 2

            Kirigami.Heading {
                text: KI18n.i18nc("@title", "Accounts")
                level: 2
                Layout.alignment: Qt.AlignVCenter
            }

            Item {
                Layout.fillWidth: true
            }

            ComboBox {
                id: launchSequenceSelector
                model: LaunchSequenceManager
                textRole: "name"
                enabled: !root.sequenceRunning && count > 0
                displayText: count > 0 ? currentText : KI18n.i18nc("@item:incombobox", "No Launch Sequences")
                Layout.preferredWidth: Kirigami.Units.gridUnit * 10
                onActivated: index => LaunchSequenceManager.currentIndex = index

                // Picking an entry overwrites currentIndex, which would break a
                // plain binding.
                Binding {
                    target: launchSequenceSelector
                    property: "currentIndex"
                    value: LaunchSequenceManager.currentIndex
                }
            }

            Button {
                text: root.sequenceRunning ? KI18n.i18nc("@action:button", "Cancel Sequence") : KI18n.i18nc("@action:button", "Launch Sequence")
                icon.name: root.sequenceRunning ? "media-playback-stop" : "media-playback-start"
                enabled: root.sequenceRunning || launchSequenceSelector.currentIndex >= 0
                onClicked: {
                    if (root.sequenceRunning) {
                        root.cancelSequenceClicked();
                    } else {
                        root.launchSequenceClicked(launchSequenceSelector.currentIndex);
                    }
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
                text: KI18n.i18nc("@action:inmenu", "Clone")
                icon.name: "edit-copy"
                onTriggered: root.modelData.cloneProfile(contextMenu.currentRowIndex)
            }
            MenuItem {
                text: KI18n.i18nc("@action:inmenu", "Edit")
                icon.name: "edit-entry"
                onTriggered: root.editAccountClicked(contextMenu.currentRowIndex)
            }
            MenuItem {
                text: KI18n.i18nc("@action:inmenu", "Delete")
                icon.name: "edit-delete"
                onTriggered: deletePrompt.open()
            }
            MenuSeparator {}
            MenuItem {
                text: KI18n.i18nc("@action:inmenu", "Move Up")
                icon.name: "arrow-up"
                enabled: contextMenu.currentRowIndex > 0
                onTriggered: root.modelData.moveUp(contextMenu.currentRowIndex)
            }
            MenuItem {
                text: KI18n.i18nc("@action:inmenu", "Move Down")
                icon.name: "arrow-down"
                enabled: contextMenu.currentRowIndex < root.modelData.rowCount() - 1
                onTriggered: root.modelData.moveDown(contextMenu.currentRowIndex)
            }
        }

        Kirigami.PromptDialog {
            id: deletePrompt

            title: KI18n.i18nc("@title:window", "Delete Account")
            subtitle: contextMenu.currentRowData ? KI18n.i18nc("@info", "Delete the account \"%1\"? This cannot be undone.", contextMenu.currentRowData.profileName) : ""
            standardButtons: Kirigami.Dialog.Cancel
            showCloseButton: false

            customFooterActions: [
                Kirigami.Action {
                    text: KI18n.i18nc("@action:button", "Delete")
                    icon.name: "edit-delete"
                    onTriggered: {
                        root.modelData.removeProfileAt(contextMenu.currentRowIndex);
                        deletePrompt.close();
                    }
                }
            ]
        }

        Menu {
            id: contextMenuAdd
            MenuItem {
                text: KI18n.i18nc("@action:inmenu", "Add new Account")
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

            /**
             * The auth method, region and action cells carry controls of their
             * own, so a click there is aimed at the control rather than at the
             * row.
             */
            function columnHasControl(column: int): bool {
                return column === ProfileColumn.AuthMethod || column === ProfileColumn.Region || column === ProfileColumn.Actions;
            }

            /**
             * The cell under the given point, in this view's coordinates.
             */
            function cellAt(point: point): point {
                return tableView.cellAtPosition(tableView.mapToItem(tableView.contentItem, point), true);
            }

            // Cells live in their own components and cannot reach the view, so
            // the hovered cell is tracked here for all of them at once.
            HoverHandler {
                id: rowHoverHandler
                onPointChanged: {
                    tableView.hoveredRow = tableView.cellAt(rowHoverHandler.point.position).y;
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

                    const cell = tableView.cellAt(eventPoint.position);
                    if (cell.y === -1 || tableView.columnHasControl(cell.x)) {
                        return;
                    }
                    root.editAccountClicked(cell.y);
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
                        starting: model.status === ProfileState.Starting
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
                        starting: model.status === ProfileState.Starting
                        onTriggered: root.startStopClicked(row)
                    }
                }
            }
        }
    }
}
