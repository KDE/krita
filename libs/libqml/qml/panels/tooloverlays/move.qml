/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Dan Leinir Turthra Jensen <admin@leinir.dk>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.3
import org.krita.sketch 1.0
import org.krita.sketch.components 1.0

Item {
    height: Constants.GridHeight * 1.5 + Constants.DefaultMargin * 2;
    Rectangle {
        anchors {
            top: parent.top;
            bottom: parent.bottom;
            right: parent.right;
        }
        opacity: toolManager.currentTool.moveInProgress ? 1 : 0;
        Behavior on opacity { PropertyAnimation { duration: Constants.AnimationDuration; } }
        width: Constants.GridWidth * 2 + Constants.DefaultMargin * 3;
        color: "#63ffffff";
        border.width: 1;
        border.color: "silver";
        radius: Constants.DefaultMargin;

        Label {
            anchors {
                top: parent.top;
                right: parent.right;
                margins: Constants.DefaultMargin;
            }
            text: "Moving";
        }
        Button {
            anchors {
                left: parent.left;
                bottom: parent.bottom;
                margins: Constants.DefaultMargin;
            }
            text: "Abort";
            textColor: "black";
            color: "#63ffffff";
            border.width: 1;
            border.color: "silver";
            radius: Constants.DefaultMargin;
            onClicked: toolManager.currentTool.requestStrokeCancellation();
        }
        Button {
            anchors {
                right: parent.right;
                bottom: parent.bottom;
                margins: Constants.DefaultMargin;
            }
            text: "Complete";
            textColor: "black";
            color: "#63ffffff";
            border.width: 1;
            border.color: "silver";
            radius: Constants.DefaultMargin;
            onClicked: toolManager.currentTool.requestStrokeEnd();
        }
    }
}
