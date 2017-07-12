import QtQuick 2.3

Rectangle {
    id: simpleButton
    color: "grey"
    width: 150; height: 75

    Text {
        id: buttonLabel
        anchors.centerIn: parent
        text: "button label"
    }
}
