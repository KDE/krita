/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2014 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.3
import org.krita.sketch 1.0

Item {
    id: base;
    property alias text: label.text;

    Rectangle {
        id: body;

        width: label.width + Constants.DefaultMargin * 2;
        height: label.height + Constants.DefaultMargin * 2;
        radius: Constants.DefaultMargin;

        opacity: 0;
        Behavior on opacity { NumberAnimation { duration: 100 } }

        Label {
            id: label;
            x: Constants.DefaultMargin;
            y: Constants.DefaultMargin;
        }

        Shadow {
            anchors.top: parent.bottom;
            anchors.horizontalCenter: parent.horizontalCenter;
            width: label.width;
        }
    }

    function show(x, y) {
        body.opacity = 1;
        body.x = x - (body.width / 2);
        body.y = y - body.height;
    }

    function hide() {
        body.opacity = 0;
    }
}
