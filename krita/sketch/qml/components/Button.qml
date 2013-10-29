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

Item {
    id: base;

    signal clicked();

    property bool checkable: false;
    property bool checked: false;

    property alias image: icon.source;
    property alias color: fill.color;
    property alias border: fill.border;
    property alias radius: fill.radius;
    property alias text: label.text;
    property alias textColor: label.color;
    property alias textSize: label.font.pixelSize;
    property alias bold: label.font.bold;
    property bool shadow: true;
    property bool enabled: true; // XXX: visualize disabledness
    property alias asynchronous: icon.asynchronous;

    property bool highlight: false;
    property color highlightColor: "transparent";

    property bool hasFocus: false;

    width: Constants.GridWidth;
    height: Constants.GridHeight;

    Rectangle {
        id: fill;
        anchors.fill: parent;
        anchors.margins: 0;
        color: base.highlight && mouse.pressed && base.enabled ? base.highlightColor : "transparent";
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
            opacity: base.checked ? 0.7 : 0;
            Behavior on opacity { NumberAnimation { duration: 150; } }
            anchors.fill: parent;
            anchors.margins: 2;
            color: "white";
            radius: base.height === base.width ? base.height / 2 - 1 : base.radius;
        }

        Image {
            id: icon;
            anchors.fill: parent;
            anchors.margins: 8;
            fillMode: Image.PreserveAspectFit;
            smooth: true;
            opacity: base.enabled ? 1 : 0.7;
            Behavior on opacity { NumberAnimation { duration: 150; } }

            sourceSize.width: width > height ? height : width;
            sourceSize.height: width > height ? height : width;
        }

        Label {
            id: label;
            anchors.verticalCenter: parent.verticalCenter;
            height: font.pixelSize;
            width: parent.width;
            horizontalAlignment: Text.AlignHCenter;
            elide: Text.ElideRight;
            opacity: base.enabled ? 1 : 0.7;
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
                if ( base.checkable ) {
                    base.checked = !base.checked;
                }
            }
        }
    }
    MouseArea {
        id: mouse;
        anchors.fill: parent;
        onClicked: {
            if (base.enabled) {
                base.clicked();
                if ( base.checkable ) {
                    base.checked = !base.checked;
                }
            }
        }
    }

    states: State {
        name: "pressed";
        when: (mouse.pressed || base.checked) && enabled;

        PropertyChanges {
            target: mouse
            anchors.topMargin: 0
        }
    }

    transitions: Transition {
        ParallelAnimation {
            NumberAnimation { properties: "size"; duration: 50; }
            ColorAnimation { duration: 50; }
        }
    }
}
