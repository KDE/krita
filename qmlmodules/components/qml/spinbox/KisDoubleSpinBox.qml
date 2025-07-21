/*
 *  SPDX-FileCopyrightText: 2024 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Controls 2.15

SpinBox {
    id: root

    property int decimals: 2
    property real dValue: 0.0
    property real dFrom: 0.0
    property real dTo: 100.0
    property real dStepSize: 1.0

    property real factor: Math.pow(10, decimals)

    stepSize: Math.round(dStepSize * factor)
    from: Math.round(dFrom * factor)
    to: Math.round(dTo * factor)

    onValueChanged: root.dValue = root.value / root.factor;

    onDValueChanged: {
        let v = root.dValue;
        if (v > root.dTo) {
            v = root.dTo;
        } else if (v < root.dFrom) {
            v = root.dFrom;
        }
        if (v !== root.dValue) {
            root.dValue = v;
        }
        root.value = Math.round(root.dValue * root.factor);
    }

    onDFromChanged: {
        if (root.dFrom > root.dTo) {
            root.dFrom = root.dTo;
        }
        if (root.dFrom > root.dValue) {
            root.dValue = root.dFrom;
        }
    }

    onDToChanged: {
        if (root.dTo < root.dFrom) {
            root.dTo = root.dFrom;
        }
        if (root.dTo < root.dValue) {
            root.dValue = root.dTo;
        }
    }

    textFromValue: function(value, locale) {
        return Number(value / factor).toLocaleString(locale, 'f', decimals)
    }

    valueFromText: function(text, locale) {
        return Number.fromLocaleString(locale, text) * factor;
    }

    Component.onCompleted: {
        root.value = Math.round(root.dValue * root.factor)

        // The following is to avoid an oddity inside Fusion
        // Where padding is added to ensure a centered text.
        if (root.mirrored) {
            root.rightPadding = 0;
        } else {
            root.leftPadding = 0;
        }
    }
}
