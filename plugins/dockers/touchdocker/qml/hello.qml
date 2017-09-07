import QtQuick 2.3
import org.krita.sketch.components 1.0

Button {
    id: fileOpenButton
    color: "grey"
    width: 150; height: 75
    image: Settings.theme.icon("fileopen");

    onClicked: {
            mainWindow.slotButtonPressed("fileOpenButton")
    }


}
