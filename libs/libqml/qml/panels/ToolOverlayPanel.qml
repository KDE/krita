/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Dan Leinir Turthra Jensen <admin@leinir.dk>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
