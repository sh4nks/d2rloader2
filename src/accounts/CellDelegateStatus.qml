import QtQuick
import org.kde.kirigami as Kirigami

/**
 * Cell showing whether the profile of this row is currently running.
 */
CellDelegate {
    id: root

    property bool running: false
    property bool starting: false

    Rectangle {
        anchors.centerIn: parent
        width: Kirigami.Units.gridUnit * 0.6
        height: width
        radius: width / 2
        color: root.running ? Kirigami.Theme.positiveTextColor : (root.starting ? Kirigami.Theme.neutralTextColor : Kirigami.Theme.disabledTextColor)
    }
}
