/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Dan Leinir Turthra Jensen <admin@leinir.dk>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
