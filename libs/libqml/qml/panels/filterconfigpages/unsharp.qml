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
        amount.value = configuration.readProperty("amount");
        halfSize.value = configuration.readProperty("halfSize");
        threshold.value = configuration.readProperty("threshold");
    }
    Column {
        anchors.fill: parent;
        RangeInput {
            id: amount;
            width: parent.width;
            placeholder: "Amount";
            min: 0; max: 1; decimals: 2;
            value: 0;
            onValueChanged: {
                setProp("amount", value);
            }
        }
        RangeInput {
            id: halfSize;
            width: parent.width;
            placeholder: "Size";
            min: 0; max: 100; decimals: 0;
            value: 0;
            onValueChanged: {
                setProp("halfSize", value);
            }
        }
        RangeInput {
            id: threshold;
            width: parent.width;
            placeholder: "Threshold";
            min: 0; max: 255; decimals: 0;
            value: 0;
            onValueChanged: {
                setProp("threshold", value);
            }
        }
    }
}
