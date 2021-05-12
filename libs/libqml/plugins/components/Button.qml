/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.3
import org.krita.sketch 1.0

Item {
    id: base;

    signal clicked();

    property alias image: icon.source;
    //property color color: Settings.theme.color("components/button/base");
    // define background and highlight color transparent by default,
    // so they can be defined depending on the context; also make the icon half transparent on click
    // if the color is transparent #00000000
    property color color: "#00000000"
    property alias border: fill.border;
    property alias radius: fill.radius;
    property alias text: label.text;
    property color textColor: Settings.theme.color("components/button/text");
    property alias textSize: label.font.pixelSize;
    property alias bold: label.font.bold;
    property bool shadow: false;
    property bool enabled: true; // XXX: visualize disabledness
    property alias asynchronous: icon.asynchronous;

    property bool highlight: false;
    //property color highlightColor: Settings.theme.color("components/button/highlight");
    property color highlightColor: "#00000000"

    property bool checkable: false;
    property bool checked: false;
    property color checkedColor: Settings.theme.color("components/button/checked");

    property bool hasFocus: false;

    property string tooltip: "";

    width: Constants.GridWidth;
    height: Constants.GridHeight;

    Rectangle {
        id: fill;
        anchors.fill: parent;
        anchors.margins: 0;
        color: base.highlight && mouse.pressed && base.enabled ? base.highlightColor : base.color;
        visible: true

        Rectangle {
            anchors {
                left: parent.left;
                right: parent.right;
                bottom: parent.bottom;
                margins: fill.radius / 2;
            }
            height: fill.radius / 2;
            radius: fill.radius / 4;
            color: base.textColor;
            visible: base.hasFocus;
            opacity: 0.3
        }

        Rectangle {
            id: checkedVisualiser;
            opacity: base.checked ? 1 : 0;
            Behavior on opacity { NumberAnimation { duration: Constants.AnimationDuration; } }
            anchors.fill: parent;
            anchors.margins: 2;
            color: base.checkedColor;
            radius: base.height === base.width ? base.height / 2 - 1 : base.radius;
        }

        Image {
            id: icon;
            anchors.centerIn: parent
            width: parent.width > parent.height ? parent.height * 0.9 : parent.width * 0.9;
            height: width
            fillMode: Image.PreserveAspectFit;
            smooth: true;
            asynchronous: true;
            opacity: base.enabled ? (mouse.pressed && base.highlightColor == "#00000000" ? 0.5 : 1) : 0.7;
            Behavior on opacity { NumberAnimation { duration: Constants.AnimationDuration * 0.5; } }

            sourceSize.width: width;
            sourceSize.height: width;
        }

        Label {
            id: label;
            anchors.verticalCenter: parent.verticalCenter;
            height: font.pixelSize;
            width: parent.width;
            horizontalAlignment: Text.AlignHCenter;
            elide: Text.ElideRight;
            opacity: base.enabled ? 1 : 0.7;
            color: base.textColor;
        }
//         Rectangle {
//             id: enabledVisualiser;
//             opacity: base.enabled ? 0 : 0.7;
//             anchors.fill: parent;
//             color: "black";
//         }
    }

    SimpleTouchArea {
        anchors.fill: parent;
        onTouched: {
            if (base.enabled) {
                base.clicked();
                if (base.checkable) {
                    base.checked = !base.checked;
                }
            }
        }
    }
    MouseArea {
        id: mouse;
        anchors.fill: parent;
        hoverEnabled: true;
        acceptedButtons: Qt.LeftButton | Qt.RightButton;

        onClicked: {
            if(mouse.button == Qt.LeftButton && base.enabled) {
                base.clicked();
                if(base.checkable) {
                    base.checked = !base.checked;
                }
            } else if(mouse.button == Qt.RightButton && base.tooltip != "") {
                tooltip.show(base.width / 2, 0);
            }
        }
        onEntered: {
            hoverDelayTimer.start();
        }
        onPositionChanged: {
            if(hoverDelayTimer.running) {
                hoverDelayTimer.restart();
            }
        }
        onExited: {
            hoverDelayTimer.stop();
            tooltip.hide();
        }
    }

    Timer {
        id: hoverDelayTimer;
        interval: 1000;
        onTriggered: { if(base.tooltip != "") tooltip.show(base.width / 2, 0) }
    }

    Tooltip {
        id: tooltip;
        text: base.tooltip;
    }

    states: State {
        name: "pressed";
        when: (mouse.pressed || base.checked) && enabled;

        PropertyChanges {
            target: fill
            color: base.highlightColor
            anchors.topMargin: 0
        }
    }

    transitions: Transition {
        from: "";
        to: "down";
        reversible: true;
        ParallelAnimation {
            NumberAnimation { properties: "size"; duration: 150; }
            ColorAnimation { duration: 150; }
        }
    }
}
