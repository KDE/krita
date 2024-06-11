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
    propertyName: i18nc("@label", "Line Height");
    propertyType: TextPropertyBase.Character;
    toolTip: i18nc("@info:tooltip",
                   "Line Height controls the line height used for the range of text.");
    searchTerms: i18nc("comma separated search terms for the line-height property, matching is case-insensitive",
                       "line-height, line-spacing");
    property alias isNormal: lineHeightNormalCbx.checked;
    property alias lineHeight: lineHeightSpn.value;
    property int lineHeightUnit: LineHeightModel.Lines;

    onPropertiesUpdated: {
        blockSignals = true;
        isNormal = properties.lineHeight.isNormal;
        lineHeight = properties.lineHeight.value * lineHeightSpn.multiplier;
        lineHeightUnit = properties.lineHeight.unit;
        visible = properties.lineHeightState !== KoSvgTextPropertiesModel.PropertyUnset;
        blockSignals = false;
    }

    onLineHeightUnitChanged: {
        lineHeightUnitCmb.currentIndex = lineHeightUnitCmb.indexOfValue(lineHeightUnit)
        if (!blockSignals) {
            properties.lineHeight.unit = lineHeightUnit
        }
    }
    onIsNormalChanged: {
        if (!blockSignals) {
            properties.lineHeight.isNormal = isNormal;
        }
    }

    onLineHeightChanged: {
        if (!blockSignals) {
            properties.lineHeight.value = lineHeight / lineHeightSpn.multiplier;
        }
    }

    onEnableProperty: properties.lineHeightState = KoSvgTextPropertiesModel.PropertySet;

    Component.onCompleted: {
        mainWindow.connectAutoEnabler(lineHeightSpnArea);
    }

    GridLayout {
        columns: 3;
        columnSpacing: columnSpacing;
        width: parent.width;

        RevertPropertyButton {
            revertState: properties.lineHeightState;
            onClicked: properties.lineHeightState = KoSvgTextPropertiesModel.PropertyUnset;
        }

        Label {
            text: propertyName;
            elide: Text.ElideRight;
            Layout.fillWidth: true;
            font.italic: properties.lineHeightState === KoSvgTextPropertiesModel.PropertyTriState;
        }

        CheckBox {
            text: i18nc("@option:check", "Normal")
            id: lineHeightNormalCbx;
            onCheckedChanged: {
                lineHeightSpn.enabled = !checked;
                lineHeightUnitCmb.enabled = !checked;
            }
        }
        Item {
            width: firstColumnWidth;
            height: 1;
        }
        MouseArea {
            id: lineHeightSpnArea;
            function autoEnable() {
                lineHeightNormalCbx.checked = false;
            }
            Layout.fillWidth: true;
            Layout.fillHeight: true;
            DoubleSpinBox {
                id: lineHeightSpn
                width: parent.width;
                enabled: !lineHeightNormalCbx.checked;
                from: 0;
                to: 999 * multiplier;
            }
        }

        ComboBox {
            id: lineHeightUnitCmb;
            property QtObject spinBoxControl: lineHeightSpn;
            model: [
                { text: i18nc("@label:inlistbox", "Pt"), value: LineHeightModel.Absolute},
                { text: i18nc("@label:inlistbox", "Em"), value: LineHeightModel.Em},
                { text: i18nc("@label:inlistbox", "Ex"), value: LineHeightModel.Ex},
                { text: i18nc("@label:inlistbox", "%"), value: LineHeightModel.Percentage},
                { text: i18nc("@label:inlistbox", "Lines"), value: LineHeightModel.Lines},
            ]
            textRole: "text";
            valueRole: "value";
            enabled: !lineHeightNormalCbx.checked;
            wheelEnabled: true;

            onActivated: {
                var currentValueInPt = 0;
                if (lineHeightUnit === LineHeightModel.Absolute) {
                    currentValueInPt = spinBoxControl.value;
                } else if (lineHeightUnit === LineHeightModel.Ex) {
                    currentValueInPt = spinBoxControl.value * properties.resolvedXHeight(false);
                } else if (lineHeightUnit === LineHeightModel.Percentage) {
                    currentValueInPt = (spinBoxControl.value / 100) * properties.resolvedFontSize(false);
                } else { // EM, Lines
                    currentValueInPt = spinBoxControl.value * properties.resolvedFontSize(false);
                }

                var newValue = 0;
                if (currentValue === LineHeightModel.Absolute) {
                    newValue = currentValueInPt
                } else if (currentValue === LineHeightModel.Ex) {
                    newValue = currentValueInPt / properties.resolvedXHeight(false);
                } else if (currentValue === LineHeightModel.Percentage) {
                    newValue = (currentValueInPt / properties.resolvedFontSize(false)) * 100;
                } else { // Em, Lines
                    newValue = currentValueInPt / properties.resolvedFontSize(false);
                }
                lineHeightUnit = currentValue;
                spinBoxControl.value = newValue;
            }
        }
    }
}
