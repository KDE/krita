import QtQuick 2.3

Rectangle {
    id: fileOpenButton
    color: "grey"
    width: 150; height: 75

    Text {
        id: buttonLabel
        anchors.centerIn: parent
        text: "Create or open an Image"
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            mainWindow.slotButtonPressed("fileOpenButton")
        }
    }


}
