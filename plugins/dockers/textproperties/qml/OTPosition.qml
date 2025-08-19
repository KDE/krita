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
    propertyTitle: i18nc("@label", "Glyphs: Position");
    propertyName: "ot-position";
    propertyType: TextPropertyConfigModel.Character;
    toolTip: i18nc("@info:tooltip",
                   "Enable super or subscripts on the text.");
    searchTerms: i18nc("comma separated search terms for the font-variant-position property, matching is case-insensitive",
                       "font-variant-position, superscript, subscript");

    property int positionType;

    Connections {
        target: properties;
        function onFontVariantPositionChanged() {
            updateOTPosition();
            updateVisibility();
        }

        function onFontVariantPositionStateChanged() {
            updateVisibility();
        }
    }
    onPropertiesChanged: {
        updateOTPosition();
        updateVisibility();
    }
    function updateOTPosition() {
        blockSignals = true;
        positionType = properties.fontVariantPosition;
        blockSignals = false;
    }

    function updateVisibility() {
        propertyState = [properties.fontVariantPositionState];
        setVisibleFromProperty();
    }

    onPositionTypeChanged: {
        positionCmb.currentIndex = positionCmb.indexOfValue(positionType)
        if (!blockSignals) {
            properties.fontVariantPosition = positionType;
        }
    }

    onEnableProperty: properties.fontVariantPositionState = KoSvgTextPropertiesModel.PropertySet;
    GridLayout {
        columns: 2;
        columnSpacing: columnSpacing;
        width: parent.width;

        RevertPropertyButton {
            revertState: properties.fontVariantPositionState;
            onClicked: properties.fontVariantPositionState = KoSvgTextPropertiesModel.PropertyUnset;
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
                { text: i18nc("@label:inlistbox", "Normal"), value: KoSvgText.PositionNormal},
                { text: i18nc("@label:inlistbox", "Super"), value: KoSvgText.PositionSuper},
                { text: i18nc("@label:inlistbox", "Sub"), value: KoSvgText.PositionSub}
            ]
            Layout.fillWidth: true;
            textRole: "text";
            valueRole: "value";
            onActivated: positionType = currentValue;
            id: positionCmb;
        }
    }
}
