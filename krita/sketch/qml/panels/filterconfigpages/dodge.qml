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
import org.krita.sketch 1.0
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
        var exposureVal = configuration.readProperty("exposure");
        exposure.value = (exposureVal === undefined) ? 0.5 : exposureVal;
        var modeVal = configuration.readProperty("type");
        modeChoice.currentIndex = (modeVal === undefined) ? 1 : modeVal;
    }
    Column {
        anchors.fill: parent;
        RangeInput {
            id: exposure;
            width: parent.width;
            placeholder: "Exposure";
            min: 0; max: 1; decimals: 2;
            onValueChanged: setProp("exposure", value);
        }
        Item {
            width: parent.width;
            height: Constants.DefaultMargin
        }
        Label {
            anchors.leftMargin: Constants.DefaultMargin;
            text: "Mode:"
            font: Settings.theme.font("panelSection");
        }
        ExpandingListView {
            id: modeChoice
            width: parent.width;
            model: ListModel {
                ListElement {
                    text: "Shadows"
                }
                ListElement {
                    text: "Midtones"
                }
                ListElement {
                    text: "Highlights"
                }
            }
            onCurrentIndexChanged: setProp("type", currentIndex);
        }
    }
}
