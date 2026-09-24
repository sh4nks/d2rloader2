import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

/**
 * The header row shared by the cards on the main window: a title on the
 * left and whatever actions the card needs on the right.
 */
RowLayout {
    id: root

    /**
     * Actions shown at the trailing edge, in order.
     */
    default property alias actions: actionRow.data

    property alias title: heading.text

    Layout.fillWidth: true
    Layout.margins: Kirigami.Units.smallSpacing

    Kirigami.Heading {
        id: heading
        level: 2
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignVCenter
        elide: Text.ElideRight
    }

    RowLayout {
        id: actionRow
        spacing: Kirigami.Units.smallSpacing
        Layout.alignment: Qt.AlignVCenter
    }
}
