import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import org.kde.kirigami as Kirigami

/**
 * One terror zone row: a time badge followed by the zone's name.
 */
Rectangle {
    id: root

    property alias time: timeLabel.text
    property alias zoneName: zoneLabel.text

    /**
     * Whether this is the zone that is currently active, as opposed to a
     * prediction for a later slot.
     */
    property bool current: false

    Layout.fillWidth: true
    implicitHeight: Kirigami.Units.gridUnit * 3.5
    color: Kirigami.Theme.alternateBackgroundColor
    border.color: Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.1)
    border.width: 1
    radius: 6

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: Kirigami.Units.gridUnit
        anchors.rightMargin: Kirigami.Units.gridUnit
        spacing: Kirigami.Units.gridUnit

        Rectangle {
            color: Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.1)
            radius: 4
            Layout.preferredWidth: Kirigami.Units.gridUnit * 4
            Layout.preferredHeight: Kirigami.Units.gridUnit * 1.8

            Label {
                id: timeLabel
                anchors.centerIn: parent
                font.bold: true
                font.family: Kirigami.Theme.fixedWidthFont.family
                color: root.current ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.textColor
            }
        }

        Kirigami.Icon {
            source: "go-next"
            Layout.preferredWidth: Kirigami.Units.smallSpacing * 2
            Layout.preferredHeight: Kirigami.Units.smallSpacing * 2
            opacity: 0.5
            color: Kirigami.Theme.textColor
        }

        Label {
            id: zoneLabel
            font.pointSize: 12
            font.weight: Font.Medium
            color: Kirigami.Theme.textColor
            Layout.fillWidth: true
        }
    }
}
