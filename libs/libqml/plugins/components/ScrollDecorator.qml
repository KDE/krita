/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Dan Leinir Turthra Jensen <admin@leinir.dk>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
