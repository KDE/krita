/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Dan Leinir Turthra Jensen <admin@leinir.dk>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.3
import org.krita.sketch 1.0

Item {
    id: base;

    property bool enabled: true;
    property alias placeholder: textField.placeholder;
    property real value: 0;
    property real min: 0;
    property real max: 1000;
    property int decimals: 2;
    property alias useExponentialValue: valueSlider.useExponentialValue;

    height: textField.height + valueSlider.height;

    property alias border: valueSlider.border;
    property alias background: valueSlider.background;

    onMinChanged: d.fixHandle();
    onMaxChanged: d.fixHandle();
    onValueChanged: {
        if (decimals === 0) {
            if (value !== Math.round(value))
            {
                value = Math.round(value);
                return;
            }
        }
        else if (value * Math.pow(10, decimals) !== Math.round(value * Math.pow(10, decimals))) {
            value = Math.round(value * Math.pow(10, decimals)) / Math.pow(10, decimals);
            return;
        }
        if (value < min) {
            value = min;
            return;
        }
        if (value > max) {
            value = max;
            return;
        }
        if (textField.text != value) {
            textField.text = value.toFixed(decimals);
        }
        if (useExponentialValue) {
             if (valueSlider.exponentialValue !== value) {
                 valueSlider.exponentialValue = ( (value - min) / (max - min) ) * 100;
             }
        }
        else {
            if (valueSlider.value !== value) {
                valueSlider.value = ( (value - min) / (max - min) ) * 100;
            }
        }
    }

    PanelTextField {
        id: textField
        anchors {
            top: parent.top;
            left: parent.left;
            right: parent.right;
        }
        onFocusLost: value = text;
        onAccepted: value = text;
        numeric: true;

        border.width: valueSlider.border.width;
        border.color: valueSlider.border.color;
        background: valueSlider.background;
    }
    Slider {
        id: valueSlider;
        anchors {
            top: textField.bottom;
            left: parent.left;
            right: parent.right;
            leftMargin: Constants.DefaultMargin;
            rightMargin: Constants.DefaultMargin;
        }
        highPrecision: true;
        onExponentialValueChanged: {
            if (useExponentialValue) {
                base.value = base.min + ((exponentialValue / 100) * (base.max - base.min))
            }
        }
        onValueChanged: {
            if (!useExponentialValue) {
                base.value = base.min + ((value / 100) * (base.max - base.min));
            }
        }
    }
    QtObject {
        id: d;
        function fixHandle() {
            var currentVal = base.value;
            // Set the value to something it isn't currently
            base.value = base.min;
            base.value = base.max;
            // Set it back to what it was
            base.value = currentVal;
        }
    }
}
