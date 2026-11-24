pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import org.kde.kirigami as Kirigami

/**
 * One row of the Diablo Clone table: a realm and how far it has progressed
 * in each of the game modes shown.
 */
RowLayout {
    id: root

    /**
     * Width the region and mode columns share with the table header.
     */
    readonly property real labelWidth: Kirigami.Units.gridUnit * 5

    property alias regionName: regionLabel.text

    /**
     * How far Diablo Clone has progressed on this realm, one value per mode.
     */
    property var progresses: []

    /**
     * Highest progress the API reports.
     */
    readonly property int maxProgress: 6

    Layout.fillWidth: true
    spacing: Kirigami.Units.gridUnit

    Label {
        id: regionLabel
        color: Kirigami.Theme.textColor
        Layout.preferredWidth: root.labelWidth
    }

    Repeater {
        model: root.progresses

        delegate: RealmProgressBar {
            required property int modelData

            progress: modelData
            maxProgress: root.maxProgress
            Layout.fillWidth: true
            Layout.preferredWidth: 1
        }
    }
}
