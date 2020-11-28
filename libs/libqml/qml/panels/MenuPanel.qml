/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.3
import org.krita.sketch 1.0
import org.krita.sketch.components 1.0

Item {
    id: base;

    property bool collapsed: true;

    property alias newButtonChecked: newButton.checked;
    property alias openButtonChecked: openButton.checked;

    signal buttonClicked( string button );

    height: Constants.GridHeight;

    Rectangle {
        anchors.bottom: parent.top;
        anchors.bottomMargin: -8;
        anchors.horizontalCenter: parent.horizontalCenter;

        width: Constants.GridWidth * 1.5;
        height: Constants.GridHeight / 2 + 8;

        color: Settings.theme.color("panels/menu/base");
        radius: 8;

        Label {
            text: "Menu";

            anchors.centerIn: parent;
            anchors.verticalCenterOffset: -4;

            font: Settings.theme.font("panelHandle");
            color: Settings.theme.color("panels/menu/text");
        }

        MouseArea {
            anchors.fill: parent;
            onClicked: base.collapsed = !base.collapsed;
        }

        SimpleTouchArea {
            anchors.fill: parent;
            onTouched: base.collapsed = !base.collapsed;
        }
    }

    Rectangle {
        id: background;
        color: Settings.theme.color("panels/menu/base");
        anchors.fill: parent;

        MouseArea {
            // This mouse area blocks any mouse click from passing through the panel. We need this to ensure we don't accidentally pass
            // any clicks to the collapsing area, or to the canvas, by accident.
            anchors.fill: parent;
            // This will always work, and never do anything - but we need some kind of processed thing in here to activate the mouse area
            onClicked: parent.focus = true;
        }
        SimpleTouchArea {
            // As above, but for touch events
            anchors.fill: parent;
            onTouched: parent.focus = true;
        }

        Row {
            Button {
                id: newButton;
                image: Settings.theme.icon("filenew")
                highlightColor: Settings.theme.color("panels/menu/buttonHighlight");
                tooltip: "New Image"
                onClicked: base.buttonClicked( "new" );
            }
            Button {
                id: openButton;
                image: Settings.theme.icon("fileopen")
                highlightColor: Settings.theme.color("panels/menu/buttonHighlight");
                tooltip: "Open Image"
                onClicked: base.buttonClicked( "open" );
            }
            Button {
                id: saveButton;
                image: Settings.theme.icon("filesave")
                highlightColor: Settings.theme.color("panels/menu/buttonHighlight");
                tooltip: "Save Image"
                onClicked: base.buttonClicked( "save" );
            }
            Button {
                id: saveAsButton;
                image: Settings.theme.icon("filesaveas")
                highlightColor: Settings.theme.color("panels/menu/buttonHighlight");
                tooltip: "Save Image As..."
                onClicked: base.buttonClicked( "saveAs" );
            }
        }

        Row {
            anchors.right: parent.right;

            Button {
                id: undoButton;
                image: Settings.theme.icon("undo")
                highlightColor: Settings.theme.color("panels/menu/buttonHighlight");
                enabled: sketchView.canUndo;
                tooltip: "Undo"
                onClicked: base.buttonClicked( "undo" );
            }
            Button {
                id: redoButton;
                image: Settings.theme.icon("redo")
                highlightColor: Settings.theme.color("panels/menu/buttonHighlight");
                enabled: sketchView.canRedo;
                tooltip: "Redo"
                onClicked: base.buttonClicked( "redo" );
            }
            Button {
                id: zoomOutButton;
                image: Settings.theme.icon("delete")
                highlightColor: Settings.theme.color("panels/menu/buttonHighlight");
                tooltip: "Zoom Out"
                onClicked: base.buttonClicked( "zoomOut" );
            }
            Button {
                id: zoomInButton;
                image: Settings.theme.icon("add")
                highlightColor: Settings.theme.color("panels/menu/buttonHighlight");
                tooltip: "Zoom In"
                onClicked: base.buttonClicked( "zoomIn" );
            }
            Item {
                width: Constants.GridWidth;
                height: Constants.GridHeight;
            }
            Button {
                id: switchButton;
                width: visible ? Constants.GridWidth : 0;
                visible: (typeof switchToDesktopAction !== "undefined");
                enabled: (typeof switchToDesktopAction === "undefined") ? false : switchToDesktopAction.enabled;
                image: Settings.theme.icon("switch")
                highlightColor: Settings.theme.color("panels/menu/buttonHighlight");
                tooltip: "Switch to Desktop Mode"
                onClicked: {
                    base.buttonClicked( "switchToDesktop" );
                    base.collapsed = !base.collapsed;
                }
            }
            Button {
                id: minimizeButton;
                image: Settings.theme.icon("minimize")
                highlightColor: Settings.theme.color("panels/menu/buttonHighlight");
                tooltip: "Minimize"
                onClicked: {
                    base.buttonClicked( "minimize" );
                    base.collapsed = !base.collapsed;
                }
            }
            Button {
                id: closeButton;
                image: Settings.theme.icon("close")
                highlightColor: Settings.theme.color("panels/menu/buttonHighlight");
                tooltip: "Close"
                onClicked: base.buttonClicked( "close" );
            }
            /*Item {
                width: Constants.GridWidth;
                height: Constants.GridHeight;
            }
            Button {
                id: helpButton;
                width: Constants.GridWidth;
                height: Constants.GridHeight;
                color: "#000000"
                shadow: false
                image: Settings.theme.icon("help")
                highlightColor: Constants.Theme.HighlightColor;
                onClicked: base.buttonClicked( "help" );
            }
            Button {
                id: settingsButton;
                width: Constants.GridWidth;
                height: Constants.GridHeight;
                color: "#000000"
                shadow: false
                image: Settings.theme.icon("settings")
                highlightColor: Constants.Theme.HighlightColor;
                onClicked: base.buttonClicked( "settings" );
            }*/
        }
    }

    states: State {
        name: "collapsed";
        when: base.collapsed;

        PropertyChanges { target: base; height: 0 }
    }

    transitions: Transition {
        NumberAnimation { duration: Constants.AnimationDuration; properties: "height,opacity"; easing.type: Easing.InOutQuad }
    }
}
