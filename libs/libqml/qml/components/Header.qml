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

Image {
    id: base;

    property alias text: title.text;
    property alias leftArea: left.children;
    property alias rightArea: right.children;

    property alias background: base.source;

    height: Constants.GridHeight * 2;
    width: parent.width;

    source: Settings.theme.image("header_krita_sketch.png");
    smooth: true;

    Row {
        id: left;

        anchors {
            left: parent.left;
            leftMargin: Constants.GridWidth * 0.5;
            top: parent.top;
            topMargin: Constants.GridHeight * 0.5;
            bottom: parent.bottom;
            bottomMargin: Constants.GridHeight * 0.5;
        }
    }

    Label {
        id: title;
        anchors.centerIn: parent;

        font: Settings.theme.font("pageHeader");
        color: Settings.theme.color("components/header");
    }

    Row {
        id: right;

        anchors {
            right: parent.right;
            rightMargin: Constants.GridWidth * 0.5;
            top: parent.top;
            topMargin: Constants.GridHeight * 0.5;
            bottom: parent.bottom;
            bottomMargin: Constants.GridHeight * 0.5;
        }
    }
}
