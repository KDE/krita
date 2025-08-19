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
    propertyTitle: i18nc("@label", "Glyphs: East-Asian");
    propertyName: "ot-east-asian";
    propertyType: TextPropertyConfigModel.Character;
    toolTip: i18nc("@info:tooltip",
                   "Enable glyph forms related to East Asian text layout.");
    searchTerms: i18nc("comma separated search terms for the font-variant-east-asian property, matching is case-insensitive",
                       "font-variant-east-asian, jis78, jis83, jis90, jis04, ruby, proportional, full-width");

    property int variantType;
    property int widthType;
    property alias ruby: rubyCbx.checked;

    Connections {
        target: properties;
        function onFontVariantEastAsianChanged() {
            updateOTEastAsian();
            updateVisibility();
        }

        function onFontVariantEastAsianStateChanged() {
            updateVisibility();
        }
    }
    onPropertiesChanged: {
        updateOTEastAsian();
        updateVisibility();
    }

    function updateOTEastAsian() {
        blockSignals = true;
        ruby = properties.fontVariantEastAsian.ruby;
        variantType = properties.fontVariantEastAsian.variant;
        widthType = properties.fontVariantEastAsian.width;
        blockSignals = false;
    }

    function updateVisibility() {
        propertyState = [properties.fontVariantEastAsianState];
        setVisibleFromProperty();
    }

    onVariantTypeChanged: {
        variantCmb.currentIndex = variantCmb.indexOfValue(variantType)
        if (!blockSignals) {
            properties.fontVariantEastAsian.variant = variantType;
        }
    }

    onWidthTypeChanged: {
        widthCmb.currentIndex = widthCmb.indexOfValue(widthType)
        if (!blockSignals) {
            properties.fontVariantEastAsian.width = widthType;
        }
    }

    onRubyChanged: {
        if (!blockSignals) {
            properties.fontVariantEastAsian.ruby = ruby;
        }
    }

    onEnableProperty: properties.fontVariantEastAsianState = KoSvgTextPropertiesModel.PropertySet;
    GridLayout {
        columns: 3;
        columnSpacing: columnSpacing;
        width: parent.width;

        RevertPropertyButton {
            revertState: properties.fontVariantEastAsianState;
            onClicked: properties.fontVariantEastAsianState = KoSvgTextPropertiesModel.PropertyUnset;
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
            text: i18nc("@label:listbox", "Variant:")
        }
        ComboBox {
            model: [
                { text: i18nc("@label:inlistbox", "Normal"), value: KoSvgText.EastAsianVariantNormal},
                { text: i18nc("@label:inlistbox", "JIS78"), value: KoSvgText.EastAsianJis78},
                { text: i18nc("@label:inlistbox", "JIS83"), value: KoSvgText.EastAsianJis83},
                { text: i18nc("@label:inlistbox", "JIS90"), value: KoSvgText.EastAsianJis90},
                { text: i18nc("@label:inlistbox", "JIS04"), value: KoSvgText.EastAsianJis04},
                { text: i18nc("@label:inlistbox", "Simplified"), value: KoSvgText.EastAsianSimplified},
                { text: i18nc("@label:inlistbox", "Traditional"), value: KoSvgText.EastAsianTraditional}
            ]
            Layout.fillWidth: true;
            textRole: "text";
            valueRole: "value";
            onActivated: variantType = currentValue;
            id: variantCmb;
        }

        Item {
            width: firstColumnWidth;
            height: 1;
        }
        Label {
            text: i18nc("@label:listbox", "Width:")
        }
        ComboBox {
            model: [
                { text: i18nc("@label:inlistbox", "Normal"), value: KoSvgText.EastAsiantNormalWidth},
                { text: i18nc("@label:inlistbox", "Full width"), value: KoSvgText.EastAsianFullWidth},
                { text: i18nc("@label:inlistbox", "Proportional"), value: KoSvgText.EastAsianProportionalWidth}
            ]
            Layout.fillWidth: true;
            textRole: "text";
            valueRole: "value";
            onActivated: widthType = currentValue;
            id: widthCmb;
        }

        Item {
            width: firstColumnWidth;
            height: 1;
            Layout.columnSpan: 2;
        }
        CheckBox {
            text: i18nc("@option:check", "Ruby")
            id: rubyCbx;
        }

    }
}
