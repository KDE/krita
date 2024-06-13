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
    propertyName: i18nc("@title:group", "Word Spacing");
    propertyType: TextPropertyBase.Character;
    toolTip: i18nc("@info:tooltip",
                   "Word spacing controls the size of word-break characters, such as the space character.");
    searchTerms: i18nc("comma separated search terms for the word-spacing property, matching is case-insensitive",
                       "word-spacing, tracking, white space,");
    property alias wordSpacing: wordSpacingSpn.value;
    visible: properties.wordSpacingState !== KoSvgTextPropertiesModel.PropertyUnset;
    property alias wordSpacingUnit: wordSpacingUnitCmb.comboBoxUnit;

    onPropertiesUpdated: {
        blockSignals = true;
        wordSpacing = properties.wordSpacing.value * wordSpacingSpn.multiplier;
        wordSpacingUnit = properties.wordSpacing.unitType;
        visible = properties.wordSpacingState !== KoSvgTextPropertiesModel.PropertyUnset;
        blockSignals = false;
    }
    onWordSpacingChanged: {
        if (!blockSignals) {
            properties.wordSpacing.value = wordSpacing / wordSpacingSpn.multiplier;
        }
    }

    onWordSpacingUnitChanged: {
        wordSpacingUnitCmb.currentIndex = wordSpacingUnitCmb.indexOfValue(wordSpacingUnit);
        if (!blockSignals) {
            properties.wordSpacing.unitType = wordSpacingUnit;
        }
    }

    onEnableProperty: properties.wordSpacingState = KoSvgTextPropertiesModel.PropertySet;

    RowLayout {
        spacing: columnSpacing;
        width: parent.width;

        RevertPropertyButton {
            revertState: properties.wordSpacingState;
            onClicked: properties.wordSpacingState = KoSvgTextPropertiesModel.PropertyUnset;
        }

        Label {
            text: propertyName;
            elide: Text.ElideRight;
            Layout.fillWidth: true;
            font.italic: properties.wordSpacingState === KoSvgTextPropertiesModel.PropertyTriState;
        }

        DoubleSpinBox {
            id: wordSpacingSpn
            editable: true;
            Layout.fillWidth: true;
            from: -999 * multiplier;
            to: 999 * multiplier;
        }

        UnitComboBox {
            id: wordSpacingUnitCmb
            spinBoxControl: wordSpacingSpn;
            isFontSize: false;
            Layout.fillWidth: true;
            allowPercentage: false; // CSS-Text-4 has percentages for word-spacing, but so does SVG 1.1, and they both are implemented differently.
        }
    }
}
