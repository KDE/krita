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
import org.krita.sketch 1.0
import "../components"

Item {
    id: base;

    property bool collapsed: true;

    property alias newButtonChecked: newButton.checked;
    property alias openButtonChecked: openButton.checked;

    signal buttonClicked( string button );

    height: Constants.GridHeight;

    Rectangle {
        id: background;
        color: "#000000"
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
                width: Constants.GridWidth;
                height: Constants.GridHeight;
                color: "#000000"
                shadow: false
                image: "../images/svg/icon-filenew.svg"
                highlightColor: Constants.Theme.HighlightColor;
                onClicked: base.buttonClicked( "new" );
            }
            Button {
                id: openButton;
                width: Constants.GridWidth;
                height: Constants.GridHeight;
                color: "#000000"
                shadow: false
                image: "../images/svg/icon-fileopen.svg"
                highlightColor: Constants.Theme.HighlightColor;
                onClicked: base.buttonClicked( "open" );
            }
            Button {
                id: saveButton;
                width: Constants.GridWidth;
                height: Constants.GridHeight;
                color: "#000000"
                shadow: false
                image: "../images/svg/icon-filesave.svg"
                highlightColor: Constants.Theme.HighlightColor;
                onClicked: base.buttonClicked( "save" );
            }
            Button {
                id: saveAsButton;
                width: Constants.GridWidth;
                height: Constants.GridHeight;
                color: "#000000"
                shadow: false
                image: "../images/svg/icon-filesaveas.svg"
                highlightColor: Constants.Theme.HighlightColor;
                onClicked: base.buttonClicked( "saveAs" );
            }
        }

        Row {
            anchors.right: parent.right;

            Button {
                id: undoButton;
                width: Constants.GridWidth;
                height: Constants.GridHeight;
                color: "#000000"
                shadow: false
                image: "../images/svg/icon-undo.svg"
                highlightColor: Constants.Theme.HighlightColor;
                enabled: sketchView.canUndo;
                onClicked: base.buttonClicked( "undo" );
            }
            Button {
                id: redoButton;
                width: Constants.GridWidth;
                height: Constants.GridHeight;
                color: "#000000"
                shadow: false
                image: "../images/svg/icon-redo.svg"
                highlightColor: Constants.Theme.HighlightColor;
                enabled: sketchView.canRedo;
                onClicked: base.buttonClicked( "redo" );
            }
            Button {
                id: zoomOutButton;
                width: Constants.GridWidth;
                height: Constants.GridHeight;
                color: "#000000"
                shadow: false
                image: "../images/svg/icon-delete.svg"
                highlightColor: Constants.Theme.HighlightColor;
                onClicked: base.buttonClicked( "zoomOut" );
            }
            Button {
                id: zoomInButton;
                width: Constants.GridWidth;
                height: Constants.GridHeight;
                color: "#000000"
                shadow: false
                image: "../images/svg/icon-add.svg"
                highlightColor: Constants.Theme.HighlightColor;
                onClicked: base.buttonClicked( "zoomIn" );
            }
            Item {
                width: Constants.GridWidth;
                height: Constants.GridHeight;
            }
            Button {
                id: switchButton;
                width: visible ? Constants.GridWidth : 0;
                height: Constants.GridHeight;
                visible: (typeof switchToDesktopAction !== "undefined");
                color: "#000000"
                shadow: false
                enabled: (typeof switchToDesktopAction === "undefined") ? false : switchToDesktopAction.enabled;
                image: "../images/svg/icon-switch.svg"
                highlightColor: Constants.Theme.HighlightColor;
                onClicked: {
                    base.buttonClicked( "switchToDesktop" );
                    base.collapsed = !base.collapsed;
                }
            }
            Button {
                id: minimizeButton;
                width: Constants.GridWidth;
                height: Constants.GridHeight;
                color: "#000000"
                shadow: false
                image: "../images/svg/icon-minimize.svg"
                highlightColor: Constants.Theme.HighlightColor;
                onClicked: {
                    base.buttonClicked( "minimize" );
                    base.collapsed = !base.collapsed;
                }
            }
            Button {
                id: closeButton;
                width: Constants.GridWidth;
                height: Constants.GridHeight;
                color: "#000000"
                shadow: false
                image: "../images/svg/icon-close.svg"
                highlightColor: Constants.Theme.HighlightColor;
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
                image: "../images/svg/icon-help.svg"
                highlightColor: Constants.Theme.HighlightColor;
                onClicked: base.buttonClicked( "help" );
            }
            Button {
                id: settingsButton;
                width: Constants.GridWidth;
                height: Constants.GridHeight;
                color: "#000000"
                shadow: false
                image: "../images/svg/icon-settings.svg"
                highlightColor: Constants.Theme.HighlightColor;
                onClicked: base.buttonClicked( "settings" );
            }*/
        }
    }

    Rectangle {
        anchors.bottom: parent.top;
        anchors.bottomMargin: -8;
        anchors.horizontalCenter: parent.horizontalCenter;

        width: Constants.GridWidth * 1.5;
        height: Constants.GridHeight / 2 + 8;

        color: "#000000"
        radius: 8;

        Label {
            text: "Menu";

            anchors.centerIn: parent;
            anchors.verticalCenterOffset: -4;

            font.bold: true
            font.pixelSize: Constants.DefaultFontSize
            color: "white";
        }

        MouseArea {
            id: mousearea1
            anchors.fill: parent;
            onClicked: base.collapsed = !base.collapsed;
        }

        SimpleTouchArea {
            anchors.fill: parent;
            onTouched: base.collapsed = !base.collapsed;
        }
    }

    states: State {
        name: "collapsed";
        when: base.collapsed;

        PropertyChanges { target: base; height: 0 }
    }

    transitions: Transition {
        NumberAnimation { duration: 200; properties: "height,opacity"; easing.type: Easing.InOutQuad }
    }
}
