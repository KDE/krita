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

import QtQuick 2.3
import org.krita.sketch 1.0
import org.krita.sketch.components 1.0

Page {
    DropShadow {
        anchors {
            top: parent.top;
            left: parent.left;
            right: parent.right;
        }

        height: Constants.GridHeight;
        z: 10;

        Header {
            text: "Settings";

            leftArea: Button {
                width: Constants.GridWidth;
                height: Constants.GridHeight;
                highlightColor: Constants.Theme.HighlightColor;
                image: "./images/svg/icon-back.svg";
                onClicked: pageStack.pop();
            }
        }
    }

    CategorySwitcher {
        anchors.bottom: parent.bottom;
        height: Constants.GridHeight * 7;

        categories: [ { name: "Display", page: displayPage }, { name: "Pressure", page: pressurePage } ];
    }

    Component { id: displayPage; Page {
        Flickable {
            anchors.fill: parent;
            anchors.leftMargin: Constants.DefaultMargin;

            contentWidth: width;
            contentHeight: content.height;

            Flow {
                id: content;
                width: parent.width;

                Label { width: Constants.GridWidth * 6 - Constants.DefaultMargin * 2; text: "Background Color"; }
                Button { width: Constants.GridWidth * 2; color: "#505050"; }
                Label { width: parent.width; text: "Transparency Checkers"; }
                Item { width: Constants.GridWidth; height: Constants.GridHeight }
                Label { width: Constants.GridWidth * 5 - Constants.DefaultMargin * 2; text: "Size"; }
                TextField { width: Constants.GridWidth * 2; text: "32px"; }
                Item { width: Constants.GridWidth; height: Constants.GridHeight }
                Label { width: Constants.GridWidth * 5 - Constants.DefaultMargin * 2; text: "Color"; }
                Button { width: Constants.GridWidth * 2; color: "grey"; }
                Item { width: Constants.GridWidth; height: Constants.GridHeight }
                Label { width: Constants.GridWidth * 5; text: "Move when Scrolling"; }
            }
        }
    } }
    Component { id: pressurePage; Page { Label { anchors.centerIn: parent; text: "Pressure" } } }
}
