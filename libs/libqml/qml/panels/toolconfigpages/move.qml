/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Dan Leinir Turthra Jensen <admin@leinir.dk>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.3
import org.krita.sketch 1.0
import org.krita.sketch.components 1.0

Item {
    id: base
    property bool fullView: true;
    height: childrenRect.height;

    ExpandingListView {
        anchors {
            top: parent.top;
            left: parent.left;
            right: parent.right;
            margins: Constants.DefaultMargin;
        }
        expandedHeight: Constants.GridHeight * 3;
        model: ListModel {
            ListElement {
                text: "Current Layer";
            }
            ListElement {
                text: "Touch content";
            }
            ListElement {
                text: "Group";
            }
        }
        onCurrentIndexChanged: if (toolManager.currentTool && toolManager.currentTool.setMoveToolMode) toolManager.currentTool.moveToolMode = currentIndex;
    }
}
