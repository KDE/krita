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
