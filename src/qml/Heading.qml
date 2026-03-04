import QtQuick.Controls 6.6
import QtQml 6.6

Label {
    id: mainLabel
    property int level: 0
    font.bold: level > 1

    font.pointSize: referenceLabel.font.pointSize + level
    property list<QtObject> children: [
        Label {
            id: referenceLabel
            visible: false
        }
    ]
}
