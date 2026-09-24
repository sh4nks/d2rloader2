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

        background: Rectangle {
            color: root.running ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
            radius: 4
            opacity: !button.enabled ? 0.5 : (button.pressed ? 0.8 : (button.hovered ? 0.9 : 1.0))
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
                    color: Kirigami.Theme.highlightedTextColor
                }

                Label {
                    anchors.verticalCenter: parent.verticalCenter
                    text: button.text
                    color: Kirigami.Theme.highlightedTextColor
                    font.bold: true
                }
            }
        }

        onClicked: root.triggered()
    }
}
