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
        // indexOfValue() does not register the model as a binding dependency,
        // so reading count first is what makes this re-evaluate once the model
        // is populated.
        currentIndex: comboBox.count > 0 ? comboBox.indexOfValue(root.value) : -1

        onActivated: root.valueSelected(comboBox.currentValue)
    }
}
