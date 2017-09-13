import QtQuick 2.3
import QtQuick.Layouts 1.3
import org.krita.sketch 1.0
import org.krita.sketch.components 1.0

Rectangle {
    id: base
    color: Settings.theme.color("pages/open/background")
    anchors.fill: parent


    ColumnLayout {

        GridLayout {

            id: toolbar
            columns: 3

            Button {
                id: fileOpenButton
                color: "grey"
                width: 75; height: 75
                radius: 3
                image: Settings.theme.icon("fileopen");
                onClicked: {
                    mainWindow.slotButtonPressed("fileOpenButton")
                }
            }

            Button {
                id: fileSaveButton
                color: "grey"
                width: 75; height: 75
                radius: 3
                image: Settings.theme.icon("filesave");
                onClicked: {
                    mainWindow.slotButtonPressed("fileSaveButton")
                }
            }

            Button {
                id: fileSaveAsButton
                color: "grey"
                width: 75; height: 75
                radius: 3
                image: Settings.theme.icon("filesaveas");
                onClicked: {
                    mainWindow.slotButtonPressed("fileSaveAsButton")
                }
            }

            Button {
                id: undoButton
                color: "grey"
                width: 75; height: 75
                radius: 3
                image: Settings.theme.icon("undo");
                onClicked: {
                    mainWindow.slotButtonPressed("edit_undo")
                }
            }

            Button {
                id: redoButton
                color: "grey"
                width: 75; height: 75
                radius: 3
                image: Settings.theme.icon("redo");
                onClicked: {
                    mainWindow.slotButtonPressed("edit_redo")
                }
            }

            Button {
                id: canvasOnlyButton
                color: "grey"
                width: 75; height: 75
                radius: 3
                image: Settings.theme.icon("redo");
                onClicked: {
                    mainWindow.slotButtonPressed("edit_redo")
                }
            }

        }

        Button {
            id: button1
            color: "grey"
            width: 75; height: 75
            radius: 3
            image: Settings.theme.icon("redo");
            onClicked: {
                mainWindow.slotButtonPressed("button1")
            }
        }

        Button {
            id: button2
            color: "grey"
            width: 75; height: 75
            radius: 3
            image: Settings.theme.icon("redo");
            onClicked: {
                mainWindow.slotButtonPressed("button2")
            }
        }

        Button {
            id: button3
            color: "grey"
            width: 75; height: 75
            radius: 3
            image: Settings.theme.icon("redo");
            onClicked: {
                mainWindow.slotButtonPressed("button3")
            }
        }

        Button {
            id: button4
            color: "grey"
            width: 75; height: 75
            radius: 3
            image: Settings.theme.icon("redo");
            onClicked: {
                mainWindow.slotButtonPressed("button4")
            }
        }

        Rectangle {
            id: rockerSwitch
            width: parent.width
            height: 300
            color: red

        }

        Button {
            id: button5
            color: "grey"
            width: 75; height: 75
            radius: 3
            image: Settings.theme.icon("redo");
            onClicked: {
                mainWindow.slotButtonPressed("button5")
            }
        }

        Button {
            id: button6
            color: "grey"
            width: 75; height: 75
            radius: 3
            image: Settings.theme.icon("redo");
            onClicked: {
                mainWindow.slotButtonPressed("button6")
            }
        }

        Button {
            id: button7
            color: "grey"
            width: 75; height: 75
            radius: 3
            image: Settings.theme.icon("redo");
            onClicked: {
                mainWindow.slotButtonPressed("button7")
            }
        }

        Button {
            id: button8
            color: "grey"
            width: 75; height: 75
            radius: 3
            image: Settings.theme.icon("redo");
            onClicked: {
                mainWindow.slotButtonPressed("button8")
            }
        }

    }


}
