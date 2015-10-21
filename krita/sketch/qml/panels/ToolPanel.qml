/* This file is part of the KDE project
 * Copyright (C) 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

import QtQuick 2.3
import org.krita.sketch 1.0
import org.krita.sketch.components 1.0

Panel {
    id: base;
    name: "Tool";
    colorSet: "tool";

    CompositeOpModel {
        id: compositeOpModel;
        view: sketchView.view;
    }

    ListModel {
        id: paintingTools;
        ListElement {
            text: "Paint"
            name: "paint"
        }
        ListElement {
            text: "Fill"
            name: "fill"
        }
        ListElement {
            text: "Gradient"
            name: "gradient"
        }
    }

    ListModel {
        id: adjustmentTools;
        ListElement {
            text: "Move"
            name: "move"
        }
//         ListElement {
//             text: "Transform"
//             name: "transform"
//         }
        ListElement {
            text: "Crop"
            name: "crop"
        }
    }

    function toolNameToID(toolName) {
        var names = {
            "paint": "KritaShape/KisToolBrush",
            "fill": "KritaFill/KisToolFill",
            "gradient": "KritaFill/KisToolGradient",
            "move": "KritaTransform/KisToolMove",
            "transform": "KisToolTransform",
            "crop": "KisToolCrop"
        };
        return names[toolName];
    }

    function changeTool(toolName) {
        if (toolName === "paint" || toolName === "fill" || toolName === "gradient") {
            eraserButton.visible = true;
        }
        else {
            eraserButton.visible = false;
        }
        if (toolName === "transform" || toolName === "crop") {
            topApplyButton.visible = true;
        }
        else {
            // Move tool doesn't have an apply function
            topApplyButton.visible = false;
        }
        toolManager.requestToolChange(toolNameToID(toolName));
        toolOptionsPeek.source = "toolconfigpages/" + toolName + ".qml";
        toolOptionsFull.source = "toolconfigpages/" + toolName + ".qml";
    }

    actions: [
        Button {
            id: firstTool;
            property string toolName: "paint";
            onToolNameChanged: changeTool(toolName);
            width: height;
            height: Constants.ToolbarButtonSize
            image: Settings.theme.icon(toolName);
            onClicked: {
                fullContentsItem.state = "";
                changeTool(toolName);
            }
        },
        Button {
            id: secondTool;
            property string toolName: "move";
            onToolNameChanged: changeTool(toolName);
            width: height;
            height: Constants.ToolbarButtonSize
            image: Settings.theme.icon(toolName);
            onClicked: {
                fullContentsItem.state = "secondTool";
                changeTool(toolName);
            }
        },
        Item {
            width: base.width - Constants.DefaultMargin - (Constants.ToolbarButtonSize * 3)
            height: Constants.ToolbarButtonSize;
        },
        Button {
            id: eraserButton;
            width: height;
            height: Constants.ToolbarButtonSize
            image: Settings.theme.icon("erase");
            checked: compositeOpModel.eraserMode;
            onClicked: compositeOpModel.eraserMode = !compositeOpModel.eraserMode;
        },
        Button {
            id: topApplyButton;
            width: height;
            height: Constants.ToolbarButtonSize;
            image: Settings.theme.icon("apply");
            visible: false;
            onClicked: state === "peek" ? toolOptionsPeek.item.apply() : toolOptionsFull.item.apply();
        }
    ]

    peekContents: Item {
        anchors.fill: parent;
        MouseArea {
            anchors.fill: parent;
            hoverEnabled: true;
            onContainsMouseChanged: toolOptionsPeek.focus = containsMouse;
        }
        Flickable {
            id: toolOptionsPeekFlickable;
            anchors.fill: parent
            contentHeight: toolOptionsPeek.height;
            Loader {
                id: toolOptionsPeek;
                width: parent.width;
                height: item.childrenRect.height;
                onSourceChanged: item.fullView = false;
                source: "toolconfigpages/paint.qml";
            }
        }
        ScrollDecorator { flickableItem: toolOptionsPeekFlickable; }
    }

    fullContents: Item {
        id: fullContentsItem;
        states: [
            State {
                name: "secondTool";
                PropertyChanges { target: firstToolSelector; visible: false; }
                PropertyChanges { target: secondToolSelector; visible: true; }
            }
        ]
        anchors.fill: parent;
        Item {
            id: toolSelectorContainer;
            anchors {
                top: parent.top;
                left: parent.left;
                right: parent.right;
            }
            height: childrenRect.height;
            z: 2;
            Item {
                id: firstToolSelector;
                width: parent.width;
                height: Constants.ToolbarButtonSize;
                Rectangle {
                    anchors.fill: parent;
                    color: Settings.theme.color("panels/tool/subheader");
                }
                Row {
                    anchors.horizontalCenter: parent.horizontalCenter;
                    width: childrenRect.width;
                    spacing: Constants.DefaultMargin;
                    Repeater {
                        model: paintingTools;
                        delegate: Button {
                            width: height;
                            height: Constants.ToolbarButtonSize
                            image: Settings.theme.icon(model.name + "-black");
                            checked: toolManager.currentTool !== null ? (toolManager.currentTool.toolId() === toolNameToID(model.name)) : false;
                            onClicked: {
                                firstTool.toolName = model.name;
                                changeTool(model.name);
                            }
                        }
                    }
                }
            }
            Item {
                id: secondToolSelector;
                width: parent.width;
                height: Constants.ToolbarButtonSize;
                visible: false;
                Rectangle {
                    anchors.fill: parent;
                    color: Settings.theme.color("panels/tool/subheader");
                }
                Row {
                    anchors.horizontalCenter: parent.horizontalCenter;
                    width: childrenRect.width;
                    spacing: Constants.DefaultMargin;
                    Repeater {
                        model: adjustmentTools;
                        delegate: Button {
                            width: height;
                            height: Constants.ToolbarButtonSize
                            image: Settings.theme.icon(model.name + "-black");
                            checked: toolManager.currentTool !== null ? (toolManager.currentTool.toolId() === toolNameToID(model.name)) : false;
                            onClicked: {
                                secondTool.toolName = model.name;
                                changeTool(model.name);
                            }
                        }
                    }
                }
            }
        }
        Flickable {
            id: toolOptionsFullFlickable;
            contentHeight: toolOptionsFull.height;
            boundsBehavior: Flickable.StopAtBounds;
            anchors {
                top: toolSelectorContainer.bottom;
                left: parent.left;
                leftMargin: Constants.DefaultMargin;
                right: parent.right;
                rightMargin: Constants.DefaultMargin * 2;
                bottom: parent.bottom;
            }

            MouseArea {
                anchors.fill: parent;
                hoverEnabled: true;
                onContainsMouseChanged: toolOptionsFull.focus = containsMouse;
            }

            Loader {
                id: toolOptionsFull;
                width: parent.width;
                height: item.height;
                source: "toolconfigpages/paint.qml";
            }
        }
        ScrollDecorator { anchors.topMargin: toolSelectorContainer.height; flickableItem: toolOptionsFullFlickable; }
    }
}
