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
    propertyName: i18nc("@label:spinbox", "Letter Spacing");
    propertyType: TextPropertyBase.Character;
    toolTip: i18nc("@info:tooltip",
                   "Letter spacing controls the spacing between visible clusters of characters.");
    searchTerms: i18nc("comma separated search terms for the letter-spacing property, matching is case-insensitive",
                       "letter-spacing, tracking, kerning");
    property alias letterSpacing: letterSpacingSpn.value;
    property alias letterSpacingUnit: letterSpacingUnitCmb.comboBoxUnit;

    onPropertiesUpdated: {
        blockSignals = true;
        letterSpacing = properties.letterSpacing.value * letterSpacingSpn.multiplier;
        letterSpacingUnit = properties.letterSpacing.unitType;
        visible = properties.letterSpacingState !== KoSvgTextPropertiesModel.PropertyUnset;
        blockSignals = false;
    }
    onLetterSpacingChanged: {
        if (!blockSignals) {
            properties.letterSpacing.value = letterSpacing / letterSpacingSpn.multiplier;
        }
    }

    onLetterSpacingUnitChanged: {
        letterSpacingUnitCmb.currentIndex = letterSpacingUnitCmb.indexOfValue(letterSpacingUnit);
        if (!blockSignals) {
            properties.letterSpacing.unitType = letterSpacingUnit;
        }
    }

    onEnableProperty: properties.letterSpacingState = KoSvgTextPropertiesModel.PropertySet;

    RowLayout {
        spacing: columnSpacing;
        width: parent.width;

        RevertPropertyButton {
            revertState: properties.letterSpacingState;
            onClicked: properties.letterSpacingState = KoSvgTextPropertiesModel.PropertyUnset;
        }

            Label {
                text: propertyName;
                elide: Text.ElideRight;
                Layout.fillWidth: true;
                font.italic: properties.letterSpacingState === KoSvgTextPropertiesModel.PropertyTriState;
            }

            DoubleSpinBox {
                id: letterSpacingSpn
                editable: true;
                Layout.fillWidth: true;
                from: -999 * multiplier;
                to: 999 * multiplier;
            }

            UnitComboBox {
                id: letterSpacingUnitCmb
                spinBoxControl: letterSpacingSpn;
                isFontSize: false;
                Layout.fillWidth: true;
                allowPercentage: false; // CSS-Text-4 has percentages for letter-spacing, but so does SVG 1.1, and they both are implemented differently.
            }
    }
}
