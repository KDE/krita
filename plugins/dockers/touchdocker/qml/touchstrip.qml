import QtQuick 2.3
import org.krita.sketch 1.0
import org.krita.sketch.components 1.0

Rectangle {
    id: base
    property var palette: SystemPalette {colorGroup: SystemPalette.active }

    color: palette.dark
    
    Button {
        id: fileOpenButton
        color: palette.button
        textColor: palette.buttonText
        radius: 8
        width: base.width / 3
        height: base.height / 13
        anchors.left: base.left
        anchors.top: base.top
        image: Settings.theme.icon("fileopen");
        onClicked: {
            mainWindow.slotButtonPressed("fileOpenButton")
        }
    }

    Button {
        id: fileSaveButton
        color: palette.button
        textColor: palette.buttonText
        radius: 8
        width: base.width / 3
        height: base.height / 13
        anchors.top: base.top
        anchors.left: fileOpenButton.right
        image: Settings.theme.icon("filesave");
        onClicked: {
            mainWindow.slotButtonPressed("fileSaveButton")
        }
    }

    Button {
        id: fileSaveAsButton
        color: palette.button
        textColor: palette.buttonText
        radius: 8
        width: base.width / 3
        height: base.height / 13
        anchors.top: base.top
        anchors.left: fileSaveButton.right
        image: Settings.theme.icon("filesaveas");
        onClicked: {
            mainWindow.slotButtonPressed("fileSaveAsButton")
        }
    }
    
    Button {
        id: undoButton
        color: palette.button
        textColor: palette.buttonText
        radius: 8
        width: base.width / 2
        height: base.height / 13
        anchors.left: base.left
        anchors.top: fileOpenButton.bottom
        image: Settings.theme.icon("undo");
        onClicked: {
            mainWindow.slotButtonPressed("edit_undo")
        }
    }

    Button {
        id: redoButton
        color: palette.button
        textColor: palette.buttonText
        radius: 8
        width: base.width / 2
        height: base.height / 13
        anchors.left: undoButton.right
        anchors.top: undoButton.top
        image: Settings.theme.icon("redo");
        onClicked: {
            mainWindow.slotButtonPressed("edit_redo")
        }
    }
    
    Button {
        id: button1
        color: palette.button
        textColor: palette.buttonText
        width: base.width
        height: base.height / 13
        anchors.left: base.left
        anchors.top: undoButton.bottom
        radius: 3
        text: mainWindow.imageForButton("button1") === "" ? mainWindow.textForButton("button1") : ""
        checkable: text === "shift" || text == "ctrl" || text == "alt" ? true : false;
        image: mainWindow.imageForButton("button1")
        onClicked: {
            mainWindow.slotButtonPressed("button1")
        }
    }

    Button {
        id: button2
        color: palette.button
        textColor: palette.buttonText
        width: base.width
        height: base.height / 13
        anchors.left: base.left
        anchors.top: button1.bottom
        radius: 3
        text: mainWindow.imageForButton("button2") === "" ? mainWindow.textForButton("button2") : ""
        checkable: text === "shift" || text == "ctrl" || text == "alt" ? true : false;
        image: mainWindow.imageForButton("button2")
        onClicked: {
            mainWindow.slotButtonPressed("button2")
        }
    }

    Button {
        id: button3
        color: palette.button
        textColor: palette.buttonText
        width: base.width
        height: base.height / 13
        anchors.left: base.left
        anchors.top: button2.bottom
        radius: 3
        text: mainWindow.imageForButton("button3") === "" ? mainWindow.textForButton("button3") : ""
        checkable: text === "shift" || text == "ctrl" || text == "alt" ? true : false;
        image: mainWindow.imageForButton("button3")
        onClicked: {
            mainWindow.slotButtonPressed("button3")
        }
    }

    Button {
        id: button4
        color: palette.button
        textColor: palette.buttonText
        width: base.width
        height: base.height / 13
        anchors.left: base.left
        anchors.top: button3.bottom
        radius: 3
        text: mainWindow.imageForButton("button4") === "" ? mainWindow.textForButton("button4") : ""
        checkable: text === "shift" || text == "ctrl" || text == "alt" ? true : false;
        image: mainWindow.imageForButton("button4")
        onClicked: {
            mainWindow.slotButtonPressed("button4")
        }
    }

    Button {
        id: rockerSwitchTop
        color: palette.button
        textColor: palette.buttonText
        radius: 8
        width: base.width / 3
        height: base.height / 13
        anchors.left: fileSaveButton.left
        anchors.top: button4.bottom

        anchors.right: rockerSwitchCenter.Left
        image: mainWindow.imageForButton("view_zoom_in")
        onClicked: {
            mainWindow.slotButtonPressed("view_zoom_in")
        }
    }


    Button {
        id: rockerSwitchLeft
        color: palette.button
        textColor: palette.buttonText
        radius: 8
        width: base.width / 3
        height: base.height / 13
        anchors.left: base.left
        anchors.top: rockerSwitchTop.bottom

        image: mainWindow.imageForButton("rotate_canvas_left")
        onClicked: {
            mainWindow.slotButtonPressed("rotate_canvas_left")
        }
    }

    Button {
        id: rockerSwitchCenter
        color: palette.button
        textColor: palette.buttonText
        radius: 8
        width: base.width / 3
        height: base.height / 13
        anchors.left: rockerSwitchLeft.right
        anchors.top: rockerSwitchTop.bottom

        image: mainWindow.imageForButton("reset_canvas_rotation")
        onClicked: {
            mainWindow.slotButtonPressed("reset_canvas_rotation")
            mainWindow.slotButtonPressed("zoom_to_100pct")
        }
    }

    Button {
        id: rockerSwitchRight
        color: palette.button
        textColor: palette.buttonText
        radius: 8
        width: base.width / 3
        height: base.height / 13
        anchors.left: rockerSwitchCenter.right
        anchors.top: rockerSwitchTop.bottom

        image: mainWindow.imageForButton("rotate_canvas_right")
        onClicked: {
            mainWindow.slotButtonPressed("rotate_canvas_right")
        }
    }

    Button {
        id: rockerSwitchBottom
        color: palette.button
        textColor: palette.buttonText
        radius: 8
        width: base.width / 3
        height: base.height / 13
        anchors.left: rockerSwitchCenter.left
        anchors.top: rockerSwitchCenter.bottom

        anchors.right: rockerSwitchCenter.Left
        image: mainWindow.imageForButton("view_zoom_out")
        onClicked: {
            mainWindow.slotButtonPressed("view_zoom_out")
        }
    }

    Button {
        id: button5
        color: palette.button
        textColor: palette.buttonText
        width: base.width
        height: base.height / 13
        anchors.left: base.left
        anchors.top: rockerSwitchBottom.bottom
        radius: 3
        text: mainWindow.imageForButton("button5") === "" ? mainWindow.textForButton("button5") : ""
        checkable: text === "shift" || text == "ctrl" || text == "alt" ? true : false;
        image: mainWindow.imageForButton("button5")
        onClicked: {
            mainWindow.slotButtonPressed("button5")
        }
    }

    Button {
        id: button6
        color: palette.button
        textColor: palette.buttonText
        width: base.width
        height: base.height / 13
        anchors.left: base.left
        anchors.top: button5.bottom
        radius: 3
        text: mainWindow.imageForButton("button6") === "" ? mainWindow.textForButton("button6") : ""
        checkable: text === "shift" || text == "ctrl" || text == "alt" ? true : false;
        image: mainWindow.imageForButton("button6")
        onClicked: {
            mainWindow.slotButtonPressed("button6")
        }
    }

    Button {
        id: button7
        color: palette.button
        textColor: palette.buttonText
        width: base.width
        height: base.height / 13
        anchors.left: base.left
        anchors.top: button6.bottom
        radius: 3
        text: mainWindow.imageForButton("button7") === "" ? mainWindow.textForButton("button7") : ""
        checkable: text === "shift" || text == "ctrl" || text == "alt" ? true : false;
        image: mainWindow.imageForButton("button7")
        onClicked: {
            mainWindow.slotButtonPressed("button7")
        }
    }

    Button {
        id: button8
        color: palette.button
        textColor: palette.buttonText
        width: base.width
        height: base.height / 13
        anchors.left: base.left
        anchors.top: button7.bottom
        radius: 3
        text: mainWindow.imageForButton("button8") === "" ? mainWindow.textForButton("button8") : ""
        checkable: text === "shift" || text == "ctrl" || text == "alt" ? true : false;
        image: mainWindow.imageForButton("button8")
        onClicked: {
            mainWindow.slotButtonPressed("button8")
        }
    }


}
