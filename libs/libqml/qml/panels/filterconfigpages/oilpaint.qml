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
        brushSize.value = configuration.readProperty("brushSize");
        smooth.value = configuration.readProperty("smooth");
    }
    Column {
        anchors.fill: parent;
        RangeInput {
            id: brushSize;
            width: parent.width;
            placeholder: "Brush size";
            min: 1; max: 5; decimals: 0;
            onValueChanged: setProp("halfWidth", value);
        }
        RangeInput {
            id: smooth;
            width: parent.width;
            placeholder: "Smooth";
            min: 10; max: 255; decimals: 0;
            onValueChanged: setProp("smooth", value);
        }
    }
}
