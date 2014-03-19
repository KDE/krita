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
    colorSet: "layers";

    actions: [
        Button {
            id: backFromEditButton;
            width: height;
            height: Constants.ToolbarButtonSize
            image: Settings.theme.icon("back")
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
            image: Settings.theme.icon("add")
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
            image: Settings.theme.icon("edit")
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
            image: Settings.theme.icon("delete")
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
            Item {
                id: topSpacer;
                height: model.childCount == 0 ? 0 : Constants.DefaultMargin;
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
                color: model.activeLayer ? Settings.theme.color("panels/layers/layer/active") : Settings.theme.color("panels/layers/layer/inactive");
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
                Label {
                    id: layerNameLbl
                    anchors {
                        top: parent.top;
                        left: layerThumbContainer.right;
                        right: parent.right;
                    }
                    text: model.name;
                    color: Settings.theme.color("panels/layers/layer/text");
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

            color: Settings.theme.color("panels/layers/subheader");

            Row {
                anchors.centerIn: parent;
                height: childrenRect.height;
                width: childrenRect.width;
                Button {
                    width: height; height: Constants.ToolbarButtonSize * 0.9
                    image: Settings.theme.icon("layer_paint-black")
                    onClicked: {
                        layerModel.addLayer(0);
                        addLayerButtons.state = "";
                    }
                }
                Button {
                    width: height; height: Constants.ToolbarButtonSize * 0.9
                    image: Settings.theme.icon("layer_group-black")
                    onClicked: {
                        layerModel.addLayer(1);
                        addLayerButtons.state = "";
                    }
                }
                Button {
                    width: height; height: Constants.ToolbarButtonSize * 0.9
                    image: Settings.theme.icon("layer_filter-black")
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
                        color: model.activeLayer ? Settings.theme.color("panels/layers/layer/active") : Settings.theme.color("panels/layers/layer/inactive");
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
                        Label {
                            id: layerNameLbl
                            anchors {
                                top: parent.top;
                                left: layerThumbContainer.right;
                                right: parent.right;
                            }
                            text: model.name;
                            color: Settings.theme.color("panels/layers/layer/text");
                            elide: Text.ElideRight;
                        }
                        MouseArea {
                            anchors.fill: parent;
                            onClicked: layerModel.setActive(model.index);
                        }
                        Row {
                            id: modeButtons;
                            anchors {
                                left: layerThumbContainer.right;
                                bottom: parent.bottom;
                            }
                            height: childrenRect.height;

                            Button {
                                width: height;
                                height: Constants.GridHeight / 2;
                                checkable: true;
                                checked: model.visible;
                                checkedColor: Settings.theme.color("panels/layers/layer/visible");
                                image: checked ? Settings.theme.icon("visible_on_small") : Settings.theme.icon("visible_off_small");
                                onCheckedChanged: layerModel.setVisible(model.index, checked);
                            }
                            Button {
                                width: height;
                                height: Constants.GridHeight / 2;
                                checkable: true;
                                checked: model.locked;
                                checkedColor: Settings.theme.color("panels/layers/layer/locked");
                                image: checked ? Settings.theme.icon("locked_on_small") : Settings.theme.icon("locked_off_small");
                                onCheckedChanged: layerModel.setLocked(model.index, checked);
                            }
                        }
                        Label {
                            id: modeLabel;
                            anchors {
                                top: layerNameLbl.bottom;
                                right: parent.right;
                                rightMargin: Constants.DefaultMargin;
                                left: modeButtons.right;
                            }
                            text: "M: " + model.compositeDetails;
                            font: Settings.theme.font("small");
                            horizontalAlignment: Text.AlignRight;
                        }
                        Label {
                            anchors {
                                top: modeLabel.bottom;
                                right: parent.right;
                                rightMargin: Constants.DefaultMargin;
                                left: modeButtons.right;
                            }
                            text: "O: " + model.percentOpacity + "%";
                            font: Settings.theme.font("small");
                            horizontalAlignment: Text.AlignRight;
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
                        source: Settings.theme.icon("up");
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
                        source: Settings.theme.icon("down");
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
                        source: Settings.theme.icon("back");
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
                        source: Settings.theme.icon("forward");
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
