import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import org.kde.kirigami as Kirigami

/**
 * One row of the Diablo Clone table: a realm and how far its ladder and
 * non-ladder variant have progressed.
 */
RowLayout {
    id: root

    /**
     * Width the region and mode columns share with the table header.
     */
    readonly property real labelWidth: Kirigami.Units.gridUnit * 5

    property alias regionName: regionLabel.text

    /**
     * How far Diablo Clone has progressed on this realm.
     */
    property int progress: 0

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

    RealmProgressBar {
        progress: root.progress
        maxProgress: root.maxProgress
        Layout.fillWidth: true
    }
}
