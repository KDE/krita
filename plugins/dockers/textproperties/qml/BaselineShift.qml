/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.12
import org.krita.flake.text 1.0

TextPropertyBase {
    propertyName: i18nc("@label", "Baseline-Shift")
    propertyType: TextPropertyBase.Character;
    toolTip: i18nc("@info:tooltip",
                   "Baseline shift allows moving the text away from the baseline, either by predefined super and subscript values, or by a fixed amount.");
    searchTerms: i18nc("comma separated search terms for the baseline-shift property, matching is case-insensitive",
                       "baseline-shift, superscript, subscript");

    property alias baselineShiftValue: baselineShiftSpn.value;
    property alias baselineShiftUnit: baselineShiftUnitCmb.comboBoxUnit;
    property int baselineShiftMode;

    onPropertiesUpdated: {
        blockSignals = true;
        baselineShiftValue = properties.baselineShiftValue.value * baselineShiftSpn.multiplier;
        baselineShiftMode = properties.baselineShiftMode;
        baselineShiftUnit = properties.baselineShiftValue.unitType;
        visible = properties.baselineShiftState !== KoSvgTextPropertiesModel.PropertyUnset
        blockSignals = false;
    }

    onBaselineShiftValueChanged: {
        if (!blockSignals) {
            properties.baselineShiftValue.value = baselineShiftValue / baselineShiftSpn.multiplier;
        }
    }

    onBaselineShiftUnitChanged: {
        baselineShiftUnitCmb.currentIndex = baselineShiftUnitCmb.indexOfValue(baselineShiftUnit);
        if (!blockSignals) {
            properties.baselineShiftValue.unitType = baselineShiftUnit;
        }
    }

    onBaselineShiftModeChanged: {
        baselineShiftCmb.currentIndex = baselineShiftCmb.indexOfValue(baselineShiftMode);
        baselineShiftSpn.enabled = baselineShiftMode == 3;
        if (!blockSignals) {
            properties.baselineShiftMode = baselineShiftMode;
        }
    }

    onEnableProperty: properties.baselineShiftState = KoSvgTextPropertiesModel.PropertySet;

    Component.onCompleted: {
        mainWindow.connectAutoEnabler(baselineShiftSpnArea);
    }
    GridLayout {
        columns: 3;
        columnSpacing: columnSpacing;
        width: parent.width;

        RevertPropertyButton {
            revertState: properties.baselineShiftState;
            onClicked: properties.baselineShiftState = KoSvgTextPropertiesModel.PropertyUnset;
        }

        Label {
            text: propertyName;
            elide: Text.ElideRight;
            Layout.fillWidth: true;
            Layout.columnSpan: 2;
            font.italic: properties.baselineShiftState === KoSvgTextPropertiesModel.PropertyTriState;
        }


        Item {
            width: firstColumnWidth;
            height: 1;
        }

        ComboBox {
            Layout.columnSpan: 2;
            Layout.fillWidth: true;
            model: [
                { text: i18nc("@label:inlistbox", "None"), value: KoSvgText.ShiftNone},
                { text: i18nc("@label:inlistbox", "Length"), value: KoSvgText.ShiftLengthPercentage},
                { text: i18nc("@label:inlistbox", "Super"), value: KoSvgText.ShiftSuper},
                { text: i18nc("@label:inlistbox", "Sub"), value: KoSvgText.ShiftSub}
            ]
            id: baselineShiftCmb;
            textRole: "text";
            valueRole: "value";
            onActivated: baselineShiftMode = currentValue;
            wheelEnabled: true;
        }

        Item {
            width: firstColumnWidth;
            height: 1;
        }

        MouseArea {
            id: baselineShiftSpnArea;
            function autoEnable() {
                baselineShiftMode = 3;
            }
            Layout.fillWidth: true;
            Layout.fillHeight: true;
            DoubleSpinBox {
                id: baselineShiftSpn
                width: parent.width;
                enabled: baselineShiftMode == 3;
                from: -999 * multiplier;
                to: 999 * multiplier;
            }
        }

        UnitComboBox {
            id: baselineShiftUnitCmb;
            spinBoxControl: baselineShiftSpn;
        }
    }
}
