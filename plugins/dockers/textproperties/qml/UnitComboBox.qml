/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Controls 2.15
import org.krita.flake.text 1.0

SqueezedComboBox {
    property QtObject spinBoxControl;
    property bool isFontSize: false;
    property int comboBoxUnit;
    property bool allowPercentage: true;
    wheelEnabled: true;

    property int minimumUnitBoxWidth: height+indicator.width;

    property string ptString: i18nc("@label:inlistbox", "Pt");
    property string emString: i18nc("@label:inlistbox", "Em");
    property string exString: i18nc("@label:inlistbox", "Ex");
    property string chString: i18nc("@label:inlistbox", "Ch");
    property string capString: i18nc("@label:inlistbox", "Cap");
    property string icString: i18nc("@label:inlistbox", "Ic");
    property string lhString: i18nc("@label:inlistbox", "Lh");
    property string prcString: i18nc("@label:inlistbox", "%");

    Component.onCompleted: {
        if (!allowPercentage) {
            // Not great, but listmodels don't seem to allow adding i18n strings as properties, because they are in functions?
            model = [
                        {text: ptString, value: 0},
                        {text: emString, value: 2},
                        {text: exString, value: 3},
                        {text: capString, value: 4},
                        {text: chString, value: 5},
                        {text: icString, value: 6},
                        {text: lhString, value: 7}
                    ];
        }
    }

    model: [
        {text: ptString, value: 0},
        {text: emString, value: 2},
        {text: exString, value: 3},
        {text: capString, value: 4},
        {text: chString, value: 5},
        {text: icString, value: 6},
        {text: lhString, value: 7},
        {text: prcString, value: 1}
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
