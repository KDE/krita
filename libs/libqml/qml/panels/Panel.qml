/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.3
import org.krita.sketch 1.0
import org.krita.draganddrop 1.0 as DnD
import org.krita.sketch.components 1.0

Item {
    id: base;

    property bool roundTop: false;

    property string colorSet: "base";
    property string name;

    property alias actions: actionsLayout.children;
    property alias peekContents: peek.children;
    property alias fullContents: full.children;

    property real editWidth: Constants.GridWidth * 4;

    property Item page;
    property Item lastArea;

    signal collapsed();
    signal peek();
    signal full();
    signal dragStarted();
    signal drop(int action);

    state: "collapsed";

    Item {
        id: fill;

        width: parent.width;
        height: parent.height;
        z: 2;

        Rectangle {
            id: rectangle3
            anchors.fill: parent;
            color: "transparent";// base.panelColor;
            clip: true;

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

            Rectangle {
                id: background;
                anchors {
                    top: parent.top;
                    bottom: parent.bottom;
                    left: parent.left;
                    right: parent.right;
                    topMargin: Constants.DefaultMargin;
                    bottomMargin: Constants.DefaultMargin;
                }
                color: Settings.theme.color("panels/" + base.colorSet + "/base");
//                 Rectangle {
//                     id: rectangle4
//                     anchors.fill: parent;
//                     color: "#ffffff"
//                     opacity: 0.630
//                     color: Settings.theme.color(base.colorSet + "/subheader")
//                 }
            }

            Item {
                id: peek;
                clip: true;
                anchors.top: parent.top;
                anchors.bottom: header.top;
                anchors.left: parent.left;
                anchors.right: parent.right;
            }

            Item {
                id: full;
                clip: true;
                anchors.top: header.bottom;
                anchors.bottom: footer.top;
                anchors.left: parent.left;
                anchors.right: parent.right;
            }

            Item {
                id: header;

                anchors.left: parent.left
                anchors.right: parent.right;
                height: Constants.GridHeight;

                Rectangle {
                    id: rectangle1
                    anchors.fill: parent;
                    color: Settings.theme.color("panels/" + base.colorSet + "/header");
                    radius: Constants.DefaultMargin;
                }

                Rectangle {
                    id: headerCornerFill;
                    anchors.bottom: parent.bottom;
                    anchors.left: parent.left;
                    anchors.right: parent.right;
                    height: Constants.DefaultMargin;
                    color: Settings.theme.color("panels/" + base.colorSet + "/header");
                }

                DnD.DragArea {
                    anchors.fill: parent;
                    delegate: base.dragDelegate;
                    source: base;
                    enabled: base.state === "full";

                    onDragStarted: {
                        handle.opacity = 1;
                        handle.dragStarted();
                    }
                    onDrop: {
                        if (action == Qt.IgnoreAction) {
                            handle.opacity = 0;
                        }
                        handle.drop(action);
                    }

                    MouseArea {

                    }
                }

                Flow {
                    id: actionsLayout;
                    anchors {
                        verticalCenter: parent.verticalCenter;
                        left: parent.left;
                        right: parent.right;
                    }
                    height: Constants.ToolbarButtonSize;
                }
            }

            Shadow { anchors { top: header.bottom; left: header.left; right: header.right; } }

            Item {
                id: footer;

                anchors.bottom: parent.bottom;
                width: parent.width;
                height: Constants.GridHeight;
                clip: true;

                Rectangle {
                    id: rectanglefoot
                    color: Settings.theme.color("panels/" + base.colorSet + "/header");
                    width: parent.width;
                    height: parent.height + Constants.DefaultMargin;
                    y: -Constants.DefaultMargin;
                    radius: Constants.DefaultMargin;
                }

                Rectangle {
                    anchors.fill: parent;
                    color: "transparent";//base.panelColor;

                    Label {
                        anchors.left: parent.left;
                        anchors.leftMargin: 16;
                        anchors.baseline: parent.bottom;
                        anchors.baselineOffset: -16;

                        text: base.name;
                        color: Settings.theme.color("panels/" + base.colorSet + "/headerText");
                        font: Settings.theme.font("panelHeader");
                    }
                }

                DnD.DragArea {
                    anchors.fill: parent;
                    delegate: base.dragDelegate;
                    source: base;

                    onDragStarted: {
                        handle.opacity = 1;
                        handle.dragStarted();
                    }
                    onDrop: {
                        if (action == Qt.IgnoreAction) {
                            handle.opacity = 0;
                        }
                        handle.drop(action);
                    }

                    MouseArea {

                    }
                }
            }

            Shadow { anchors { bottom: footer.top; left: footer.left; right: footer.right; } rotation: 180; }
        }
    }

    Item {
        id: handle;

        Behavior on y { id: yHandleAnim; enabled: false; NumberAnimation { onRunningChanged: handle.fixParent(); } }
        Behavior on x { id: xHandleAnim; enabled: false; NumberAnimation { onRunningChanged: handle.fixParent(); } }

        width: Constants.GridWidth;
        height: Constants.GridHeight / 2;
        opacity: 0;

        property bool dragging: false;

        function fixParent() {
            if (!handleDragArea.dragging && !xHandleAnim.animation.running && !yHandleAnim.animation.running) {
                xHandleAnim.enabled = false;
                yHandleAnim.enabled = false;
                handle.parent = base;
                handle.x = 0;
                handle.y = 0;
            }
        }

        function dragStarted() {
            base.dragStarted();
            handle.parent = base.page;
            Krita.MouseTracker.addItem(handle, Qt.point(-handle.width / 2, -handle.height / 2));
            handle.dragging = true;
        }

        function drop(action) {
            base.drop(action);

            Krita.MouseTracker.removeItem(handle);

            xHandleAnim.enabled = true;
            yHandleAnim.enabled = true;
            dragging = false;

            var handlePos = base.mapToItem(base.page, 0, 0);
            handle.x = handlePos.x;
            handle.y = handlePos.y;
        }

        Rectangle {
            visible: (base.state === "collapsed") ? !base.roundTop : true;
            anchors {
                top: parent.top;
                left: handleBackground.left;
                right: handleBackground.right;
            }
            color: Settings.theme.color("panels/" + base.colorSet + "/header");
            radius: 0

            height: (base.state === "peek") ? Constants.GridHeight / 2 : Constants.GridHeight / 4 + 1
        }

        Rectangle {
            id: handleBackground

            width: Constants.GridWidth;
            height: Constants.GridHeight / 2;

            color: Settings.theme.color("panels/" + base.colorSet + "/header");
            radius: 8

            Label {
                id: handleLabel;

                anchors.centerIn: parent;

                text: base.name;
                color: Settings.theme.color("panels/" + base.colorSet + "/headerText");
                font: Settings.theme.font("panelHandle");
            }
        }

        DnD.DragArea {
            id: handleDragArea;

            anchors.fill: parent;

            source: base;

            onDragStarted: {
                base.lastArea = base.parent;
                handle.dragStarted();
            }
            onDrop: {
                handle.drop(action);
            }

            MouseArea {
                anchors.fill: parent;
                onClicked: base.state = base.state == "peek" ? "collapsed" : "peek";
            }
            SimpleTouchArea {
                anchors.fill: parent;
                onTouched: base.state = base.state == "peek" ? "collapsed" : "peek";
            }
        }
    }

    states: [
        State {
            name: "collapsed";

            PropertyChanges { target: base; width: Constants.GridWidth; }
            PropertyChanges { target: fill; opacity: 0; height: 0; }
            PropertyChanges { target: handle; opacity: 1; }
            PropertyChanges { target: background; anchors.topMargin: 0; }
        },
        State {
            name: "peek";

            PropertyChanges { target: base; width: Constants.IsLandscape ? Constants.GridWidth * 4 : Constants.GridWidth * 2; }
            PropertyChanges { target: fill; height: Constants.GridHeight * 3.75; y: handle.height; }
            PropertyChanges { target: handle; opacity: 1; }
            PropertyChanges { target: peek; opacity: 1; }
            PropertyChanges { target: full; opacity: 0; }
            AnchorChanges { target: header; anchors.bottom: rectangle3.bottom }
            PropertyChanges { target: footer; opacity: 0; }
            PropertyChanges { target: background; anchors.topMargin: 0; }
            PropertyChanges { target: headerCornerFill; height: Constants.DefaultMargin; }
            AnchorChanges { target: headerCornerFill; anchors.bottom: undefined; anchors.top: header.top; }
        },
        State {
            name: "full";
            PropertyChanges { target: peek; opacity: 0; }
            PropertyChanges { target: full; opacity: 1; }
            PropertyChanges { target: handle; height: 0; }
        },
        State {
            name: "edit";
            PropertyChanges { target: peek; opacity: 0; }
            PropertyChanges { target: full; opacity: 1; }
            PropertyChanges { target: base; width: base.editWidth; }
        }
    ]

    transitions: [
        Transition {
            from: "collapsed";
            to: "peek";

            SequentialAnimation {
                ScriptAction { script: base.peek(); }
                AnchorAnimation { targets: [ header ] ; duration: 0; }
                PropertyAction { targets: [ header, footer ]; properties: "height,width,opacity" }
                PropertyAction { targets: [ base ]; properties: "width"; }
                PropertyAction { targets: [ fill ]; properties: "y"; }
                NumberAnimation { targets: [ base, fill, handle, peek, full ]; properties: "height,opacity"; duration: Constants.AnimationDuration; }
            }
        },
        Transition {
            from: "peek";
            to: "collapsed";

            SequentialAnimation {
                NumberAnimation { targets: [ base, fill, handle, peek, full ]; properties: "height,opacity"; duration: Constants.AnimationDuration; }
                AnchorAnimation { targets: [ header ] ; duration: 0; }
                PropertyAction { targets: [ fill ]; properties: "y"; }
                PropertyAction { targets: [ base ]; properties: "width"; }
                PropertyAction { targets: [ header, footer ]; properties: "height,width,opacity" }
                ScriptAction { script: base.collapsed(); }
            }
        },
        Transition {
            from: "peek";
            to: "full";
            reversible: true;

            NumberAnimation { properties: "height,width,opacity"; duration: 0; }
            ScriptAction { script: base.full(); }
        },
        Transition {
            from: "collapsed";
            to: "full";

            NumberAnimation { properties: "opacity"; duration: 100; }
            ScriptAction { script: base.full(); }
        },
        Transition {
            from: "full";
            to: "collapsed";

            NumberAnimation { properties: "opacity"; duration: 100; }
            ScriptAction { script: base.collapsed(); }
        },
        Transition {
            from: "full"
            to: "edit"
            reversible: true;

            NumberAnimation { properties: "width"; duration: Constants.AnimationDuration; }
        }
    ]
}
