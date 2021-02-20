/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Dan Leinir Turthra Jensen <admin@leinir.dk>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.3
import org.krita.sketch.components 1.0

Item {
    id: base;
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
        horizRadius.value = configuration.readProperty("horizRadius");
        vertRadius.value = configuration.readProperty("vertRadius");
        lockAspect.checked = configuration.readProperty("lockAspect");
    }
    Column {
        anchors.fill: parent;
        RangeInput {
            id: horizRadius;
            width: parent.width;
            placeholder: "Horizontal Radius";
            min: 1; max: 100; decimals: 0;
            value: 1;
            onValueChanged: {
                if(lockAspect.checked && vertRadius.value != value) {
                    vertRadius.value = value;
                }
                setProp("horizRadius", value);
            }
        }
        RangeInput {
            id: vertRadius;
            width: parent.width;
            placeholder: "Vertical Radius";
            min: 1; max: 100; decimals: 0;
            value: 1;
            onValueChanged: {
                if(lockAspect.checked && horizRadius.value != value) {
                    horizRadius.value = value;
                }
                setProp("vertRadius", value);
            }
        }
        CheckBox {
            id: lockAspect
            width: parent.width;
            text: "Lock aspect ratio";
            checked: false;
            onCheckedChanged: {
                if(checked && vertRadius.value != horizRadius.value) {
                    vertRadius.value = horizRadius.value;
                }
                setProp("lockAspect", checked);
            }
        }
    }
}
