import QtQuick
import QtQuick.Controls
import org.kde.kirigami as Kirigami

/**
 * Cell letting the user pick one value of an enum backed model, such as
 * AuthMethodModel or RegionModel.
 */
CellDelegate {
    id: root

    /**
     * A model exposing "name" and "value" roles.
     */
    property alias options: comboBox.model

    /**
     * The enum value currently stored on the profile.
     */
    property int value: -1

    signal valueSelected(int newValue)

    ComboBox {
        id: comboBox

        Kirigami.Theme.colorSet: Kirigami.Theme.Button
        Kirigami.Theme.inherit: false

        anchors.fill: parent
        anchors.topMargin: root.verticalPadding
        anchors.bottomMargin: root.verticalPadding
        anchors.leftMargin: Kirigami.Units.smallSpacing
        anchors.rightMargin: Kirigami.Units.smallSpacing

        textRole: "name"
        valueRole: "value"
        currentIndex: comboBox.indexOfValue(root.value)

        onActivated: root.valueSelected(comboBox.currentValue)
    }
}
