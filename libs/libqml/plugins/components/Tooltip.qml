/* This file is part of the KDE project
 * Copyright (C) 2014 Arjen Hiemstra <ahiemstra@heimr.nl>
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
