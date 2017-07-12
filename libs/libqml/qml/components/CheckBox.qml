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
    property bool checked: false;
    property alias text: txt.text;
    onCheckedChanged: state = checked ? "checked" : "";
    // Double-double margins looks odd, but it's to ensure the /icon/ has the right amount of padding
    height: Constants.DefaultFontSize + Constants.DefaultMargin * 4;
    Image {
        id: notCheckedImg;
        anchors {
            left: parent.left;
            top: parent.top;
            bottom: parent.bottom;
            margins: Constants.DefaultMargin;
        }
        width: height;
        smooth: true;
        source: Settings.theme.icon("checkbox-unchecked");
    }
    Image {
        id: checkedImg;
        anchors {
            left: parent.left;
            top: parent.top;
            bottom: parent.bottom;
            margins: Constants.DefaultMargin;
        }
        width: height;
        smooth: true;
        opacity: 0;
        source: Settings.theme.icon("checkbox-checked");
    }
    Label {
        id: txt;
        anchors {
            top: parent.top;
            left: notCheckedImg.right;
            leftMargin: Constants.DefaultMargin;
            right: parent.right;
            bottom: parent.bottom;
        }
    }
    MouseArea {
        anchors.fill: parent;
        onClicked: checked = !checked;
    }
    states: [
        State {
            name: "checked";
            PropertyChanges { target: notCheckedImg; opacity: 0; }
            PropertyChanges { target: checkedImg; opacity: 1; }
        }
    ]
    transitions: [
        Transition {
            from: "";
            to: "checked";
            reversible: true;
            PropertyAnimation { targets: [notCheckedImg, checkedImg]; properties: "opacity"; duration: Constants.AnimationDuration; }
        }
    ]
}
