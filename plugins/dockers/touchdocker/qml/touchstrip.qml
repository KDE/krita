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
            columns: 2

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

        }
    }
}
