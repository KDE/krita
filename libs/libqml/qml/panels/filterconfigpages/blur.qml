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
        horizRadius.value = configuration.readProperty("halfWidth");
        vertRadius.value = configuration.readProperty("halfHeight");
        var lockAspectVal = configuration.readProperty("lockAspect");
        lockAspect.checked = (lockAspectVal === undefined) ? true : lockAspectVal;
        var rotateVal = configuration.readProperty("rotate");
        rotate.value = (rotateVal === undefined || rotateVal === null) ? 1 : configuration.readProperty("rotate");
        strength.value = configuration.readProperty("strength");
        blurShape.currentIndex = configuration.readProperty("shape");
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
                if (lockAspect.checked && vertRadius.value != value) {
                    vertRadius.value = value;
                }
                setProp("halfWidth", value);
            }
        }
        RangeInput {
            id: vertRadius;
            width: parent.width;
            placeholder: "Vertical Radius";
            min: 1; max: 100; decimals: 0;
            value: 1;
            onValueChanged: {
                if (lockAspect.checked && horizRadius.value != value) {
                    horizRadius.value = value;
                }
                setProp("halfHeight", value);
            }
        }
        CheckBox {
            id: lockAspect
            width: parent.width;
            text: "Lock aspect ratio";
            checked: false;
            onCheckedChanged: {
                if (checked && vertRadius.value != horizRadius.value) {
                    vertRadius.value = horizRadius.value;
                }
                setProp("lockAspect", checked);
            }
        }
        RangeInput {
            id: rotate;
            width: parent.width;
            placeholder: "Angle";
            min: 0; max: 360; decimals: 0;
            value: 1;
            onValueChanged: setProp("rotate", value);
        }
        RangeInput {
            id: strength;
            width: parent.width;
            placeholder: "Strength";
            min: 0; max: 100; decimals: 0;
            value: 100;
            onValueChanged: setProp("strength", value);
        }
        Item {
            width: parent.width;
            height: Constants.DefaultMargin
        }
        Label {
            anchors.leftMargin: Constants.DefaultMargin;
            text: "Shape:"
            font: Settings.theme.font("panelSection");
        }
        ExpandingListView {
            id: blurShape
            width: parent.width;
            model: ListModel {
                ListElement {
                    text: "Circle"
                }
                ListElement {
                    text: "Rectangle"
                }
            }
            onCurrentIndexChanged: setProp("shape", currentIndex);
        }
    }
}
