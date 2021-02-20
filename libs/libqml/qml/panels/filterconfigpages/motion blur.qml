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
        blurAngle.value = configuration.readProperty("blurAngle");
        blurLength.value = configuration.readProperty("blurLength");
    }
    Column {
        anchors.fill: parent;
        RangeInput {
            id: blurAngle;
            width: parent.width;
            placeholder: "Angle";
            min: 0; max: 360; decimals: 0;
            onValueChanged: setProp("blurAngle", value);
        }
        RangeInput {
            id: blurLength;
            width: parent.width;
            placeholder: "Length";
            min: 0; max: 256; decimals: 0;
            useExponentialValue: true;
            onValueChanged: setProp("blurLength", value);
        }
    }
}
