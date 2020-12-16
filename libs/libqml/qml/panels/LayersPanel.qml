/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.3
import org.krita.sketch.components 1.0
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
                        asynchronous: true;
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
            Behavior on height { PropertyAnimation { duration: Constants.AnimationDuration;  } }
            Behavior on opacity { PropertyAnimation { duration: Constants.AnimationDuration;  } }
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
                    image: Settings.theme.icon("fileclip-black")
                    visible: KisClipBoard.clip;
                    onClicked: {
                        sketchView.selectionManager.paste();
                        addLayerButtons.state = "";
                    }
                }
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
                            leftMargin: Constants.DefaultMargin * model.depth;
                        }
                        height: model.activeLayer ? Constants.GridHeight + layerControls.height : Constants.GridHeight;
                        Behavior on height { NumberAnimation { duration: 100; } }
                        radius: 8
                        color: Settings.theme.color("panels/layers/layer/background");
                    }
                    Rectangle {
                        anchors {
                            top: layerBgRect.top;
                            left: layerBgRect.left;
                            right: layerBgRect.right;
                        }
                        height: Constants.GridHeight;
                        color: model.activeLayer ? Settings.theme.color("panels/layers/layer/active") : Settings.theme.color("panels/layers/layer/inactive");
                        radius: 8
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
                                asynchronous: true;
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
                                image: checked ? Settings.theme.icon("visible_on-small") : Settings.theme.icon("visible_off-small");
                                onCheckedChanged: layerModel.setVisible(model.index, checked);
                                tooltip: checked ? "Hide Layer" : "Show Layer";
                            }
                            Button {
                                width: height;
                                height: Constants.GridHeight / 2;
                                checkable: true;
                                checked: model.locked;
                                checkedColor: Settings.theme.color("panels/layers/layer/locked");
                                image: checked ? Settings.theme.icon("locked_on-small") : Settings.theme.icon("locked_off-small");
                                onCheckedChanged: layerModel.setLocked(model.index, checked);
                                tooltip: checked ? "Unlock Layer" : "Lock Layer";
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
                            text: model.compositeDetails;
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
                            text: model.percentOpacity + "%";
                            font: Settings.theme.font("small");
                            horizontalAlignment: Text.AlignRight;
                        }
                    }
                    Row {
                        id: layerControls;

                        anchors {
                            left: layerBgRect.left;
                            right: layerBgRect.right;
                            bottom: layerBgRect.bottom;
                        }

                        height: childrenRect.height;
                        opacity: model.activeLayer ? 1.0 : 0.0;
                        Behavior on opacity { NumberAnimation { duration: 100; } }
                        spacing: 0;

                        Button {
                            id: moveUpButton;
                            width: parent.width / 6;
                            height: width;
                            enabled: model.canMoveUp;
                            opacity: enabled ? 1.0 : 0.2;
                            image: Settings.theme.icon("layer_move_up");
                            onClicked: layerModel.moveUp();
                            tooltip: "Move Layer Up";
                        }
                        Button {
                            id: moveDownButton;
                            width: parent.width / 6;
                            height: width;
                            enabled: model.canMoveDown;
                            opacity: enabled ? 1.0 : 0.2;
                            image: Settings.theme.icon("layer_move_down");
                            onClicked: layerModel.moveDown();
                            tooltip: "Move Layer Down";
                        }
                        Button {
                            id: moveLeftButton;
                            width: parent.width / 6;
                            height: width;
                            enabled: model.canMoveLeft;
                            opacity: enabled ? 1.0 : 0.2;
                            image: Settings.theme.icon("layer_move_left");
                            onClicked: layerModel.moveLeft();
                            tooltip: "Move Layer out of Group";
                        }
                        Button {
                            id: moveRightButton;
                            width: parent.width / 6;
                            height: width;
                            enabled: model.canMoveRight;
                            opacity: enabled ? 1.0 : 0.2;
                            image: Settings.theme.icon("layer_move_right");
                            onClicked: layerModel.moveRight();
                            tooltip: "Move Layer into Group";
                        }
                        Button {
                            id: duplicateLayerButton;
                            width: parent.width / 6;
                            height: width;
                            image: Settings.theme.icon("layer_duplicate");
                            onClicked: layerModel.clone();
                            tooltip: "Duplicate Layer";
                        }
                        Button {
                            id: clearLayerButton;
                            width: parent.width / 6;
                            height: width;
                            image: Settings.theme.icon("layer_clear");
                            onClicked: layerModel.clear();
                            tooltip: "Clear Layer";
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
