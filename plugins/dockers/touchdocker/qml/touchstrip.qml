/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick 2.3
import org.krita.sketch 1.0
import org.krita.sketch.components 1.0

Rectangle {
    id: root

    SystemPalette {
        id: palette;
        colorGroup: SystemPalette.Active
    }

    property int rowHeight: height/9;
    color: palette.base;

    Column {
        width: root.width
        Row {
            height: root.rowHeight;
            Repeater {
                model: ["fileOpen", "fileSave", "fileSaveAs"]
                Button {
                    color: palette.button
                    highlightColor: palette.highlight
                    textColor: palette.buttonText
                    radius: 8;
                    width: root.width / 3
                    height: parent.height;
                    onClicked: {
                        mainWindow.slotButtonPressed(modelData+"Button")
                    }
                    image: Settings.theme.icon(modelData.toLowerCase());
                }
            }
        }

        Row {
            width: parent.width;
            height: root.rowHeight;
            Button {
                color: palette.button
                highlightColor: palette.highlight
                textColor: palette.buttonText
                radius: 8;
                id: undoButton
                width: root.width / 2
                height: parent.height;
                image: Settings.theme.icon("undo");
                onClicked: {
                    mainWindow.slotButtonPressed("edit_undo")
                }
            }

            Button {
                color: palette.button
                highlightColor: palette.highlight
                textColor: palette.buttonText
                radius: 8;
                width: root.width / 2
                height: parent.height;
                image: Settings.theme.icon("redo");
                onClicked: {
                    mainWindow.slotButtonPressed("edit_redo")
                }
            }
        }

        Row{
            width: parent.width;
            height: childrenRect.height;
            Repeater {
                model: [2, 3]
                Button {
                    color: palette.button
                    highlightColor: palette.highlight
                    textColor: palette.buttonText
                    radius: 8;
                    width: parent.width/2;
                    height: root.rowHeight
                    text: mainWindow.imageForButton("button" + modelData) === "" ? mainWindow.textForButton("button" + modelData) : ""
                    checkable: text === "shift" || text == "ctrl" || text == "alt" ? true : false;
                    onClicked: {
                        mainWindow.slotButtonPressed("button" + modelData)
                    }
                    image: root.width != 0 ? mainWindow.imageForButton("button" + modelData) : "";
                }
            }
        }

        Row{
            width: parent.width;
            height: childrenRect.height;
            Repeater {
                model: [1, 4]
                Button {
                    color: palette.button
                    highlightColor: palette.highlight
                    textColor: palette.buttonText
                    radius: 8;
                    width: parent.width/2;
                    height: root.rowHeight
                    text: mainWindow.imageForButton("button" + modelData) === "" ? mainWindow.textForButton("button" + modelData) : ""
                    checkable: text === "shift" || text == "ctrl" || text == "alt" ? true : false;
                    onClicked: {
                        mainWindow.slotButtonPressed("button" + modelData)
                    }
                    image: root.width != 0 ? mainWindow.imageForButton("button" + modelData) : "";
                }
            }
        }

        Row {
            width: parent.width;
            height: childrenRect.height;
            Repeater {
                model: ["rotate_canvas_left", "reset_canvas_rotation", "rotate_canvas_right"]
                Item {
                    height: root.rowHeight
                    width: root.width/3
                    Button {
                        id: rockerSwitch
                        color: palette.button
                        highlightColor: palette.highlight
                        textColor: palette.buttonText
                        radius: 8;
                        anchors.fill: parent;
                        visible: modelData !== "";
                        image: root.width != 0 ? mainWindow.imageForButton(modelData) : "";
                        onClicked: {
                            mainWindow.slotButtonPressed(modelData)
                            if (modelData === "reset_canvas_rotation") {
                                mainWindow.slotButtonPressed("zoom_to_100pct")
                            }
                        }
                    }
                }
            }
        }

        Row{
            width: parent.width;
            height: childrenRect.height;
            Repeater {
                model: [6, "view_zoom_in"]
                Button {
                    color: palette.button
                    highlightColor: palette.highlight
                    textColor: palette.buttonText
                    radius: 8;
                    width: parent.width/2;
                    height: root.rowHeight
                    text: (modelData !== "view_zoom_in" && mainWindow.imageForButton("button" + modelData) === "") ? mainWindow.textForButton("button" + modelData) : ""
                    checkable: text === "shift" || text == "ctrl" || text == "alt" ? true : false;
                    onClicked: {
                        if (modelData === "view_zoom_in")
                        {
                            mainWindow.slotButtonPressed(modelData)
                        }
                        else
                        {
                            mainWindow.slotButtonPressed("button" + modelData)
                        }
                    }
                    image: root.width != 0 ? mainWindow.imageForButton("button" + modelData) : ""
                }
            }
        }

        Row{
            width: parent.width;
            height: childrenRect.height;
            Repeater {
                model: [5, "view_zoom_out"]
                Button {
                    color: palette.button
                    highlightColor: palette.highlight
                    textColor: palette.buttonText
                    radius: 8;
                    width: parent.width/2;
                    height: root.rowHeight
                    text: (modelData !== "view_zoom_out" && mainWindow.imageForButton("button" + modelData) === "") ? mainWindow.textForButton("button" + modelData) : ""
                    checkable: text === "shift" || text == "ctrl" || text == "alt" ? true : false;
                    onClicked: {
                        if(modelData === "view_zoom_out")
                        {
                            mainWindow.slotButtonPressed(modelData)
                        }
                        else
                        {
                            mainWindow.slotButtonPressed("button" + modelData)
                        }
                    }
                    image: root.width != 0 ? mainWindow.imageForButton("button" + modelData) : "";
                }
            }
        }

        Repeater {
            width: parent.width;
            height: childrenRect.height;
            model: [7, 8]
            Button {
                color: palette.button
                highlightColor: palette.highlight
                textColor: palette.buttonText
                radius: 8;
                width: parent.width;
                height: root.rowHeight
                text: mainWindow.imageForButton("button" + modelData) === "" ? mainWindow.textForButton("button" + modelData) : ""
                checkable: text === "shift" || text == "ctrl" || text == "alt" ? true : false;
                onClicked: {
                    mainWindow.slotButtonPressed("button" + modelData)
                }
                image: root.width != 0 ? mainWindow.imageForButton("button" + modelData) : ""
            }
        }
    }
}
