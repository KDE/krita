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

import QtQuick 1.1
import "../components"

Panel {
    id: base;
    name: "Select";
    panelColor: "#000000";

    actions: [
        Button {
            id: deselectButton;
            width: height;
            height: Constants.ToolbarButtonSize
            color: "transparent";
            image: "../images/svg/icon-select-deselect.svg"
            textColor: "white";
            shadow: false;
            highlight: false;
            enabled: sketchView.selectionManager ? sketchView.selectionManager.havePixelsSelected : false;
            onClicked: if (sketchView.selectionManager) sketchView.selectionManager.deselect();
        },
        Button {
            id: reselectButton;
            width: height;
            height: Constants.ToolbarButtonSize
            color: "transparent";
            image: "../images/svg/icon-select-reselect.svg"
            textColor: "white";
            shadow: false;
            highlight: false;
            onClicked: sketchView.selectionManager.reselect();
        },
        Item {
            width: base.width - Constants.DefaultMargin - (Constants.ToolbarButtonSize * 3)
            height: Constants.ToolbarButtonSize;
        },
        Button {
            id: toggleShowSelectionButton;
            property bool showSelection: sketchView.selectionManager ? sketchView.selectionManager.displaySelection : false;
            width: height;
            height: Constants.ToolbarButtonSize
            color: "transparent";
            image: showSelection ? "../images/svg/icon-select-show.svg" : "../images/svg/icon-select-hide.svg";
            textColor: "white";
            shadow: false;
            highlight: false;
            onClicked: sketchView.selectionManager.toggleDisplaySelection();
        }
    ]

    peekContents: Item {
        id: peekItem;
        anchors.fill: parent;

        Item {
            width: childrenRect.width;
            height: peekItem.height / 3;
            anchors {
                horizontalCenter: parent.horizontalCenter;
                top: parent.top;
            }
            Button {
                id: selectRectangle;
                anchors.verticalCenter: parent.verticalCenter;
                image: "../images/svg/icon-select-rectangle.svg"
                width: Constants.ToolbarButtonSize * 0.8;
                height: width;
                color: "transparent";
                textColor: "white";
                shadow: false;
                highlight: false;
                onClicked: toolManager.requestToolChange("KisToolSelectRectangular");
                checked: toolManager.currentTool !== null ? toolManager.currentTool.toolId() === "KisToolSelectRectangular" : false;
            }
            Button {
                id: selectPolygon;
                anchors.verticalCenter: parent.verticalCenter;
                anchors.left: selectRectangle.right;
                image: "../images/svg/icon-select-polygon.svg"
                width: Constants.ToolbarButtonSize * 0.8;
                height: width;
                color: "transparent";
                textColor: "white";
                shadow: false;
                highlight: false;
                onClicked: toolManager.requestToolChange("KisToolSelectPolygonal");
                checked: toolManager.currentTool !== null ? toolManager.currentTool.toolId() === "KisToolSelectPolygonal" : false;
            }
            Button {
                id: selectArea;
                anchors.verticalCenter: parent.verticalCenter;
                anchors.left: selectPolygon.right;
                image: "../images/svg/icon-select-area.svg"
                width: Constants.ToolbarButtonSize * 0.8;
                height: width;
                color: "transparent";
                textColor: "white";
                shadow: false;
                highlight: false;
                onClicked: toolManager.requestToolChange("KisToolSelectContiguous");
                checked: toolManager.currentTool !== null ? toolManager.currentTool.toolId() === "KisToolSelectContiguous" : false;
            }
            Button {
                id: selectColor;
                anchors.verticalCenter: parent.verticalCenter;
                anchors.left: selectArea.right;
                image: "../images/svg/icon-select-color.svg"
                width: Constants.ToolbarButtonSize * 0.8;
                height: width;
                color: "transparent";
                textColor: "white";
                shadow: false;
                highlight: false;
                onClicked: toolManager.requestToolChange("KisToolSelectSimilar");
                checked: toolManager.currentTool !== null ? toolManager.currentTool.toolId() === "KisToolSelectSimilar" : false;
            }
        }
        Item {
            width: childrenRect.width;
            height: childrenRect.height;
            anchors.centerIn: parent;
            Button {
                id: selectReplace;
                image: "../images/svg/icon-select-replace.svg"
                width: Constants.GridWidth * 2 / 5;
                height: width;
                color: "transparent";
                shadow: false;
                highlight: false;
                checked: (toolManager.currentTool && toolManager.currentTool.selectionAction === 0) ? true : false;
                onClicked: if (toolManager.currentTool && toolManager.currentTool.selectionAction !== undefined) toolManager.currentTool.selectionAction = 0;
            }
            Button {
                id: selectIntersect;
                anchors.left: selectReplace.right;
                image: "../images/svg/icon-select-intersect.svg"
                width: Constants.ToolbarButtonSize * 0.8;
                height: width;
                color: "transparent";
                shadow: false;
                highlight: false;
                checked: (toolManager.currentTool && toolManager.currentTool.selectionAction === 3) ? true : false;
                onClicked: if (toolManager.currentTool && toolManager.currentTool.selectionAction !== undefined) toolManager.currentTool.selectionAction = 3;
            }
            Button {
                id: selectAdd;
                anchors.left: selectIntersect.right;
                image: "../images/svg/icon-select-add.svg"
                width: Constants.ToolbarButtonSize * 0.8;
                height: width;
                color: "transparent";
                shadow: false;
                highlight: false;
                checked: (toolManager.currentTool && toolManager.currentTool.selectionAction === 1) ? true : false;
                onClicked: if (toolManager.currentTool && toolManager.currentTool.selectionAction !== undefined) toolManager.currentTool.selectionAction = 1;
            }
            Button {
                id: selectSub;
                anchors.left: selectAdd.right;
                image: "../images/svg/icon-select-sub.svg"
                width: Constants.ToolbarButtonSize * 0.8;
                height: width;
                color: "transparent";
                shadow: false;
                highlight: false;
                checked: (toolManager.currentTool && toolManager.currentTool.selectionAction === 2) ? true : false;
                onClicked: if (toolManager.currentTool && toolManager.currentTool.selectionAction !== undefined) toolManager.currentTool.selectionAction = 2;
            }
        }
        Item {
            width: childrenRect.width;
            height: peekItem.height / 3;
            anchors {
                horizontalCenter: parent.horizontalCenter;
                bottom: parent.bottom;
            }
            Button {
                id: selectAll;
                anchors.verticalCenter: parent.verticalCenter;
                anchors.margins: Constants.DefaultMargin;
                text: "All";
                textColor: "black";
                color: "#63ffffff";
                border.width: 1;
                border.color: "silver";
                radius: Constants.DefaultMargin;
                width: Constants.IsLandscape ? Constants.GridWidth : (Constants.GridWidth * 2 / 3);// - Constants.DefaultMargin;
                height: textSize + Constants.DefaultMargin * 2;
                onClicked: sketchView.selectionManager.selectAll();
            }
            Button {
                id: selectInvert;
                anchors.verticalCenter: parent.verticalCenter;
                anchors.left: selectAll.right;
                anchors.margins: Constants.DefaultMargin;
                text: "Invert";
                textColor: "black";
                color: "#63ffffff";
                border.width: 1;
                border.color: "silver";
                radius: Constants.DefaultMargin;
                width: Constants.IsLandscape ? Constants.GridWidth : (Constants.GridWidth * 2 / 3);// - Constants.DefaultMargin;
                height: textSize + Constants.DefaultMargin * 2
                onClicked: sketchView.selectionManager.invert();
            }
            /*Button {
                id: selectOpaque;
                anchors.verticalCenter: parent.verticalCenter;
                anchors.left: selectInvert.right;
                anchors.margins: Constants.DefaultMargin;
                text: "Opaque";
                textColor: "black";
                color: "#63ffffff";
                border.width: 1;
                border.color: "silver";
                radius: Constants.DefaultMargin;
                width: (Constants.GridWidth * 2 / 3) - Constants.DefaultMargin;
                height: textSize + Constants.DefaultMargin * 2
            }*/
        }
    }

    fullContents: Item {
        anchors.fill: parent;
        Flickable {
            id: selectOptionsFullFlickable;
            anchors.fill: parent
            contentHeight: fullItem.height;
            Item {
                id: fullItem;
                width: parent.width;
                height: Constants.GridHeight * 9.5;

                Item {
                    id: fullTopRow;
                    width: childrenRect.width;
                    height: Constants.GridHeight;
                    anchors {
                        horizontalCenter: parent.horizontalCenter;
                        top: parent.top;
                    }
                    Button {
                        id: selectRectangleFull;
                        anchors.verticalCenter: parent.verticalCenter;
                        image: "../images/svg/icon-select-rectangle.svg"
                        width: Constants.ToolbarButtonSize * 0.8;
                        height: width;
                        color: "transparent";
                        textColor: "white";
                        shadow: false;
                        highlight: false;
                        onClicked: toolManager.requestToolChange("KisToolSelectRectangular");
                        checked: toolManager.currentTool !== null ? toolManager.currentTool.toolId() === "KisToolSelectRectangular" : false;
                    }
                    Button {
                        id: selectPolygonFull;
                        anchors.verticalCenter: parent.verticalCenter;
                        anchors.left: selectRectangleFull.right;
                        image: "../images/svg/icon-select-polygon.svg"
                        width: Constants.ToolbarButtonSize * 0.8;
                        height: width;
                        color: "transparent";
                        textColor: "white";
                        shadow: false;
                        highlight: false;
                        onClicked: toolManager.requestToolChange("KisToolSelectPolygonal");
                        checked: toolManager.currentTool !== null ? toolManager.currentTool.toolId() === "KisToolSelectPolygonal" : false;
                    }
                    Button {
                        id: selectAreaFull;
                        anchors.verticalCenter: parent.verticalCenter;
                        anchors.left: selectPolygonFull.right;
                        image: "../images/svg/icon-select-area.svg"
                        width: Constants.ToolbarButtonSize * 0.8;
                        height: width;
                        color: "transparent";
                        textColor: "white";
                        shadow: false;
                        highlight: false;
                        onClicked: toolManager.requestToolChange("KisToolSelectContiguous");
                        checked: toolManager.currentTool !== null ? toolManager.currentTool.toolId() === "KisToolSelectContiguous" : false;
                    }
                    Button {
                        id: selectColorFull;
                        anchors.verticalCenter: parent.verticalCenter;
                        anchors.left: selectAreaFull.right;
                        image: "../images/svg/icon-select-color.svg"
                        width: Constants.ToolbarButtonSize * 0.8;
                        height: width;
                        color: "transparent";
                        textColor: "white";
                        shadow: false;
                        highlight: false;
                        onClicked: toolManager.requestToolChange("KisToolSelectSimilar");
                        checked: toolManager.currentTool !== null ? toolManager.currentTool.toolId() === "KisToolSelectSimilar" : false;
                    }
                }
                Label {
                    id: modeLabel
                    anchors {
                        left: parent.left;
                        leftMargin: Constants.DefaultMargin;
                        top: fullTopRow.bottom;
                    }
                    horizontalAlignment: Text.AlignLeft;
                    font.pixelSize: Constants.LargeFontSize;
                    font.bold: true;
                    height: Constants.GridHeight / 2;
                    text: "Mode:";
                }
                Item {
                    id: fullModeRow;
                    width: childrenRect.width;
                    height: Constants.GridHeight;
                    anchors.horizontalCenter: parent.horizontalCenter;
                    anchors.top: modeLabel.bottom;
                    Button {
                        id: selectReplaceFull;
                        image: "../images/svg/icon-select-replace.svg"
                        width: Constants.ToolbarButtonSize * 0.8;
                        height: width;
                        color: "transparent";
                        shadow: false;
                        highlight: false;
                        checked: (toolManager.currentTool && toolManager.currentTool.selectionAction === 0) ? true : false;
                        onClicked: if (toolManager.currentTool && toolManager.currentTool.selectionAction !== undefined) toolManager.currentTool.selectionAction = 0;
                    }
                    Button {
                        id: selectIntersectFull;
                        anchors.left: selectReplaceFull.right;
                        image: "../images/svg/icon-select-intersect.svg"
                        width: Constants.ToolbarButtonSize * 0.8;
                        height: width;
                        color: "transparent";
                        shadow: false;
                        highlight: false;
                        checked: (toolManager.currentTool && toolManager.currentTool.selectionAction === 3) ? true : false;
                        onClicked: if (toolManager.currentTool && toolManager.currentTool.selectionAction !== undefined) toolManager.currentTool.selectionAction = 3;
                    }
                    Button {
                        id: selectAddFull;
                        anchors.left: selectIntersectFull.right;
                        image: "../images/svg/icon-select-add.svg"
                        width: Constants.ToolbarButtonSize * 0.8;
                        height: width;
                        color: "transparent";
                        shadow: false;
                        highlight: false;
                        checked: (toolManager.currentTool && toolManager.currentTool.selectionAction === 1) ? true : false;
                        onClicked: if (toolManager.currentTool && toolManager.currentTool.selectionAction !== undefined) toolManager.currentTool.selectionAction = 1;
                    }
                    Button {
                        id: selectSubFull;
                        anchors.left: selectAddFull.right;
                        image: "../images/svg/icon-select-sub.svg"
                        width: Constants.ToolbarButtonSize * 0.8;
                        height: width;
                        color: "transparent";
                        shadow: false;
                        highlight: false;
                        checked: (toolManager.currentTool && toolManager.currentTool.selectionAction === 2) ? true : false;
                        onClicked: if (toolManager.currentTool && toolManager.currentTool.selectionAction !== undefined) toolManager.currentTool.selectionAction = 2;
                    }
                }
                Label {
                    id: fullSelectCommandsLabel;
                    anchors {
                        left: parent.left;
                        leftMargin: Constants.DefaultMargin;
                        top: fullModeRow.bottom;
                    }
                    font.pixelSize: Constants.LargeFontSize;
                    font.bold: true;
                    height: Constants.GridHeight / 2;
                    text: "Select:";
                }
                Item {
                    id: fullSelectCommands;
                    width: childrenRect.width;
                    height: Constants.GridHeight;
                    anchors {
                        horizontalCenter: parent.horizontalCenter;
                        top: fullSelectCommandsLabel.bottom;
                    }
                    Button {
                        id: selectAllFull;
                        anchors.verticalCenter: parent.verticalCenter;
                        anchors.margins: Constants.DefaultMargin;
                        text: "All";
                        textColor: "black";
                        color: "#63ffffff";
                        border.width: 1;
                        border.color: "silver";
                        radius: Constants.DefaultMargin;
                        width: Constants.IsLandscape ? Constants.GridWidth : (Constants.GridWidth * 2 / 3);// - Constants.DefaultMargin;
                        height: textSize + Constants.DefaultMargin * 2
                        onClicked: sketchView.selectionManager.selectAll();
                    }
                    Button {
                        id: selectInvertFull;
                        anchors.verticalCenter: parent.verticalCenter;
                        anchors.left: selectAllFull.right;
                        anchors.margins: Constants.DefaultMargin;
                        text: "Invert";
                        textColor: "black";
                        color: "#63ffffff";
                        border.width: 1;
                        border.color: "silver";
                        radius: Constants.DefaultMargin;
                        width: Constants.IsLandscape ? Constants.GridWidth : (Constants.GridWidth * 2 / 3);// - Constants.DefaultMargin;
                        height: textSize + Constants.DefaultMargin * 2
                        onClicked: sketchView.selectionManager.invert();
                    }
                    /*Button {
                        id: selectOpaqueFull;
                        anchors.verticalCenter: parent.verticalCenter;
                        anchors.left: selectInvertFull.right;
                        anchors.margins: Constants.DefaultMargin;
                        text: "Opaque";
                        textColor: "black";
                        color: "#63ffffff";
                        border.width: 1;
                        border.color: "silver";
                        radius: Constants.DefaultMargin;
                        width: (Constants.GridWidth * 2 / 3) - Constants.DefaultMargin;
                        height: textSize + Constants.DefaultMargin * 2
                    }*/
                }
                Label {
                    id: fullEditingLabel;
                    anchors {
                        left: parent.left;
                        leftMargin: Constants.DefaultMargin;
                        top: fullSelectCommands.bottom;
                    }
                    font.pixelSize: Constants.LargeFontSize;
                    font.bold: true;
                    height: Constants.GridHeight / 2;
                    text: "Edit:";
                }
                Column {
                    anchors {
                        top: fullEditingLabel.bottom;
                        left: parent.left;
                        right: parent.right;
                    }
                    spacing: Constants.DefaultMargin;
                    Item {
                        width: fullItem.width;
                        height: featherTxt.height;
                        RangeInput {
                            id: featherTxt;
                            anchors {
                                left: parent.left;
                                right: featherImg.left;
                            }
                            placeholder: "Feather";
                            min: 1; max: 500; decimals: 0;
                            useExponentialValue: true;
                        }
                        Image {
                            id: featherImg;
                            anchors {
                                right: parent.right;
                                rightMargin: Constants.DefaultMargin;
                                verticalCenter: featherTxt.verticalCenter;
                            }
                            height: parent.height - (Constants.DefaultMargin * 6);
                            width: height;
                            source: "../images/svg/icon-select-apply.svg";
                            smooth: true;
                            MouseArea {
                                anchors.fill: parent;
                                onClicked: sketchView.selectionManager().feather(featherTxt.value);
                            }
                        }
                    }
                    Item {
                        width: fullItem.width;
                        height: growTxt.height;
                        RangeInput {
                            id: growTxt;
                            anchors {
                                left: parent.left;
                                right: growImg.left;
                            }
                            placeholder: "Grow";
                            min: 1; max: 500; decimals: 0;
                            useExponentialValue: true;
                        }
                        Image {
                            id: growImg;
                            anchors {
                                right: parent.right;
                                rightMargin: Constants.DefaultMargin;
                                verticalCenter: growTxt.verticalCenter;
                            }
                            height: parent.height - (Constants.DefaultMargin * 6);
                            width: height;
                            source: "../images/svg/icon-select-apply.svg";
                            smooth: true;
                            MouseArea {
                                anchors.fill: parent;
                                onClicked: sketchView.selectionExtras().grow(growTxt.value, growTxt.value);
                            }
                        }
                    }
                    Item {
                        width: fullItem.width;
                        height: borderTxt.height;
                        RangeInput {
                            id: borderTxt;
                            anchors {
                                left: parent.left;
                                right: borderImg.left;
                            }
                            placeholder: "Border";
                            min: 1; max: 500; decimals: 0;
                            useExponentialValue: true;
                        }
                        Image {
                            id: borderImg;
                            anchors {
                                right: parent.right;
                                rightMargin: Constants.DefaultMargin;
                                verticalCenter: borderTxt.verticalCenter;
                            }
                            height: parent.height - (Constants.DefaultMargin * 6);
                            width: height;
                            source: "../images/svg/icon-select-apply.svg";
                            smooth: true;
                            MouseArea {
                                anchors.fill: parent;
                                onClicked: sketchView.selectionManager().border(borderTxt.value, borderTxt.value);
                            }
                        }
                    }
                    Item {
                        width: fullItem.width;
                        height: shrinkTxt.height;
                        RangeInput {
                            id: shrinkTxt;
                            anchors {
                                left: parent.left;
                                right: shrinkImg.left;
                            }
                            placeholder: "Shrink";
                            min: 1; max: 500; decimals: 0;
                            useExponentialValue: true;
                        }
                        Image {
                            id: shrinkImg;
                            anchors {
                                right: parent.right;
                                rightMargin: Constants.DefaultMargin;
                                verticalCenter: shrinkTxt.verticalCenter;
                            }
                            height: parent.height - (Constants.DefaultMargin * 6);
                            width: height;
                            source: "../images/svg/icon-select-apply.svg";
                            smooth: true;
                            MouseArea {
                                anchors.fill: parent;
                                onClicked: sketchView.selectionManager().shrink(shrinkTxt.value, shrinkTxt.value, false);
                            }
                        }
                    }
                }
            }
        }
        ScrollDecorator { flickableItem: selectOptionsFullFlickable; }
    }
}
