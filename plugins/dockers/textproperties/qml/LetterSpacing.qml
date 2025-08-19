/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.12
import org.krita.flake.text 1.0
import org.krita.components 1.0 as Kis

TextPropertyBase {
    id: letterSpacingBase;
    propertyTitle: i18nc("@label:spinbox", "Letter Spacing");
    propertyName: "letter-spacing";
    propertyType: TextPropertyConfigModel.Character;
    toolTip: i18nc("@info:tooltip",
                   "Letter spacing controls the spacing between visible clusters of characters.");
    searchTerms: i18nc("comma separated search terms for the letter-spacing property, matching is case-insensitive",
                       "letter-spacing, tracking, kerning");
    property alias letterSpacing: letterSpacingUnitCmb.dataValue;
    property alias letterSpacingUnit: letterSpacingUnitCmb.dataUnit;

    Connections {
        target: properties;
        function onLetterSpacingChanged() {
            updateLetterSpacing();
            updateVisibility();
        }
        // Fontsize and lineheight affect the metrics.
        function onFontSizeChanged() {
            updateUnits();
        }
        function onFontFamiliesChanged() {
            updateUnits();
        }
        function onLineHeightChanged() {
            updateUnits();
        }

        function onLetterSpacingStateChanged() {
            updateVisibility();
        }
    }
    onPropertiesChanged: {
        updateUnits();
        updateLetterSpacing();
        updateVisibility();
    }

    function updateUnits() {
        blockSignals = true;
        letterSpacingUnitCmb.setTextProperties(properties);
        blockSignals = false;
    }

    function updateLetterSpacing() {
        if (!letterSpacingSpn.isDragging) {
            blockSignals = true;
            letterSpacingUnitCmb.setDataValueAndUnit(properties.letterSpacing.value, properties.letterSpacing.unitType);
            blockSignals = false;
        }
    }

    function updateVisibility() {
        propertyState = [properties.letterSpacingState];
        setVisibleFromProperty();
    }

    onLetterSpacingChanged: {
        if (!blockSignals) {
            properties.letterSpacing.value = letterSpacing;
        }
    }

    onLetterSpacingUnitChanged: {
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


        Kis.DoubleSliderSpinBox {
            id: letterSpacingSpn;
            prefix: propertyTitle+ ": "
            Layout.fillWidth: true;

            dFrom: -99;
            dTo: 99;
            softDFrom: -10;
            softDTo: 10;
            dStepSize: 0.1;
            softRangeActive: true;

            onDValueChanged: letterSpacingUnitCmb.userValue = dValue;
            blockUpdateSignalOnDrag: true;
        }

        UnitComboBox {
            id: letterSpacingUnitCmb
            spinBoxControl: letterSpacingSpn;
            isFontSize: false;
            dpi:letterSpacingBase.dpi;
            onUserValueChanged: letterSpacingSpn.dValue = userValue;
            Layout.preferredWidth: minimumUnitBoxWidth;
            Layout.maximumWidth: implicitWidth;
            allowPercentage: false; // CSS-Text-4 has percentages for letter-spacing, but so does SVG 1.1, and they both are implemented differently.
        }
    }
}
