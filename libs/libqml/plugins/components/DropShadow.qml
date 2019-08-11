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

Item {
    id: base;
    property real size: Constants.DefaultMargin * 0.666;

    Image {
        anchors.right: parent.left;
        anchors.bottom: parent.top;

        width: parent.size;
        height: parent.size;

        opacity: 0.5;

        source: "../images/shadow-tl.png";

        fillMode: Image.Stretch;
    }

    Image {
        anchors.horizontalCenter: parent.horizontalCenter;
        anchors.bottom: parent.top;

        width: parent.width;
        height: parent.size;

        opacity: 0.5;

        source: "../images/shadow-tc.png";
        fillMode: Image.Stretch;
    }

    Image {
        anchors.left: parent.right;
        anchors.bottom: parent.top;

        width: parent.size;
        height: parent.size;

        opacity: 0.5;

        source: "../images/shadow-tr.png";
        fillMode: Image.Stretch;
    }

    Image {
        anchors.left: parent.right;
        anchors.verticalCenter: parent.verticalCenter;

        width: parent.size;
        height: parent.height;

        opacity: 0.5;

        source: "../images/shadow-rc.png";
        fillMode: Image.Stretch;
    }

    Image {
        anchors.left: parent.right;
        anchors.top: parent.bottom;

        width: parent.size;
        height: parent.size;

        opacity: 0.5;

        source: "../images/shadow-br.png";
        fillMode: Image.Stretch;
    }

    Image {
        anchors.horizontalCenter: parent.horizontalCenter;
        anchors.top: parent.bottom;

        width: parent.width;
        height: parent.size;

        opacity: 0.5;

        source: "../images/shadow-bc.png";
        fillMode: Image.Stretch;
    }

    Image {
        anchors.right: parent.left;
        anchors.top: parent.bottom;

        width: parent.size;
        height: parent.size;

        opacity: 0.5;

        source: "../images/shadow-bl.png";
        fillMode: Image.Stretch;
    }

    Image {
        anchors.right: parent.left;
        anchors.verticalCenter: parent.verticalCenter;

        width: parent.size;
        height: parent.height;

        opacity: 0.5;

        source: "../images/shadow-lc.png";
        fillMode: Image.Stretch;
    }
}
