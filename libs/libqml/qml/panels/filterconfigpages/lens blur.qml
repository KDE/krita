/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Dan Leinir Turthra Jensen <admin@leinir.dk>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.3
import org.krita.sketch.components 1.0

Item {
    id: base
    property QtObject configuration;
    function applyConfigurationChanges() {
        fullFilters.applyConfiguration(configuration);
    }
    function setProp(name, value) {
        if(configuration !== null) {
            configuration.writeProperty(name, value);
            base.applyConfigurationChanges();
        }
    }
    onConfigurationChanged: {
        var irisShapeVal = configuration.readProperty("irisShape");
        irisShape.currentIndex = (irisShapeVal === undefined) ? d.stringToId("Pentagon (5)") : d.stringToId(irisShapeVal);
        var irisRadiusVal = configuration.readProperty("irisRadius");
        irisRadius.value = (irisRadiusVal === undefined) ? 5 : irisRadiusVal;
        var irisRotationVal = configuration.readProperty("irisRotation");
        irisRotation.value = (irisRotationVal === undefined) ? 0 : irisRotationVal;
    }
    QtObject {
        id: d;
        function idToString(id) {
            return irisShape.model.get(id).text;
        }
        function stringToId(name) {
            for(var i = 0; i < irisShape.model.count; i = i + 1) {
                if(name === irisShape.model.get(i).text) {
                    return i;
                }
            }
            return -1;
        }
    }
    Column {
        anchors.fill: parent;
        ExpandingListView {
            id:irisShape;
            width: parent.width;
            model: ListModel {
                ListElement { text: "Triangle" }
                ListElement { text: "Quadrilateral (4)" }
                ListElement { text: "Pentagon (5)" }
                ListElement { text: "Hexagon (6)" }
                ListElement { text: "Heptagon (7)" }
                ListElement { text: "Octagon (8)" }
            }
            onCurrentIndexChanged: setProp("irisShape",  d.idToString(currentIndex));
        }
        RangeInput {
            id: irisRadius;
            width: parent.width;
            placeholder: "Radius";
            min: 0; max: 256; decimals: 0;
            onValueChanged: setProp("irisRadius", value);
        }
        RangeInput {
            id: irisRotation;
            width: parent.width;
            placeholder: "Rotation";
            min: 0; max: 360; decimals: 0;
            onValueChanged: setProp("irisRotation", value);
        }
    }
}
