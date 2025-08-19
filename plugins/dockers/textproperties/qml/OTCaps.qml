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
    propertyTitle: i18nc("@label", "Glyphs: Capitals");
    propertyName: "ot-capitals";
    propertyType: TextPropertyConfigModel.Character;
    toolTip: i18nc("@info:tooltip",
                   "Enable opentype features related to capitals");
    searchTerms: i18nc("comma separated search terms for the font-variant-caps property, matching is case-insensitive",
                       "font-variant-caps, Small caps, Petite Caps, Unicase, Titling, Capitals");

    property int capsType;

    Connections {
        target: properties;
        function onFontVariantCapsChanged() {
            updateOTCaps();
            updateVisibility();
        }

        function onFontVariantCapsStateChanged() {
            updateVisibility();
        }
    }
    onPropertiesChanged: {
        updateOTCaps();
        updateVisibility();
    }
    function updateOTCaps() {
        blockSignals = true;
        capsType = properties.fontVariantCaps;
        blockSignals = false;
    }

    function updateVisibility() {
        propertyState = [properties.fontVariantCapsState];
        setVisibleFromProperty();
    }

    onCapsTypeChanged: {
        capsCmb.currentIndex = capsCmb.indexOfValue(capsType)
        if (!blockSignals) {
            properties.fontVariantCaps = capsType;
        }
    }

    onEnableProperty: properties.fontVariantCapsState = KoSvgTextPropertiesModel.PropertySet;
    GridLayout {
        columns: 2;
        columnSpacing: columnSpacing;
        width: parent.width;

        RevertPropertyButton {
            revertState: properties.fontVariantCapsState;
            onClicked: properties.fontVariantCapsState = KoSvgTextPropertiesModel.PropertyUnset;
        }

        Label {
            text: propertyTitle;
            Layout.fillWidth: true;
        }

        Item {
            width: firstColumnWidth;
            height: 1;
        }
        ComboBox {
            model: [
                { text: i18nc("@label:inlistbox", "Normal"), value: KoSvgText.CapsNormal},
                { text: i18nc("@label:inlistbox", "Small Caps"), value: KoSvgText.CapsSmall},
                { text: i18nc("@label:inlistbox", "All Small Caps"), value: KoSvgText.CapsAllSmall},
                { text: i18nc("@label:inlistbox", "Petite Caps"), value: KoSvgText.CapsPetite},
                { text: i18nc("@label:inlistbox", "All Petite Caps"), value: KoSvgText.CapsAllPetite},
                { text: i18nc("@label:inlistbox", "Unicase"), value: KoSvgText.CapsUnicase},
                { text: i18nc("@label:inlistbox", "Titling Caps"), value: KoSvgText.CapsTitling}
            ]
            Layout.fillWidth: true;
            textRole: "text";
            valueRole: "value";
            onActivated: capsType = currentValue;
            id: capsCmb;
        }
    }
}
