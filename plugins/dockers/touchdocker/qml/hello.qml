import QtQuick 2.3
import QtQuick.Layouts 1.2
import org.krita.sketch.components 1.0

Rectangle {

    anchors.fill: parent

    Button {
        id: fileOpenButton
        color: "grey"
        width: 75; height: 75
        image: Settings.theme.icon("fileopen");
        anchors.top: parent.top;
        anchors.left: parent.left;
        onClicked: {
            mainWindow.slotButtonPressed("fileOpenButton")
        }
    }

    Button {
        id: fileSaveButton
        color: "grey"
        width: 70; height: 75
        image: Settings.theme.icon("filesave");
        anchors.top: parent.top;
        anchors.left: fileOpenButton.right;
        onClicked: {
            mainWindow.slotButtonPressed("fileSaveButton")
        }
    }

    Button {
        id: fileSaveAsButton
        color: "grey"
        width: 75; height: 75
        image: Settings.theme.icon("filesaveas");
        anchors.top: parent.top;
        anchors.left: fileSaveButton.right;
        onClicked: {
            mainWindow.slotButtonPressed("fileSaveAsButton")
        }
    }
}
