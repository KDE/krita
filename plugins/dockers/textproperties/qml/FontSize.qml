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
    propertyName: i18nc("@label:spinbox", "Font Size");
    propertyType: TextPropertyBase.Character;
    toolTip: i18nc("@info:tooltip",
                   "Font size allows setting the size of the characters.");
    searchTerms: i18nc("comma separated search terms for the fontsize property, matching is case-insensitive",
                       "size, small, big, medium");

    property alias fontSize: fontSizeSpn.value;
    property alias fontSizeUnit: fontSizeUnitCmb.comboBoxUnit;

    onPropertiesUpdated: {
        blockSignals = true;
        fontSize = properties.fontSize.value * fontSizeSpn.multiplier;
        fontSizeUnit = properties.fontSize.unitType;
        visible = properties.fontSizeState !== KoSvgTextPropertiesModel.PropertyUnset;
        blockSignals = false;
    }
    onFontSizeChanged: {
        if (!blockSignals) {
            properties.fontSize.value = fontSize / fontSizeSpn.multiplier;
        }
    }
    onFontSizeUnitChanged: {
        fontSizeUnitCmb.currentIndex = fontSizeUnitCmb.indexOfValue(fontSizeUnit);
        if (!blockSignals) {
            properties.fontSize.unitType = fontSizeUnitCmb.comboBoxUnit;
        }
    }

    onEnableProperty: properties.fontSizeState = KoSvgTextPropertiesModel.PropertySet;

    RowLayout {
        spacing: columnSpacing;
        width: parent.width;

        RevertPropertyButton {
            revertState: properties.fontSizeState;
            onClicked: properties.fontSizeState = KoSvgTextPropertiesModel.PropertyUnset;
        }

        Label {
            text: propertyName;
            elide: Text.ElideRight;
            Layout.fillWidth: true;
            font.italic: properties.fontSizeState === KoSvgTextPropertiesModel.PropertyTriState;
        }

        DoubleSpinBox {
            id: fontSizeSpn;
            editable: true;
            Layout.fillWidth: true;

            from: 0;
            to: 999 * multiplier;
            stepSize: 100;
        }

        UnitComboBox {
            id: fontSizeUnitCmb;
            spinBoxControl: fontSizeSpn;
            isFontSize: true;
            Layout.fillWidth: true;
        }
    }
}
