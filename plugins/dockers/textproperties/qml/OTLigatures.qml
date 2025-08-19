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
    propertyTitle: i18nc("@label", "Glyphs: Ligatures");
    propertyName: "ot-ligatures";
    propertyType: TextPropertyConfigModel.Character;
    toolTip: i18nc("@info:tooltip",
                   "Enable or disable ligatures and contextual alternates on the text.");
    searchTerms: i18nc("comma separated search terms for the font-variant-ligatures property, matching is case-insensitive",
                       "font-variant-ligatures, common-ligatures, discretionary-ligatures, historical-ligatures, contextual-alts");

    property alias commonLigatures: commonLigaCbx.checked;
    property alias discretionaryLigatures: discretionaryLigaCbx.checked;
    property alias historicalLigatures: historicalLigaCbx.checked;
    property alias contextualAlternates: contextualLigaCbx.checked;

    Connections {
        target: properties;
        function onFontVariantLigaturesChanged() {
            updateOTLigatures();
            updateVisibility();
        }

        function onFontVariantLigaturesStateChanged() {
            updateVisibility();
        }
    }
    onPropertiesChanged: {
        updateOTLigatures();
        updateVisibility();
    }
    function updateOTLigatures() {
        blockSignals = true;
        commonLigatures = properties.fontVariantLigatures.commonLigatures;
        discretionaryLigatures = properties.fontVariantLigatures.discretionaryLigatures;
        historicalLigatures = properties.fontVariantLigatures.historicalLigatures;
        contextualAlternates = properties.fontVariantLigatures.contextualAlternates;
        blockSignals = false;
    }

    function updateVisibility() {
        propertyState = [properties.fontVariantLigaturesState];
        setVisibleFromProperty();
    }

    onCommonLigaturesChanged: {
        if (!blockSignals) {
            properties.fontVariantLigatures.commonLigatures = commonLigatures;
        }
    }

    onDiscretionaryLigaturesChanged: {
        if (!blockSignals) {
            properties.fontVariantLigatures.discretionaryLigatures = discretionaryLigatures;
        }
    }

    onHistoricalLigaturesChanged: {
        if (!blockSignals) {
            properties.fontVariantLigatures.historicalLigatures = historicalLigatures;
        }
    }

    onContextualAlternatesChanged: {
        if (!blockSignals) {
            properties.fontVariantLigatures.contextualAlternates = contextualAlternates;
        }
    }

    onEnableProperty: properties.fontVariantLigaturesState = KoSvgTextPropertiesModel.PropertySet;

    GridLayout {
        columns: 2;
        columnSpacing: columnSpacing;
        width: parent.width;

        RevertPropertyButton {
            revertState: properties.fontVariantLigaturesState;
            onClicked: properties.fontVariantLigaturesState = KoSvgTextPropertiesModel.PropertyUnset;
        }

        Label {
            text: propertyTitle;
            Layout.fillWidth: true;
        }


        Item {
            width: firstColumnWidth;
            height: 1;
        }
        CheckBox {
            text: i18nc("@option:check", "Common")
            id: commonLigaCbx;
        }
        Item {
            width: firstColumnWidth;
            height: 1;
        }
        CheckBox {
            text: i18nc("@option:check", "Discretionary")
            id: discretionaryLigaCbx;
        }
        Item {
            width: firstColumnWidth;
            height: 1;
        }
        CheckBox {
            text: i18nc("@option:check", "Historical")
            id: historicalLigaCbx;
        }
        Item {
            width: firstColumnWidth;
            height: 1;
        }
        CheckBox {
            text: i18nc("@option:check", "Contextual")
            id: contextualLigaCbx;
        }

    }
}
