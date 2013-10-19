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
import org.krita.sketch 1.0

Panel {
    id: base;
    name: "Layers";
    panelColor: "#000000";

    actions: [
        Button {
            id: backFromEditButton;
            width: height;
            height: Constants.ToolbarButtonSize
            color: "transparent";
            image: "../images/svg/icon-back.svg"
            textColor: "white";
            shadow: false;
            highlight: false;
            onClicked: {
                fullViewStack.pop();
                backFromEditButton.visible = false;
                addButton.visible = true;
                removeButton.visible = true;
            }
            visible: false;
        },
        Button {
            id: addButton;
            width: height;
            height: Constants.ToolbarButtonSize
            color: "transparent";
            image: "../images/svg/icon-add.svg"
            textColor: "white";
            shadow: false;
            highlight: false;
            onClicked: {
                if (base.state === "full") {
                    addLayerButtons.toggle();
                }
                else {
                    layerModel.addLayer(0);
                }
            }
        },
        Item {
            width: editButton.visible ? 0 : editButton.width;
            height: Constants.ToolbarButtonSize;
        },
        Button {
            id: editButton;
            width: height;
            height: Constants.ToolbarButtonSize
            color: "transparent";
            image: "../images/svg/icon-edit.svg"
            textColor: "white";
            shadow: false;
            highlight: false;
            enabled: layerModel.count > 0;
            visible: (base.state === "full" && backFromEditButton.visible === false);
            onClicked: {
                fullViewStack.push(editLayerPage);
                backFromEditButton.visible = true;
                addButton.visible = false;
                removeButton.visible = false;
            }
        },
        Item {
            width: base.width - Constants.DefaultMargin - (Constants.ToolbarButtonSize * 3)
            height: Constants.ToolbarButtonSize;
        },
        Button {
            id: removeButton;
            width: height;
            height: Constants.ToolbarButtonSize
            color: "transparent";
            image: "../images/svg/icon-delete.svg"
            textColor: "white";
            shadow: false;
            highlight: false;
            enabled: layerModel.count > 0;
            onClicked: layerModel.deleteCurrentLayer();
        }
    ]

    peekContents: ListView {
        anchors.fill: parent;
        model: layerModel;
        delegate: Item {
            width: parent.width - Constants.DefaultMargin;
            height: childrenRect.height;
            Rectangle {
                id: topSpacer;
                height: model.childCount == 0 ? 0 : Constants.DefaultMargin;
                color: "transparent";
            }
            Rectangle {
                id: layerBgRect
                anchors {
                    top: topSpacer.bottom;
                    left: parent.left;
                    right: parent.right;
                    leftMargin: 8 * model.depth;
                }
                height: Constants.DefaultFontSize + 2*Constants.DefaultMargin;
                radius: 8
                opacity: model.activeLayer ? 0.5 : 0.2;
                color: "white";
            }
            Rectangle {
                anchors.fill: layerBgRect
                color: "transparent";
                Rectangle {
                    id: layerThumbContainer;
                    anchors {
                        verticalCenter: parent.verticalCenter;
                        left: parent.left;
                    }
                    height: Constants.DefaultFontSize + 2*Constants.DefaultMargin;
                    width: height;
                    color: "transparent";
                    Image {
                        anchors.centerIn: parent;
                        cache: false;
                        source: model.icon;
                        smooth: true;
                        width: parent.width * 0.8;
                        height: parent.height * 0.8;
                        fillMode: Image.PreserveAspectFit;
                    }
                }
                Text {
                    id: layerNameLbl
                    anchors {
                        top: parent.top;
                        left: layerThumbContainer.right;
                        right: parent.right;
                    }
                    text: model.name;
                    color: "black";
                    font.pixelSize: Constants.DefaultFontSize;
                    elide: Text.ElideRight;
                }
                MouseArea {
                    anchors.fill: parent;
                    onClicked: layerModel.setActive(model.index);
                }
            }
            Rectangle {
                id: bottomSpacer;
                anchors.top: layerBgRect.bottom;
                height: Constants.DefaultMargin;
                color: "transparent";
            }
        }
        ScrollDecorator {
            flickableItem: parent;
        }
    }

    fullContents: Item {
        anchors.fill: parent;

        Rectangle {
            id: addLayerButtons
            function toggle() { addLayerButtons.state = (addLayerButtons.state === "shown") ? "" : "shown"; }
            anchors {
                top: parent.top;
                left: parent.left;
                right: parent.right;
            }
            states: [
                State {
                    name: "shown";
                    PropertyChanges { target: addLayerButtons; height: Constants.GridHeight; opacity: 1; }
                }
            ]
            Behavior on height { PropertyAnimation { duration: 150;  } }
            Behavior on opacity { PropertyAnimation { duration: 150;  } }
            clip: true;
            height: 0;
            opacity: 0;
            Row {
                anchors.centerIn: parent;
                height: childrenRect.height;
                width: childrenRect.width;
                Button {
                    width: height; height: Constants.ToolbarButtonSize * 0.9
                    color: "transparent"; textColor: "white"; shadow: false; highlight: false;
                    image: "../images/svg/icon-layer_paint-black.svg"
                    onClicked: {
                        layerModel.addLayer(0);
                        addLayerButtons.state = "";
                    }
                }
                Button {
                    width: height; height: Constants.ToolbarButtonSize * 0.9
                    color: "transparent"; textColor: "white"; shadow: false; highlight: false;
                    image: "../images/svg/icon-layer_group-black.svg"
                    onClicked: {
                        layerModel.addLayer(1);
                        addLayerButtons.state = "";
                    }
                }
                Button {
                    width: height; height: Constants.ToolbarButtonSize * 0.9
                    color: "transparent"; textColor: "white"; shadow: false; highlight: false;
                    image: "../images/svg/icon-layer_filter-black.svg"
                    onClicked: {
                        layerModel.addLayer(2);
                        addLayerButtons.state = "";
                    }
                }
            }
        }
        PageStack {
            id: fullViewStack
            anchors {
                top: addLayerButtons.bottom;
                left: parent.left;
                right: parent.right;
                bottom: parent.bottom;
            }
            initialPage: ListView {
                anchors.fill: parent;
                model: layerModel;
                delegate: Item {
                    id: viewBase;
                    width: parent.width - Constants.DefaultMargin;
                    height: childrenRect.height;

                    Rectangle {
                        id: topSpacer;
                        height: model.childCount == 0 ? 0 : Constants.DefaultMargin;
                        color: "transparent";
                    }
                    Rectangle {
                        id: layerBgRect
                        anchors {
                            top: topSpacer.bottom;
                            left: parent.left;
                            right: parent.right;
                            leftMargin: 8 * model.depth;
                        }
                        height: Constants.GridHeight;
                        radius: 8
                        opacity: model.activeLayer ? 0.5 : 0.2;
                        color: "white";
                    }
                    Rectangle {
                        anchors.fill: layerBgRect
                        color: "transparent";
                        Rectangle {
                            id: layerThumbContainer;
                            anchors {
                                verticalCenter: parent.verticalCenter;
                                left: parent.left;
                            }
                            height: Constants.GridHeight;
                            width: height;
                            color: "transparent";
                            Image {
                                anchors.centerIn: parent;
                                cache: false;
                                source: model.icon;
                                smooth: true;
                                width: parent.width * 0.8;
                                height: parent.height * 0.8;
                                fillMode: Image.PreserveAspectFit;
                            }
                        }
                        Text {
                            id: layerNameLbl
                            anchors {
                                top: parent.top;
                                left: layerThumbContainer.right;
                                right: parent.right;
                            }
                            text: model.name;
                            color: "black";
                            font.pixelSize: Constants.DefaultFontSize;
                            elide: Text.ElideRight;
                        }
                        Text {
                            anchors {
                                top: layerNameLbl.bottom;
                                right: parent.right;
                                rightMargin: Constants.DefaultMargin;
                            }
                            text: "Mode: " + model.compositeDetails + ", " + model.percentOpacity + "%";
                            font.pixelSize: Constants.SmallFontSize;
                        }
                        MouseArea {
                            anchors.fill: parent;
                            onClicked: layerModel.setActive(model.index);
                        }
                        Row {
                            anchors {
                                left: layerThumbContainer.right;
                                bottom: parent.bottom;
                            }
                            height: childrenRect.height;
                            Rectangle {
                                width: Constants.DefaultFontSize;
                                height: width;
                                color: model.visible ? "silver" : "gray";
                                Text {
                                    anchors.centerIn: parent;
                                    font.pixelSize: Constants.SmallFontSize;
                                    color: model.visible ? "black" : "white";
                                    text: "V"
                                }
                                MouseArea {
                                    anchors.fill: parent;
                                    onClicked: layerModel.setVisible(model.index, !model.visible);
                                }
                            }
                            Rectangle {
                                width: Constants.DefaultFontSize;
                                height: width;
                                color: model.locked ? "silver" : "gray";
                                Text {
                                    anchors.centerIn: parent;
                                    font.pixelSize: Constants.SmallFontSize;
                                    color: model.locked ? "black" : "white";
                                    text: "L"
                                }
                                MouseArea {
                                    anchors.fill: parent;
                                    onClicked: layerModel.setLocked(model.index, !model.locked);
                                }
                            }
                        }
                    }
                    Image {
                        id: moveUpButton;
                        anchors {
                            horizontalCenter: layerBgRect.left;
                            top: layerBgRect.top;
                        }
                        height: Constants.GridHeight / 3;
                        width: height;
                        visible: model.canMoveUp;
                        fillMode: Image.PreserveAspectFit;
                        smooth: true;
                        source: "../images/svg/icon-up.svg";
                        MouseArea {
                            anchors.fill: parent;
                            onClicked: layerModel.moveUp();
                        }
                    }
                    Image {
                        id: moveDownButton;
                        anchors {
                            horizontalCenter: layerBgRect.left;
                            bottom: layerBgRect.bottom;
                        }
                        height: Constants.GridHeight / 3;
                        width: height;
                        visible: model.canMoveDown;
                        fillMode: Image.PreserveAspectFit;
                        smooth: true;
                        source: "../images/svg/icon-down.svg";
                        MouseArea {
                            anchors.fill: parent;
                            onClicked: layerModel.moveDown();
                        }
                    }
                    Image {
                        id: moveLeftButton;
                        anchors {
                            right: layerBgRect.left;
                            verticalCenter: layerBgRect.verticalCenter;
                        }
                        height: Constants.GridHeight / 3;
                        width: height;
                        visible: model.canMoveLeft;
                        fillMode: Image.PreserveAspectFit;
                        smooth: true;
                        source: "../images/svg/icon-back.svg";
                        MouseArea {
                            anchors.fill: parent;
                            onClicked: layerModel.moveLeft();
                        }
                    }
                    Image {
                        id: moveRightButton;
                        anchors {
                            left: layerBgRect.left;
                            verticalCenter: layerBgRect.verticalCenter;
                        }
                        height: Constants.GridHeight / 3;
                        width: height;
                        visible: model.canMoveRight;
                        fillMode: Image.PreserveAspectFit;
                        smooth: true;
                        source: "../images/svg/icon-forward.svg";
                        MouseArea {
                            anchors.fill: parent;
                            onClicked: layerModel.moveRight();
                        }
                    }
                    Rectangle {
                        id: bottomSpacer;
                        anchors.top: layerBgRect.bottom;
                        height: Constants.DefaultMargin;
                        color: "transparent";
                    }
                }
                ScrollDecorator {
                    flickableItem: parent;
                }
            }
        }
    }

    Component { id: editLayerPage; EditLayerPage { layersModel: layerModel; isShown: backFromEditButton.visible  } }
}
