import QtQuick
import QtQuick.Controls
import org.kde.kirigami as Kirigami
import org.kde.ki18n

/**
 * Cell holding the button that starts or stops the profile of this row.
 */
CellDelegate {
    id: root

    property bool running: false
    property bool starting: false

    signal triggered

    Button {
        id: button

        Kirigami.Theme.colorSet: Kirigami.Theme.Button
        Kirigami.Theme.inherit: false

        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: root.verticalPadding
        anchors.bottomMargin: root.verticalPadding
        width: Kirigami.Units.gridUnit * 6
        enabled: !root.starting

        text: root.running ? KI18n.i18nc("@action:button", "Stop") : (root.starting ? KI18n.i18nc("@action:button", "Starting") : KI18n.i18nc("@action:button", "Start"))
        icon.name: root.running ? "media-playback-pause" : "media-playback-start"

        readonly property color accent: root.running ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
        readonly property bool filled: button.hovered || button.pressed
        readonly property color foreground: button.filled ? Kirigami.Theme.highlightedTextColor : button.accent

        background: Rectangle {
            color: button.filled ? button.accent : Qt.rgba(button.accent.r, button.accent.g, button.accent.b, 0.12)
            border.color: Qt.rgba(button.accent.r, button.accent.g, button.accent.b, 0.6)
            border.width: 1
            radius: 4
            opacity: !button.enabled ? 0.5 : (button.pressed ? 0.8 : 1.0)
        }

        contentItem: Item {
            Row {
                anchors.centerIn: parent
                spacing: Kirigami.Units.smallSpacing

                Kirigami.Icon {
                    anchors.verticalCenter: parent.verticalCenter
                    source: button.icon.name
                    width: Kirigami.Units.gridUnit * 0.8
                    height: Kirigami.Units.gridUnit * 0.8
                    color: button.foreground
                }

                Label {
                    anchors.verticalCenter: parent.verticalCenter
                    text: button.text
                    color: button.foreground
                    font.bold: true
                }
            }
        }

        onClicked: root.triggered()
    }
}
