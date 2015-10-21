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

Item {
    id: base;
    height: toolOverlayContainer.height;

    function toolIDToName(toolID) {
        var names = {
            "KritaShape/KisToolBrush" : "none", // "paint",
            "KritaFill/KisToolFill" : "none", // "fill",
            "KritaFill/KisToolGradient" : "none", // "gradient",
            "KritaTransform/KisToolMove" : "move",
            "KisToolTransform" : "transform",
            "KisToolCrop" : "none", // "crop",
            "KisToolSelectRectangular" : "select",
            "KisToolSelectPolygonal" : "select",
            "KisToolSelectContiguous" : "select",
            "KisToolSelectSimilar" : "select"
        };
        return names[toolID];
    }

    Loader {
        id: toolOverlayContainer;
        width: parent.width;
        height: item.childrenRect.height;
        source: "tooloverlays/none.qml";
        onStatusChanged: {
            if(status === Loader.Error) {
                source = "tooloverlays/none.qml";
            }
        }
    }
    Connections {
        target: toolManager;
        onCurrentToolChanged: {
            if(toolManager.currentTool !== null) {
                toolOverlayContainer.source = "tooloverlays/" + toolIDToName(toolManager.currentTool.toolId()) + ".qml";
            }
            else {
                toolOverlayContainer.source = "tooloverlays/none.qml";
            }
        }
    }
}
