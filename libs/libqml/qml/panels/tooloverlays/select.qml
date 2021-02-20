/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Dan Leinir Turthra Jensen <admin@leinir.dk>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
