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

import QtQuick 1.1

TextField {
    property bool enabled: true;
    height: Constants.DefaultFontSize + Constants.DefaultMargin * 4;
    //shadow: false;
    /*border.width: 1;
    border.color: "silver";
    background: "#63ffffff";
    radius: height / 2;*/
    Rectangle {
        id: enabledVisualiser;
        opacity: enabled ? 0 : 0.7;
        anchors.fill: parent;
        color: "black";
    }
}
