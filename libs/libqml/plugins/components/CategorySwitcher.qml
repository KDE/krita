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

Row {
    id: base;
    property variant categories;
    property alias currentPage: pageStack.currentPage;

    ListView {
        id: view;
        width: Constants.GridWidth * 4 - 1;
        height: parent.height;
        clip: true;

        delegate: Item {
            width: parent.width;
            height: Constants.GridHeight;
            Label {
                anchors {
                    left: parent.left;
                    leftMargin: Constants.DefaultMargin;
                    verticalCenter: parent.verticalCenter;
                }
                text: modelData.name;
            }
            MouseArea {
                anchors.fill: parent;
                onClicked: {
                    if ( view.currentIndex != index ) {
                        view.currentIndex = index;
                        pageStack.replace( modelData.page );
                    }
                }
            }
        }

        highlight: DropShadow {
            width: parent.width;
            height: Constants.GridHeight;

            Rectangle { anchors.fill: parent; color: Constants.Theme.HighlightColor; }
        }

        model: base.categories;
    }

    Divider { height: parent.height; }

    PageStack {
        id: pageStack;

        width: Constants.GridWidth * 8 - 1;
        height: parent.height;
        clip: true;

        initialPage: categories[0].page;
    }
}
