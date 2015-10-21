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
import org.krita.sketch.components 1.0

Item {
    height: Constants.GridHeight * 2 + Constants.DefaultMargin * 3;
    Rectangle {
        anchors {
            top: parent.top;
            bottom: parent.bottom;
            right: parent.right;
        }
        opacity: sketchView.selectionManager.havePixelsSelected ? 1 : 0;
        Behavior on opacity { PropertyAnimation { duration: Constants.AnimationDuration; } }
        width: Constants.GridWidth * 2 + Constants.DefaultMargin * 3;
        color: "#63ffffff";
        border.width: 1;
        border.color: "silver";
        radius: Constants.DefaultMargin;

        Button {
            anchors {
                left: parent.left;
                top: parent.top;
                margins: Constants.DefaultMargin;
            }
            text: "Cut";
            textColor: "black";
            color: "#63ffffff";
            border.width: 1;
            border.color: "silver";
            radius: Constants.DefaultMargin;
            onClicked: sketchView.selectionManager.cut();
        }
        Button {
            anchors {
                right: parent.right;
                top: parent.top;
                margins: Constants.DefaultMargin;
            }
            text: "Cut to Layer";
            textColor: "black";
            color: "#63ffffff";
            border.width: 1;
            border.color: "silver";
            radius: Constants.DefaultMargin;
            onClicked: sketchView.selectionManager.cutToNewLayer();
        }
        Button {
            anchors {
                left: parent.left;
                bottom: parent.bottom;
                margins: Constants.DefaultMargin;
            }
            text: "Copy";
            textColor: "black";
            color: "#63ffffff";
            border.width: 1;
            border.color: "silver";
            radius: Constants.DefaultMargin;
            onClicked: sketchView.selectionManager.copy();
        }
        Button {
            anchors {
                right: parent.right;
                bottom: parent.bottom;
                margins: Constants.DefaultMargin;
            }
            text: "Copy Merged";
            textColor: "black";
            color: "#63ffffff";
            border.width: 1;
            border.color: "silver";
            radius: Constants.DefaultMargin;
            onClicked: sketchView.selectionManager.copyMerged();
        }
    }
}
