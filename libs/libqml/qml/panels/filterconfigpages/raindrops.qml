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
        dropsize.value = configuration.readProperty("dropsize");
        dropletCount.value = configuration.readProperty("number");
        fishEyes.value = configuration.readProperty("fishEyes");
    }
    Column {
        anchors.fill: parent;
        RangeInput {
            id: dropsize;
            width: parent.width;
            placeholder: "Drop Size";
            min: 1; max: 200; decimals: 0;
            useExponentialValue: true;
            onValueChanged: setProp("dropsize", value);
        }
        RangeInput {
            id: dropletCount;
            width: parent.width;
            placeholder: "Number";
            min: 1; max: 500; decimals: 0;
            useExponentialValue: true;
            onValueChanged: setProp("number", value);
        }
        RangeInput {
            id: fishEyes;
            width: parent.width;
            placeholder: "Fish Eyes";
            min: 1; max: 100; decimals: 0;
            useExponentialValue: true;
            onValueChanged: setProp("fishEyes", value);
        }
    }
}
