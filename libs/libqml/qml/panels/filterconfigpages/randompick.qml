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
        level.value = configuration.readProperty("level");
        windowsize.value = configuration.readProperty("windowsize");
        opacity.value = configuration.readProperty("opacity");
    }
    Column {
        anchors.fill: parent;
        RangeInput {
            id: level;
            width: parent.width;
            placeholder: "Level";
            min: 0; max: 99; decimals: 0;
            onValueChanged: setProp("level", value);
        }
        RangeInput {
            id: windowsize;
            width: parent.width;
            placeholder: "Window Size";
            min: 0; max: 99; decimals: 0;
            onValueChanged: setProp("windowsize", value);
        }
        RangeInput {
            id: opacity;
            width: parent.width;
            placeholder: "Opacity";
            min: 1; max: 100; decimals: 0;
            onValueChanged: setProp("opacity", value);
        }
    }
}
