import QtQuick
import org.kde.kirigami as Kirigami

/**
 * Shared chrome for every cell of the accounts table: the row highlight,
 * the separator towards the next row and the padding cell content is
 * expected to leave towards it.
 */
Rectangle {
    id: root

    /**
     * Whether the row this cell belongs to is currently hovered.
     */
    property bool highlighted: false

    /**
     * Padding cell content should keep towards the row separator.
     */
    readonly property real verticalPadding: Kirigami.Units.gridUnit * 0.4

    implicitHeight: Kirigami.Units.gridUnit * 2.5
    color: root.highlighted ? Kirigami.Theme.activeBackgroundColor : Kirigami.Theme.backgroundColor

    Kirigami.Separator {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
    }
}
