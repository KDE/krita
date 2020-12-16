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
    name: "Presets";
    colorSet: "presets";

    /*actions: [
        Button {
            id: addButton;

            width: Constants.GridWidth / 2
            height: Constants.GridHeight;

            color: "transparent";
            image: "../images/svg/icon-add.svg"
            textColor: "white";
            shadow: false;
            highlight: false;
            onClicked: base.toggleEdit();

            states: State {
                name: "edit"
                when: base.state == "edit";

                PropertyChanges { target: addButton; text: "Cancel"; color: Constants.Theme.NegativeColor; width: Constants.GridWidth * 2; }
            }

            transitions: Transition {
                to: "edit";
                reversible: true;

                ParallelAnimation {
                    NumberAnimation { target: addButton; properties: "width"; duration: Constants.AnimationDuration; }
                    ColorAnimation { target: addButton; properties: "color"; duration: Constants.AnimationDuration; }
                }
            }
        },
        Button {
            id: editButton;

            width: Constants.GridWidth / 2
            height: Constants.GridHeight;

            text: ""
            image: "../images/svg/icon-edit.svg"
            color: "transparent";
            textColor: "white";
            shadow: false;
            highlight: false;
            onClicked: base.toggleEdit();

            states: State {
                name: "edit"
                when: base.state == "edit";

                PropertyChanges { target: editButton; text: "Save"; color: Constants.Theme.PositiveColor; width: Constants.GridWidth * 2; }
            }

            transitions: Transition {
                to: "edit";
                reversible: true;

                ParallelAnimation {
                    NumberAnimation { target: editButton; properties: "width"; duration: Constants.AnimationDuration; }
                    ColorAnimation { target: editButton; properties: "color"; duration: Constants.AnimationDuration; }
                }
            }
        }
    ]*/

    PresetModel {
        id: presetsModel;
        view: sketchView.view;
        onCurrentPresetChanged: {
            peekViewGrid.currentIndex = nameToIndex(currentPreset);
            fullViewGrid.currentIndex = nameToIndex(currentPreset);
        }
    }
    Connections {
        target: sketchView;
        onLoadingFinished: {
//            if (window.applicationName === undefined) {
                if(toolManager.currentTool === null)
                    toolManager.requestToolChange("KritaShape/KisToolBrush");
                presetsModel.currentPreset = Settings.lastPreset;
//            }
        }
    }

    peekContents: GridView {
        id: peekViewGrid;
        anchors.fill: parent;
        keyNavigationWraps: false

        model: presetsModel;
        delegate: Button {
            width: Constants.GridWidth;
            height: Constants.GridHeight;

            checked: GridView.isCurrentItem;
            checkedColor: Settings.theme.color("panels/presets/preset/active");

            color: Settings.theme.color("panels/presets/preset/inactive")

            highlightColor: Settings.theme.color("panels/presets/preset/active")

            Image {
                anchors {
                    bottom: peekLabel.top
                    top: parent.top;
                    horizontalCenter: parent.horizontalCenter;
                    margins: 2
                }
                source: model.image
                fillMode: Image.PreserveAspectFit;
                asynchronous: true;
            }
            Label {
                    id: peekLabel
                    anchors {
                        left: parent.left
                        right: parent.right
                        bottom: parent.bottom
                        margins: 2
                    }
                    elide: Text.ElideMiddle;
                    text: model.text;
                    font.pixelSize: Constants.SmallFontSize
                    horizontalAlignment: Text.AlignHCenter
            }

            onClicked: {
                presetsModel.activatePreset(index);
                toolManager.requestToolChange("KritaShape/KisToolBrush");
                peekViewGrid.currentIndex = index;
                fullViewGrid.currentIndex = index;
            }
        }

        cellWidth: Constants.GridWidth;
        cellHeight: Constants.GridHeight;
        ScrollDecorator {
            flickableItem: parent;
        }
    }

    fullContents: PageStack {
        id: contentArea;
        anchors.fill: parent;
        initialPage: GridView {
            id: fullViewGrid;
            anchors.fill: parent;
            model: presetsModel;
            delegate: Item {
                height: Constants.GridHeight;
                width: contentArea.width;
                Rectangle {
                    anchors.fill: parent;
                    color: (model.name === presetsModel.currentPreset) ?
                        Settings.theme.color("panels/presets/preset/active") :
                        Settings.theme.color("panels/presets/preset/inactive");
                }
                Rectangle {
                    id: presetThumbContainer;
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
                        source: model.image;
                        smooth: true;
                        width: parent.width * 0.8;
                        height: parent.height * 0.8;
                        fillMode: Image.PreserveAspectFit;
                        asynchronous: true;
                    }
                }
                Label {
                    anchors {
                        top: parent.top;
                        left: presetThumbContainer.right;
                        right: parent.right;
                        bottom: parent.bottom;
                    }
                    wrapMode: Text.Wrap;
                    elide: Text.ElideRight;
                    text: model.text;
                    maximumLineCount: 3;
                }
                MouseArea {
                    anchors.fill: parent;
                    onClicked: {
                        presetsModel.activatePreset(index);
                        toolManager.requestToolChange("KritaShape/KisToolBrush");
                        peekViewGrid.currentIndex = index;
                        fullViewGrid.currentIndex = index;
                    }
                }
            }

            cellWidth: contentArea.width;
            cellHeight: Constants.GridHeight;
            ScrollDecorator {
                flickableItem: parent;
            }
        }
    }

    onStateChanged: if ( state != "edit" && contentArea.depth > 1 ) {
        contentArea.pop();
    }

    function toggleEdit() {
        if ( base.state == "edit" ) {
            base.state = "full";
            contentArea.pop();
        } else if ( base.state == "full" ) {
            base.state = "edit";
            contentArea.push( editPresetPage );
        }
    }

    Component { id: editPresetPage; EditPresetPage { } }
}
