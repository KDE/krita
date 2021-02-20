/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Dan Leinir Turthra Jensen <admin@leinir.dk>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
