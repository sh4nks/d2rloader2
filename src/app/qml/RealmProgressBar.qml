pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import org.kde.kirigami as Kirigami
import org.kde.ki18n

/**
 * A Diablo Clone progress value rendered as one segment per stage with its
 * "n/max" reading, coloured by how close the realm is to the spawn.
 * Progress 0 means the realm has not been reported on yet.
 */
RowLayout {
    id: root

    property int progress: 0
    property int maxProgress: 6

    spacing: Kirigami.Units.smallSpacing

    readonly property bool advanced: root.progress >= 3
    readonly property color stageColor: root.progress >= 5 ? Kirigami.Theme.negativeTextColor : (root.advanced ? Kirigami.Theme.neutralTextColor : Kirigami.Theme.highlightColor)

    RowLayout {
        spacing: Kirigami.Units.smallSpacing
        Layout.fillWidth: true

        Repeater {
            model: root.maxProgress

            delegate: Rectangle {
                required property int index

                Layout.fillWidth: true
                implicitHeight: Kirigami.Units.smallSpacing * 2
                radius: height / 2
                color: index < root.progress ? root.stageColor : Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.12)
            }
        }
    }

    Label {
        text: root.progress > 0 ? KI18n.i18nc("@info:status Diablo Clone progress, e.g. 4/6", "%1/%2", root.progress, root.maxProgress) : KI18n.i18nc("@info:status no Diablo Clone data for this realm", "-")
        color: root.advanced ? root.stageColor : Kirigami.Theme.textColor
        opacity: root.advanced ? 1 : 0.7
        font.family: Kirigami.Theme.fixedWidthFont.family
        Layout.preferredWidth: Kirigami.Units.gridUnit * 2
        horizontalAlignment: Text.AlignRight
    }
}
