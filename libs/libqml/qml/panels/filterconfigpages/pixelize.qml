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
        if (configuration !== null) {
            configuration.writeProperty(name, value);
            base.applyConfigurationChanges();
        }
    }
    onConfigurationChanged: {
        pixelWidth.value = configuration.readProperty("pixelWidth");
        pixelHeight.value = configuration.readProperty("pixelHeight");
    }
    Column {
        anchors.fill: parent;
        RangeInput {
            id: pixelWidth;
            width: parent.width;
            placeholder: "Pixel Width";
            min: 1; max: 100; decimals: 0;
            value: 10;
            onValueChanged: setProp("pixelWidth", value);
        }
        RangeInput {
            id: pixelHeight;
            width: parent.width;
            placeholder: "Pixel Height";
            min: 1; max: 100; decimals: 0;
            value: 10;
            onValueChanged: setProp("pixelHeight", value);
        }
    }
}
