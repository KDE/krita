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
        radius.value = configuration.readProperty("radius");
    }
    Column {
        anchors.fill: parent;
        RangeInput {
            id: radius;
            width: parent.width;
            placeholder: "Radius";
            useExponentialValue: true;
            min: 2; max: 100; decimals: 0;
            onValueChanged: {
                setProp("radius", value);
            }
        }
    }
}
