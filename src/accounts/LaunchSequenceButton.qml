pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import org.kde.kirigami as Kirigami
import org.kde.ki18n

/**
 * Starts the selected launch sequence, or cancels the one that is running.
 * The arrow at its trailing edge picks which sequence is selected.
 */
Item {
    id: root

    property bool sequenceRunning: false

    signal launchClicked(int index)
    signal cancelClicked

    // Quiet while idle, so that only a running sequence draws the eye.
    readonly property color accent: root.sequenceRunning ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.textColor
    readonly property color outline: root.tint(root.sequenceRunning ? 0.6 : 0.25)
    readonly property color hoverFill: root.sequenceRunning ? root.accent : root.tint(0.15)
    readonly property color hoverForeground: root.sequenceRunning ? Kirigami.Theme.highlightedTextColor : root.accent

    function tint(alpha: real): color {
        return Qt.rgba(root.accent.r, root.accent.g, root.accent.b, alpha);
    }

    implicitWidth: Kirigami.Units.gridUnit * 6
    implicitHeight: Kirigami.Units.gridUnit * 1.5

    Rectangle {
        anchors.fill: parent
        radius: 4
        color: root.tint(root.sequenceRunning ? 0.12 : 0.06)
        border.color: root.outline
        border.width: 1
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        AbstractButton {
            id: launchButton

            readonly property bool filled: launchButton.hovered || launchButton.pressed
            readonly property color foreground: launchButton.filled ? root.hoverForeground : root.accent

            Layout.fillWidth: true
            Layout.fillHeight: true

            text: root.sequenceRunning ? KI18n.i18nc("@action:button", "Cancel") : LaunchSequenceManager.currentName

            ToolTip.text: root.sequenceRunning ? KI18n.i18nc("@info:tooltip", "Cancel the running launch sequence") : KI18n.i18nc("@info:tooltip %1 is the name of a launch sequence", "Launch the sequence \"%1\"", LaunchSequenceManager.currentName)
            ToolTip.visible: launchButton.hovered
            ToolTip.delay: Kirigami.Units.toolTipDelay

            background: Rectangle {
                radius: 4
                color: launchButton.filled ? root.hoverFill : "transparent"
            }

            contentItem: Item {
                RowLayout {
                    anchors.centerIn: parent
                    width: Math.min(implicitWidth, parent.width - Kirigami.Units.smallSpacing * 2)
                    spacing: Kirigami.Units.smallSpacing

                    Kirigami.Icon {
                        source: root.sequenceRunning ? "media-playback-stop" : "media-playback-start"
                        color: launchButton.foreground
                        Layout.preferredWidth: Kirigami.Units.gridUnit * 0.8
                        Layout.preferredHeight: Kirigami.Units.gridUnit * 0.8
                    }

                    Label {
                        text: launchButton.text
                        color: launchButton.foreground
                        font.bold: true
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }
                }
            }

            onClicked: {
                if (root.sequenceRunning) {
                    root.cancelClicked();
                } else {
                    root.launchClicked(LaunchSequenceManager.currentIndex);
                }
            }
        }

        Rectangle {
            visible: pickerButton.visible
            color: root.outline
            Layout.preferredWidth: 1
            Layout.fillHeight: true
        }

        AbstractButton {
            id: pickerButton

            readonly property bool filled: pickerButton.hovered || pickerButton.pressed

            visible: !root.sequenceRunning
            Layout.preferredWidth: Kirigami.Units.gridUnit * 1.4
            Layout.fillHeight: true

            ToolTip.text: KI18n.i18nc("@info:tooltip", "Choose the launch sequence")
            ToolTip.visible: pickerButton.hovered
            ToolTip.delay: Kirigami.Units.toolTipDelay

            background: Rectangle {
                radius: 4
                color: pickerButton.filled ? root.hoverFill : "transparent"
            }

            contentItem: Item {
                Kirigami.Icon {
                    anchors.centerIn: parent
                    source: "arrow-down"
                    color: root.accent
                    width: Kirigami.Units.gridUnit * 0.7
                    height: Kirigami.Units.gridUnit * 0.7
                }
            }

            onClicked: sequenceMenu.popup(pickerButton, 0, pickerButton.height)

            Menu {
                id: sequenceMenu

                Instantiator {
                    model: LaunchSequenceManager

                    delegate: MenuItem {
                        required property string name
                        required property int index

                        text: name
                        checkable: true
                        checked: index === LaunchSequenceManager.currentIndex
                        onTriggered: LaunchSequenceManager.currentIndex = index
                    }

                    onObjectAdded: (index, object) => sequenceMenu.insertItem(index, object as MenuItem)
                    onObjectRemoved: (index, object) => sequenceMenu.removeItem(object as MenuItem)
                }
            }
        }
    }
}
