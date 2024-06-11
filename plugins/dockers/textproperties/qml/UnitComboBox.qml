/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Controls 2.15
import org.krita.flake.text 1.0

ComboBox {
    property QtObject spinBoxControl;
    property bool isFontSize: false;
    property int comboBoxUnit;
    wheelEnabled: true;

    model: [
        {text: i18nc("@label:inlistbox", "Pt"), value: 0},
        {text: i18nc("@label:inlistbox", "Em"), value: 2},
        {text: i18nc("@label:inlistbox", "Ex"), value: 3},
        {text: i18nc("@label:inlistbox", "%"), value: 1}
    ]
    textRole: "text";
    valueRole: "value";
    onActivated: {
        var currentValueInPt = 0;
        if (comboBoxUnit === 0) {
            currentValueInPt = spinBoxControl.value;
        } else if (comboBoxUnit === 2) {
            currentValueInPt = spinBoxControl.value * properties.resolvedFontSize(isFontSize);
        } else if (comboBoxUnit === 3) {
            currentValueInPt = spinBoxControl.value * properties.resolvedXHeight(isFontSize);
        } else if (comboBoxUnit === 1) {
            currentValueInPt = (spinBoxControl.value / 100) * properties.resolvedFontSize(isFontSize);
        }

        var newValue = 0;
        if (currentValue === 0) {
            newValue = currentValueInPt
        } else if (currentValue === 2) {
            newValue = currentValueInPt / properties.resolvedFontSize(isFontSize);
        } else if (currentValue === 3) {
            newValue = currentValueInPt / properties.resolvedXHeight(isFontSize);
        } else if (currentValue === 1) {
            newValue = (currentValueInPt / properties.resolvedFontSize(isFontSize)) * 100;
        }
        comboBoxUnit = currentValue;
        spinBoxControl.value = newValue;
    }
}
