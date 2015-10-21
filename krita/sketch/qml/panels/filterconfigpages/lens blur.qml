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
