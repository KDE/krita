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
    propertyTitle: i18nc("@label", "Glyphs: Numeric");
    propertyName: "ot-numeric";
    propertyType: TextPropertyConfigModel.Character;
    toolTip: i18nc("@info:tooltip",
                   "Enable number-related glyph forms on the text.");
    searchTerms: i18nc("comma separated search terms for the font-variant-numeric property, matching is case-insensitive",
                       "font-variant-numeric, oldstyle numerals, lining numerals, ordinals, slashed zero, fractions");

    property int figureStyleType;
    property int figureSpacingType;
    property int fractionType;
    property alias ordinals: ordinalsCbx.checked;
    property alias slashedZero: slashedZeroCbx.checked;

    Connections {
        target: properties;
        function onFontVariantNumericChanged() {
            updateOTNumeric();
            updateVisibility();
        }

        function onFontVariantNumericStateChanged() {
            updateVisibility();
        }
    }
    onPropertiesChanged: {
        updateOTNumeric();
        updateVisibility();
    }
    function updateOTNumeric() {
        blockSignals = true;
        figureStyleType = properties.fontVariantNumeric.figureStyle;
        figureSpacingType = properties.fontVariantNumeric.figureSpacing;
        fractionType = properties.fontVariantNumeric.fractions;
        ordinals = properties.fontVariantNumeric.ordinals;
        slashedZero = properties.fontVariantNumeric.slashedZero;
        blockSignals = false;
    }

    function updateVisibility() {
        propertyState = [properties.fontVariantNumericState];
        setVisibleFromProperty();
    }

    onFigureStyleTypeChanged: {
        styleCmb.currentIndex = styleCmb.indexOfValue(figureStyleType)
        if (!blockSignals) {
            properties.fontVariantNumeric.figureStyle = figureStyleType;
        }
    }

    onFigureSpacingTypeChanged: {
        spacingCmb.currentIndex = spacingCmb.indexOfValue(figureSpacingType)
        if (!blockSignals) {
            properties.fontVariantNumeric.figureSpacing = figureSpacingType;
        }
    }

    onFractionTypeChanged: {
        fractionCmb.currentIndex = fractionCmb.indexOfValue(fractionType)
        if (!blockSignals) {
            properties.fontVariantNumeric.fractions = fractionType;
        }
    }

    onOrdinalsChanged: {
        if (!blockSignals) {
            properties.fontVariantNumeric.ordinals = ordinals;
        }
    }

    onSlashedZeroChanged: {
        if (!blockSignals) {
            properties.fontVariantNumeric.slashedZero = slashedZero;
        }
    }

    onEnableProperty: properties.fontVariantNumericState = KoSvgTextPropertiesModel.PropertySet;
    GridLayout {
        columns: 3;
        columnSpacing: columnSpacing;
        width: parent.width;

        RevertPropertyButton {
            revertState: properties.fontVariantNumericState;
            onClicked: properties.fontVariantNumericState = KoSvgTextPropertiesModel.PropertyUnset;
        }

        Label {
            text: propertyTitle;
            Layout.fillWidth: true;
            Layout.columnSpan: 2;
        }

        Item {
            width: firstColumnWidth;
            height: 1;
        }
        Label {
            text:  i18nc("@label:listbox", "Style:")
        }
        ComboBox {
            model: [
                { text: i18nc("@label:inlistbox", "Normal"), value: KoSvgText.NumericFigureStyleNormal},
                { text: i18nc("@label:inlistbox", "Lining"), value: KoSvgText.NumericFigureStyleLining},
                { text: i18nc("@label:inlistbox", "Oldstyle"), value: KoSvgText.NumericFigureStyleOld}
            ]
            Layout.fillWidth: true;
            textRole: "text";
            valueRole: "value";
            onActivated: figureStyleType = currentValue;
            id: styleCmb;
        }

        Item {
            width: firstColumnWidth;
            height: 1;
        }
        Label {
            text:  i18nc("@label:listbox", "Spacing:")
        }
        ComboBox {
            model: [
                { text: i18nc("@label:inlistbox", "Normal"), value: KoSvgText.NumericFigureSpacingNormal},
                { text: i18nc("@label:inlistbox", "Proportional"), value: KoSvgText.NumericFigureSpacingProportional},
                { text: i18nc("@label:inlistbox", "Tabular"), value: KoSvgText.NumericFigureSpacingTabular}
            ]
            Layout.fillWidth: true;
            textRole: "text";
            valueRole: "value";
            onActivated: figureSpacingType = currentValue;
            id: spacingCmb;
        }

        Item {
            width: firstColumnWidth;
            height: 1;
        }
        Label {
            text:  i18nc("@label:listbox", "Fractions:")
        }
        ComboBox {
            model: [
                { text: i18nc("@label:inlistbox", "Normal"), value: KoSvgText.NumericFractionsNormal},
                { text: i18nc("@label:inlistbox", "Diagonal"), value: KoSvgText.NumericFractionsDiagonal},
                { text: i18nc("@label:inlistbox", "Stacked"), value: KoSvgText.NumericFractionsStacked}
            ]
            Layout.fillWidth: true;
            textRole: "text";
            valueRole: "value";
            onActivated: fractionType = currentValue;
            id: fractionCmb;
        }

        Item {
            width: firstColumnWidth;
            height: 1;
            Layout.columnSpan: 2;
        }
        CheckBox {

            text: i18nc("@option:check", "Ordinals")
            id: ordinalsCbx;
        }
        Item {
            width: firstColumnWidth;
            height: 1;
            Layout.columnSpan: 2;
        }
        CheckBox {

            text: i18nc("@option:check", "Slashed Zero")
            id: slashedZeroCbx;
        }

    }
}
