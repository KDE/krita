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
    // same rule as defined in kis_icon_utils to determine useDarkIcons, but in the 0-1 range
    property bool useDarkIcons: palette.button.hsvValue > 0.39 ? true : false;
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
                    image: root.useDarkIcons ? Settings.theme.icon(modelData.toLowerCase() + "-black") :
                               Settings.theme.icon(modelData.toLowerCase());
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
                image: root.useDarkIcons ? Settings.theme.icon("undo-black") :
                               Settings.theme.icon("undo");
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
                image: root.useDarkIcons ? Settings.theme.icon("redo-black") :
                               Settings.theme.icon("redo");
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
                ButtonSquared {
                    color: palette.button
                    highlightColor: palette.highlight
                    textColor: palette.buttonText
                    radius: 8;
                    width: parent.width/2;
                    height: root.rowHeight
                    checkable: text === "shift" || text == "ctrl" || text == "alt" ? true : false;
                    onClicked: {
                        mainWindow.slotButtonPressed(modelData)
                    }
                    image: root.useDarkIcons ? mainWindow.iconForButton(modelData, true) : mainWindow.iconForButton(modelData, false);
                }
            }
        }

        Row{
            width: parent.width;
            height: childrenRect.height;
            Repeater {
                model: [1, 4]
                ButtonSquared {
                    color: palette.button
                    highlightColor: palette.highlight
                    textColor: palette.buttonText
                    radius: 8;
                    width: parent.width/2;
                    height: root.rowHeight
                    checkable: text === "shift" || text == "ctrl" || text == "alt" ? true : false;
                    onClicked: {
                        mainWindow.slotButtonPressed(modelData)
                    }
                    image: root.useDarkIcons ? mainWindow.iconForButton(modelData, true) : mainWindow.iconForButton(modelData, false);
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
                    ButtonSquared {
                        id: rockerSwitch
                        color: palette.button
                        highlightColor: palette.highlight
                        textColor: palette.buttonText
                        radius: 8;
                        anchors.fill: parent;
                        visible: modelData !== "";
                        image: root.useDarkIcons ? mainWindow.iconForButton(modelData, true) : mainWindow.iconForButton(modelData, false);
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
                ButtonSquared {
                    color: palette.button
                    highlightColor: palette.highlight
                    textColor: palette.buttonText
                    radius: 8;
                    width: parent.width/2;
                    height: root.rowHeight
                    checkable: text === "shift" || text == "ctrl" || text == "alt" ? true : false;
                    onClicked: {
                        if (modelData === "view_zoom_in")
                        {
                            mainWindow.slotButtonPressed(modelData)
                        }
                        else
                        {
                            mainWindow.slotButtonPressed(modelData)
                        }
                    }
                    image: root.useDarkIcons ? mainWindow.iconForButton(modelData, true) : mainWindow.iconForButton(modelData, false);
                }
            }
        }

        Row{
            width: parent.width;
            height: childrenRect.height;
            Repeater {
                model: [5, "view_zoom_out"]
                ButtonSquared {
                    color: palette.button
                    highlightColor: palette.highlight
                    textColor: palette.buttonText
                    radius: 8;
                    width: parent.width/2;
                    height: root.rowHeight
                    checkable: text === "shift" || text == "ctrl" || text == "alt" ? true : false;
                    onClicked: {
                        if(modelData === "view_zoom_out")
                        {
                            mainWindow.slotButtonPressed(modelData)
                        }
                        else
                        {
                            mainWindow.slotButtonPressed(modelData)
                        }
                    }
                    image: root.useDarkIcons ? mainWindow.iconForButton(modelData, true) : mainWindow.iconForButton(modelData, false);
                }
            }
        }

        Repeater {
            width: parent.width;
            height: childrenRect.height;
            model: [7, 8]
            ButtonSquared {
                color: palette.button
                highlightColor: palette.highlight
                textColor: palette.buttonText
                radius: 8;
                width: parent.width;
                height: root.rowHeight
                checkable: text === "shift" || text == "ctrl" || text == "alt" ? true : false;
                onClicked: {
                    mainWindow.slotButtonPressed(modelData)
                }
                image: root.useDarkIcons ? mainWindow.iconForButton(modelData, true) : mainWindow.iconForButton(modelData, false);
            }
        }
    }
}
