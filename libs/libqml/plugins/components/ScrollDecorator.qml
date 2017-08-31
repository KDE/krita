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
    anchors.fill: parent;
    property Flickable flickableItem: parent;

    Rectangle {
        id: verticalDecorator;
        anchors {
            right: parent.right;
            rightMargin: Constants.DefaultMargin;
        }
        visible: (flickableItem.contentHeight > flickableItem.height);
        color: Settings.theme.color("components/scrollDecorator/base");
        border.width: 1;
        border.color: Settings.theme.color("components/scrollDecorator/border");
        radius: Constants.DefaultMargin / 2;
        width: Constants.DefaultMargin;
        height: (flickableItem.height * (flickableItem.height / flickableItem.contentHeight)) - Constants.DefaultMargin * 2;
        opacity: flickableItem.moving ? 0.5 : 0.2;
        Behavior on opacity { PropertyAnimation { duration: Constants.AnimationDuration; } }
        y: (flickableItem.contentY * (flickableItem.height / flickableItem.contentHeight)) + Constants.DefaultMargin;
    }
    Rectangle {
        id: horizontalDecorator;
        anchors {
            bottom: parent.bottom;
            bottomMargin: Constants.DefaultMargin;
        }
        visible: (flickableItem.contentWidth > flickableItem.width);
        color: Settings.theme.color("components/scrollDecorator/base");
        border.width: 1;
        border.color: Settings.theme.color("components/scrollDecorator/border");
        radius: Constants.DefaultMargin / 2;
        height: Constants.DefaultMargin;
        width: (flickableItem.width * (flickableItem.width / flickableItem.contentWidth)) - Constants.DefaultMargin * 2;
        opacity: flickableItem.moving ? 0.5 : 0.2;
        Behavior on opacity { PropertyAnimation { duration: Constants.AnimationDuration; } }
        x: (flickableItem.contentX * (flickableItem.width / flickableItem.contentWidth)) + Constants.DefaultMargin;
    }
}
