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
        if (configuration !== null) {
            configuration.writeProperty(name, value);
            base.applyConfigurationChanges();
        }
    }
    onConfigurationChanged: {
        var typeVal = configuration.readProperty("type");
        typeChoice.currentIndex = (typeVal === undefined) ? 1 : typeVal;
        var hueVal = configuration.readProperty("h");
        hue.value = (hueVal === undefined) ? 0 : hueVal;
        var saturationVal = configuration.readProperty("s");
        saturation.value = (saturationVal === undefined) ? 0 : saturationVal;
        var valVal = configuration.readProperty("v");
        val.value = (valVal === undefined) ? 0 : valVal;
        var colorizeVal = configuration.readProperty("colorize");
        colorize.checked = (colorizeVal === undefined) ? false : colorizeVal;
    }
    Column {
        anchors.fill: parent;
        ExpandingListView {
            id:typeChoice;
            width: parent.width;
            model: ListModel {
                ListElement { text: "Value (HSV)" }
                ListElement { text: "Lightness (HSL)" }
            }
            onCurrentIndexChanged: setProp("type", currentIndex);
        }
        RangeInput {
            id: hue;
            width: parent.width;
            placeholder: "Hue";
            min: -180; max: 180; decimals: 0;
            value: 1
            onValueChanged: setProp("h", value);
        }
        RangeInput {
            id: saturation;
            width: parent.width;
            placeholder: "Saturation";
            min: -100; max: 100; decimals: 0;
            value: 1
            onValueChanged: setProp("s", value);
        }
        RangeInput {
            id: val;
            width: parent.width;
            placeholder: (typeChoice.currentIndex === 0) ? "Value" : "Lightness";
            min: -100; max: 100; decimals: 0;
            value: 1
            onValueChanged: setProp("v", value);
        }
        CheckBox {
            id: colorize
            width: parent.width;
            text: "Colorize";
            onCheckedChanged: {
                if (checked) {
                    hue.min = 0;
                    hue.max = 360;
                    saturation.min = 0;
                    saturation.value = 50;
                    typeChoice.currentIndex = 1;
                }
                else {
                    hue.min = -180;
                    hue.max = 180;
                    saturation.min = -100;
                }
                setProp("colorize", checked);
            }
        }
    }
}
