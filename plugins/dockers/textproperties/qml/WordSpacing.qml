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
    id: wordSpacingBase;
    propertyTitle: i18nc("@title:group", "Word Spacing");
    propertyName: "word-spacing";
    propertyType: TextPropertyConfigModel.Character;
    toolTip: i18nc("@info:tooltip",
                   "Word spacing controls the size of word-break characters, such as the space character.");
    searchTerms: i18nc("comma separated search terms for the word-spacing property, matching is case-insensitive",
                       "word-spacing, tracking, white space,");
    property alias wordSpacing: wordSpacingUnitCmb.dataValue;
    property alias wordSpacingUnit: wordSpacingUnitCmb.dataUnit;

    Connections {
        target: properties;
        function onWordSpacingChanged() {
            updateWordSpacing();
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

        function onWordSpacingStateChanged() {
            updateVisibility();
        }
    }
    onPropertiesChanged: {
        updateUnits();
        updateWordSpacing();
        updateVisibility();
    }

    function updateUnits() {
        blockSignals = true;
        wordSpacingUnitCmb.setTextProperties(properties);
        blockSignals = false;
    }

    function updateWordSpacing() {
        if (!wordSpacingSpn.isDragging) {
            blockSignals = true;
            wordSpacingUnitCmb.setDataValueAndUnit(properties.wordSpacing.value, properties.wordSpacing.unitType);
            blockSignals = false;
        }
    }

    function updateVisibility() {
        propertyState = [properties.wordSpacingState];
        setVisibleFromProperty();
    }

    onWordSpacingChanged: {
        if (!blockSignals) {
            properties.wordSpacing.value = wordSpacing;
        }
    }

    onWordSpacingUnitChanged: {
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

        Kis.DoubleSliderSpinBox {
            id: wordSpacingSpn;
            prefix: propertyTitle+ ": "
            Layout.fillWidth: true;

            dFrom: -99;
            dTo: 99;
            softDFrom: -10;
            softDTo: 10;
            softRangeActive: true;
            dStepSize: 0.1;

            onDValueChanged: wordSpacingUnitCmb.userValue = dValue;
            blockUpdateSignalOnDrag: true;
        }

        UnitComboBox {
            id: wordSpacingUnitCmb
            spinBoxControl: wordSpacingSpn;
            isFontSize: false;
            dpi: wordSpacingBase.dpi;
            onUserValueChanged: wordSpacingSpn.dValue = userValue;
            Layout.preferredWidth: minimumUnitBoxWidth;
            Layout.maximumWidth: implicitWidth;
            allowPercentage: false; // CSS-Text-4 has percentages for word-spacing, but so does SVG 1.1, and they both are implemented differently.
        }
    }
}
