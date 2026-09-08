import QtQuick
import QtQuick.Controls
import org.kde.kirigami as Kirigami

/**
 * Cell showing a single line of read-only text.
 */
CellDelegate {
    id: root

    property alias text: label.text

    /**
     * Render the text in the theme's fixed width font, for values that are
     * read character by character such as launch parameters.
     */
    property bool monospace: false

    Label {
        id: label

        anchors.fill: parent
        anchors.topMargin: root.verticalPadding
        anchors.bottomMargin: root.verticalPadding
        anchors.leftMargin: Kirigami.Units.gridUnit
        anchors.rightMargin: Kirigami.Units.gridUnit

        color: Kirigami.Theme.textColor
        font.family: root.monospace ? Kirigami.Theme.fixedWidthFont.family : Kirigami.Theme.defaultFont.family
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
}
