/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.3
import org.krita.sketch 1.0
import org.krita.sketch.components 1.0

Panel {
    id: base;
    name: "Select";
    colorSet: "selection";

    actions: [
        Button {
            id: deselectButton;
            width: height;
            height: Constants.ToolbarButtonSize
            image: Settings.theme.icon("select-deselect");
            enabled: sketchView.selectionManager ? sketchView.selectionManager.havePixelsSelected : false;
            onClicked: if (sketchView.selectionManager) sketchView.selectionManager.deselect();
        },
        Button {
            id: reselectButton;
            width: height;
            height: Constants.ToolbarButtonSize
            image: Settings.theme.icon("select-reselect");
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
            image: showSelection ? Settings.theme.icon("select-show") : Settings.theme.icon("select-hide");
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
                image: Settings.theme.icon("select-rectangle");
                width: Constants.ToolbarButtonSize * 0.8;
                height: width;
                onClicked: toolManager.requestToolChange("KisToolSelectRectangular");
                checked: toolManager.currentTool !== null ? toolManager.currentTool.toolId() === "KisToolSelectRectangular" : false;
            }
            Button {
                id: selectPolygon;
                anchors.verticalCenter: parent.verticalCenter;
                anchors.left: selectRectangle.right;
                image: Settings.theme.icon("select-polygon");
                width: Constants.ToolbarButtonSize * 0.8;
                height: width;
                onClicked: toolManager.requestToolChange("KisToolSelectPolygonal");
                checked: toolManager.currentTool !== null ? toolManager.currentTool.toolId() === "KisToolSelectPolygonal" : false;
            }
            Button {
                id: selectArea;
                anchors.verticalCenter: parent.verticalCenter;
                anchors.left: selectPolygon.right;
                image: Settings.theme.icon("select-area");
                width: Constants.ToolbarButtonSize * 0.8;
                height: width;
                onClicked: toolManager.requestToolChange("KisToolSelectContiguous");
                checked: toolManager.currentTool !== null ? toolManager.currentTool.toolId() === "KisToolSelectContiguous" : false;
            }
            Button {
                id: selectColor;
                anchors.verticalCenter: parent.verticalCenter;
                anchors.left: selectArea.right;
                image: Settings.theme.icon("select-color");
                width: Constants.ToolbarButtonSize * 0.8;
                height: width;
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
                image: Settings.theme.icon("select-replace");
                width: Constants.ToolbarButtonSize * 0.8;
                height: width;
                checked: (toolManager.currentTool && toolManager.currentTool.selectionAction === 0) ? true : false;
                onClicked: if (toolManager.currentTool && toolManager.currentTool.selectionAction !== undefined) toolManager.currentTool.selectionAction = 0;
            }
            Button {
                id: selectIntersect;
                anchors.left: selectReplace.right;
                image: Settings.theme.icon("select-intersect");
                width: Constants.ToolbarButtonSize * 0.8;
                height: width;
                checked: (toolManager.currentTool && toolManager.currentTool.selectionAction === 3) ? true : false;
                onClicked: if (toolManager.currentTool && toolManager.currentTool.selectionAction !== undefined) toolManager.currentTool.selectionAction = 3;
            }
            Button {
                id: selectAdd;
                anchors.left: selectIntersect.right;
                image: Settings.theme.icon("select-add");
                width: Constants.ToolbarButtonSize * 0.8;
                height: width;
                checked: (toolManager.currentTool && toolManager.currentTool.selectionAction === 1) ? true : false;
                onClicked: if (toolManager.currentTool && toolManager.currentTool.selectionAction !== undefined) toolManager.currentTool.selectionAction = 1;
            }
            Button {
                id: selectSub;
                anchors.left: selectAdd.right;
                image: Settings.theme.icon("select-sub");
                width: Constants.ToolbarButtonSize * 0.8;
                height: width;
                checked: (toolManager.currentTool && toolManager.currentTool.selectionAction === 2) ? true : false;
                onClicked: if (toolManager.currentTool && toolManager.currentTool.selectionAction !== undefined) toolManager.currentTool.selectionAction = 2;
            }
            Button {
                id: selectSymmetricDifference;
                anchors.left: selectSub.right;
                image: Settings.theme.icon("select-symmetric-difference");
                width: Constants.ToolbarButtonSize * 0.8;
                height: width;
                checked: (toolManager.currentTool && toolManager.currentTool.selectionAction === 4) ? true : false;
                onClicked: if (toolManager.currentTool && toolManager.currentTool.selectionAction !== undefined) toolManager.currentTool.selectionAction = 4;
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
                textColor: Settings.theme.color("panels/selection/buttons/text");
                color: Settings.theme.color("panels/selection/buttons/color");
                border.width: 1;
                border.color: Settings.theme.color("panels/selection/buttons/border");
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
                textColor: Settings.theme.color("panels/selection/buttons/text");
                color: Settings.theme.color("panels/selection/buttons/color");
                border.width: 1;
                border.color: Settings.theme.color("panels/selection/buttons/border");
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
                        image: Settings.theme.icon("select-rectangle");
                        width: Constants.ToolbarButtonSize * 0.8;
                        height: width;
                        onClicked: toolManager.requestToolChange("KisToolSelectRectangular");
                        checked: toolManager.currentTool !== null ? toolManager.currentTool.toolId() === "KisToolSelectRectangular" : false;
                    }
                    Button {
                        id: selectPolygonFull;
                        anchors.verticalCenter: parent.verticalCenter;
                        anchors.left: selectRectangleFull.right;
                        image: Settings.theme.icon("select-polygon")
                        width: Constants.ToolbarButtonSize * 0.8;
                        height: width;
                        onClicked: toolManager.requestToolChange("KisToolSelectPolygonal");
                        checked: toolManager.currentTool !== null ? toolManager.currentTool.toolId() === "KisToolSelectPolygonal" : false;
                    }
                    Button {
                        id: selectAreaFull;
                        anchors.verticalCenter: parent.verticalCenter;
                        anchors.left: selectPolygonFull.right;
                        image: Settings.theme.icon("select-area")
                        width: Constants.ToolbarButtonSize * 0.8;
                        height: width;
                        onClicked: toolManager.requestToolChange("KisToolSelectContiguous");
                        checked: toolManager.currentTool !== null ? toolManager.currentTool.toolId() === "KisToolSelectContiguous" : false;
                    }
                    Button {
                        id: selectColorFull;
                        anchors.verticalCenter: parent.verticalCenter;
                        anchors.left: selectAreaFull.right;
                        image: Settings.theme.icon("select-color")
                        width: Constants.ToolbarButtonSize * 0.8;
                        height: width;
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
                    height: Constants.GridHeight / 2;
                    text: "Mode:";
                    font: Settings.theme.font("panelSection");
                }
                Item {
                    id: fullModeRow;
                    width: childrenRect.width;
                    height: Constants.GridHeight;
                    anchors.horizontalCenter: parent.horizontalCenter;
                    anchors.top: modeLabel.bottom;
                    Button {
                        id: selectReplaceFull;
                        image: Settings.theme.icon("select-replace")
                        width: Constants.ToolbarButtonSize * 0.8;
                        height: width;
                        checked: (toolManager.currentTool && toolManager.currentTool.selectionAction === 0) ? true : false;
                        onClicked: if (toolManager.currentTool && toolManager.currentTool.selectionAction !== undefined) toolManager.currentTool.selectionAction = 0;
                    }
                    Button {
                        id: selectIntersectFull;
                        anchors.left: selectReplaceFull.right;
                        image: Settings.theme.icon("select-intersect")
                        width: Constants.ToolbarButtonSize * 0.8;
                        height: width;
                        checked: (toolManager.currentTool && toolManager.currentTool.selectionAction === 3) ? true : false;
                        onClicked: if (toolManager.currentTool && toolManager.currentTool.selectionAction !== undefined) toolManager.currentTool.selectionAction = 3;
                    }
                    Button {
                        id: selectAddFull;
                        anchors.left: selectIntersectFull.right;
                        image: Settings.theme.icon("select-add")
                        width: Constants.ToolbarButtonSize * 0.8;
                        height: width;
                        checked: (toolManager.currentTool && toolManager.currentTool.selectionAction === 1) ? true : false;
                        onClicked: if (toolManager.currentTool && toolManager.currentTool.selectionAction !== undefined) toolManager.currentTool.selectionAction = 1;
                    }
                    Button {
                        id: selectSubFull;
                        anchors.left: selectAddFull.right;
                        image: Settings.theme.icon("select-sub")
                        width: Constants.ToolbarButtonSize * 0.8;
                        height: width;
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
                    height: Constants.GridHeight / 2;
                    text: "Select:";
                    font: Settings.theme.font("panelSection");
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
                        textColor: Settings.theme.color("panels/selection/buttons/text");
                        color: Settings.theme.color("panels/selection/buttons/color");
                        border.width: 1;
                        border.color: Settings.theme.color("panels/selection/buttons/border");
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
                        textColor: Settings.theme.color("panels/selection/buttons/text");
                        color: Settings.theme.color("panels/selection/buttons/color");
                        border.width: 1;
                        border.color: Settings.theme.color("panels/selection/buttons/border");
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
                    height: Constants.GridHeight / 2;
                    text: "Edit:";
                    font: Settings.theme.font("panelSection");
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
                            source: Settings.theme.icon("select-apply");
                            smooth: true;
                            MouseArea {
                                anchors.fill: parent;
                                onClicked: sketchView.selectionExtras.feather(featherTxt.value);
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
                            source: Settings.theme.icon("select-apply");
                            smooth: true;
                            MouseArea {
                                anchors.fill: parent;
                                onClicked: sketchView.selectionExtras.grow(growTxt.value, growTxt.value);
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
                            source: Settings.theme.icon("select-apply");
                            smooth: true;
                            MouseArea {
                                anchors.fill: parent;
                                onClicked: sketchView.selectionExtras.border(borderTxt.value, borderTxt.value);
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
                            source: Settings.theme.icon("select-apply");
                            smooth: true;
                            MouseArea {
                                anchors.fill: parent;
                                onClicked: sketchView.selectionExtras.shrink(shrinkTxt.value, shrinkTxt.value, false);
                            }
                        }
                    }
                }
            }
        }
        ScrollDecorator { flickableItem: selectOptionsFullFlickable; }
    }
}
