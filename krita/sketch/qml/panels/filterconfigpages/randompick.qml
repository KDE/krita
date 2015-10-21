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
