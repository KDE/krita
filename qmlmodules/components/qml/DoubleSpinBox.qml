/*
 *  SPDX-FileCopyrightText: 2024 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Controls 2.15

/*
    \qmltype DoubleSpinBox
    \inherits SpinBox
    \brief A SpinBox that allows for floating point input.

    Biggest difference between this and SpinBox is that the value and range
    (from and to) are prefixed with "d", and *must* be used instead of from/to/value.
    This is because those other values are reserved to be used by spinbox itself,
    and must be multiplied by the desired factor.

    \qml
        DoubleParseSpinBox {
            dFrom: 0
            dTo: 100.0
            dValue: 50.0
            dStepSize: 5.0;
            decimals: 2;
        }
    \endqml

 */
SpinBox {
    id: root

    /*
        \qmlproperty decimals
        The amount of decimals after the floating point to show. In practice
        this will affect the factor used to convert for the value inside the SpinBox.
        By default this is 2.
    */
    property int decimals: 2
    /*
        \qmlproperty dValue
        The floating point value. This must used instead of "value".
    */
    property real dValue: 0.0
    /*
        \qmlproperty dFrom
        The floating point lower end of the spinbox range. This must used instead of "from".
    */
    property real dFrom: 0.0
    /*
        \qmlproperty dTo
        The floating point upper end of the spinbox range. This must used instead of "to".
    */
    property real dTo: 100.0
    /*
        \qmlproperty dStepSize
        The floating point increment size of the spinbox range. This must used instead of "stepsize".
    */
    property real dStepSize: 1.0

    /*
        \qmlproperty factor
        The multiplier to multiply the d-values to spinbox internal values.
        It uses the decimals property as a power to 10 to achieve that.
    */
    readonly property real factor: Math.pow(10, decimals)

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
