/* This file is part of the KDE project
 * Copyright (C) 2012 Dan Leinir Turthra Jensen <admin@leinir.dk>
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

import QtQuick 2.3
import org.krita.sketch 1.0

Item {
    id: base;
    property bool chooseBG: false;
    property alias fgColor: fgColor.color;
    property alias bgColor: bgColor.color;
    Image {
        anchors.fill: bgColor;
        source: Settings.theme.icon("checker-small");
    }
    Image {
        anchors.fill: fgColor;
        source: Settings.theme.icon("checker-small");
    }
    Rectangle {
        id: bgColor;
        anchors {
            right: base.right;
            bottom: base.bottom;
            bottomMargin: Constants.DefaultMargin;
            topMargin: Constants.DefaultMargin;
        }
        height: (base.height - Constants.DefaultMargin * 2) / 3 * 2;
        width: (base.width - Constants.DefaultMargin * 2) / 3 * 2;
        color: "white";
        MouseArea {
            anchors.fill: parent;
            onClicked: base.state = "bgChoice";
        }
    }
    Rectangle {
        id: fgColor;
        anchors {
            top: base.top;
            left: base.left;
            topMargin: Constants.DefaultMargin;
            leftMargin: Constants.DefaultMargin;
        }
        height: bgColor.height;
        width: bgColor.width;
        color: "black";
        MouseArea {
            anchors.fill: parent;
            onClicked: base.state = "";
        }
    }
    Rectangle {
        id: chosingBG;
        anchors {
            top: bgColor.top;
            left: bgColor.left;
            right: bgColor.right;
            bottom: bgColor.bottom;
            topMargin: -3;
            leftMargin: -3;
            bottomMargin: -2;
            rightMargin: -2;
        }
        border {
            width: 3;
            color: Settings.theme.color("components/colorSwatch/border");
        }
        color: "transparent";
        opacity: 0;
        radius: 3;
    }
    Rectangle {
        id: chosingFG;
        anchors {
            top: fgColor.top;
            left: fgColor.left;
            right: fgColor.right;
            bottom: fgColor.bottom;
            topMargin: -3;
            leftMargin: -3;
            bottomMargin: -2;
            rightMargin: -2;
        }
        border {
            width: 3;
            color: Settings.theme.color("components/colorSwatch/border");
        }
        color: "transparent";
        opacity: 0.5;
        radius: 3;
    }
    
    Image {
        id: swapColors;
        anchors {
            left: base.left;
            bottom: base.bottom;
        }
        height: (base.height - Constants.DefaultMargin * 2) / 3;
        width: (base.width - Constants.DefaultMargin * 2) / 3;
    }

    states: [
        State {
            name: "bgChoice";
            PropertyChanges { target: chosingFG; opacity: 0; }
            PropertyChanges { target: chosingBG; opacity: 0.5; }
            PropertyChanges { target: base; chooseBG: true; }
        }
    ]
    
    transitions: [
        Transition {
            from: "";
            to: "bgChoice";
            reversible: true;
            PropertyAnimation { properties: "opacity"; duration: Constants.AnimationDuration; easing.type: Easing.InOutQuad; }
        }
    ]
}
