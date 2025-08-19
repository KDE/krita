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
    propertyTitle: i18nc("@label:listbox", "Direction");
    propertyName: "direction";
    propertyType: TextPropertyConfigModel.Mixed;
    toolTip: i18nc("@info:tooltip",
                   "Direction sets whether the text is left-to-right or right-to-left.");
    searchTerms: i18nc("comma separated search terms for the direction property, matching is case-insensitive",
                       "right to left, rtl, direction, bidirectional, isolate, embed, override");

    property int direction;
    property int unicodeBidi;

    Connections {
        target: properties;
        function onDirectionChanged() {
            updateDirection();
            updateVisibility();
        }
        function onUnicodeBidiChanged() {
            updateUnicodeBidi();
            updateVisibility();
        }

        function onDirectionStateChanged() {
            updateVisibility();
        }
    }
    onPropertiesChanged: {
        updateDirection();
        updateUnicodeBidi();
        updateVisibility();
    }

    function updateDirection() {
        blockSignals = true;
        direction = properties.direction;
        blockSignals = false;
    }

    function updateUnicodeBidi() {
        blockSignals = true;
        unicodeBidi = properties.unicodeBidi;
        blockSignals = false;
    }

    function updateVisibility() {
        enabled = parentPropertyType === TextPropertyBase.Paragraph? !properties.spanSelection: properties.spanSelection;
        propertyState = [properties.directionState];
        setVisibleFromProperty();
    }

    onDirectionChanged: {
        directionCmb.currentIndex = directionCmb.indexOfValue(direction);
        if (!blockSignals) {
            properties.direction = direction;
        }
    }

    onUnicodeBidiChanged: {
        unicodeBidiCmb.currentIndex = unicodeBidiCmb.indexOfValue(unicodeBidi);
        if (!blockSignals) {
            properties.unicodeBidi = unicodeBidi;
        }
    }

    onEnableProperty: properties.directionState = KoSvgTextPropertiesModel.PropertySet;

    GridLayout {
        id: row
        columns: 3
        columnSpacing: parent.columnSpacing;
        width: parent.width;

        RevertPropertyButton {
            revertState: properties.directionState;
            onClicked: properties.directionState = KoSvgTextPropertiesModel.PropertyUnset;
        }

        Label {
            text: propertyTitle;
            elide: Text.ElideRight;
            Layout.fillWidth: true;
            font.italic: properties.directionState === KoSvgTextPropertiesModel.PropertyTriState;
            palette: directionCmbPalette.palette;
        }

        SqueezedComboBox {
            id: directionCmb
            Layout.fillWidth: true;
            Layout.preferredWidth: implicitWidth;
            model: [
                {text: i18nc("@label:inlistbox", "Left to Right"), value: KoSvgText.DirectionLeftToRight, icon: "qrc:///16_light_format-text-direction-ltr.svg"},
                {text: i18nc("@label:inlistbox", "Right to Left"), value: KoSvgText.DirectionRightToLeft, icon: "qrc:///16_light_format-text-direction-rtl.svg"}
            ]
            textRole: "text";
            valueRole: "value";
            iconRole: "icon";
            iconSize: 16;
            onActivated: direction = currentValue;
            wheelEnabled: true;
            Kis.ThemedControl {
                id: directionCmbPalette;
            }
            palette: directionCmbPalette.palette;
        }

        RevertPropertyButton {
            visible: parentPropertyType === TextPropertyBase.Character;
            revertState: properties.unicodeBidiState;
            onClicked: properties.unicodeBidiState = KoSvgTextPropertiesModel.PropertyUnset;
        }

        Label {
            text: i18nc("@label:listbox", "Unicode-Bidi");
            elide: Text.ElideRight;
            Layout.fillWidth: true;
            visible: parentPropertyType === TextPropertyBase.Character;
            font.italic: properties.unicodeBidiState === KoSvgTextPropertiesModel.PropertyTriState;
            palette: unicodeBidiCmbPalette.palette;
        }

        ComboBox {
            id: unicodeBidiCmb
            visible: parentPropertyType === TextPropertyBase.Character;
            Layout.fillWidth: true;
            model: [
                {text: i18nc("@label:inlistbox", "Normal"), value: KoSvgText.BidiNormal},
                {text: i18nc("@label:inlistbox", "Embed"), value: KoSvgText.BidiEmbed},
                {text: i18nc("@label:inlistbox", "Override"), value: KoSvgText.BidiOverride},
                {text: i18nc("@label:inlistbox", "Isolate"), value: KoSvgText.BidiIsolate},
                {text: i18nc("@label:inlistbox", "Isolate-Override"), value: KoSvgText.BidiIsolateOverride},
                {text: i18nc("@label:inlistbox", "Plain Text"), value: KoSvgText.BidiPlainText}
            ]
            textRole: "text";
            valueRole: "value";
            onActivated: unicodeBidi = currentValue;
            wheelEnabled: true;

            Kis.ThemedControl {
                id: unicodeBidiCmbPalette;
            }
            palette: unicodeBidiCmbPalette.palette;
        }
    }
}
