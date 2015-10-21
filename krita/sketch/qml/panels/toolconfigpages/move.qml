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
