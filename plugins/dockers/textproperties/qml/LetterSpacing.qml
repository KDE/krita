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

    RowLayout {
        spacing: columnSpacing;
        width: parent.width;

        RevertPropertyButton {
            revertEnabled: properties.letterSpacingState === KoSvgTextPropertiesModel.PropertySet;
            onClicked: properties.letterSpacingState = KoSvgTextPropertiesModel.PropertyUnset;
        }

            Label {
                text: propertyName;
                elide: Text.ElideRight;
                Layout.fillWidth: true;
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
            }
    }
}
