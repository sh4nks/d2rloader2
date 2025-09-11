import QtQuick 6.6

QtObject {
    id: units

    property double gridUnit: fontMetrics.boundingRect.height
    property list<QtObject> children: [
        TextMetrics {
            id: fontMetrics
            text: "M"
        }
    ]
}
