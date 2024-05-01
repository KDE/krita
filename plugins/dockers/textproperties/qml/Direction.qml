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
    propertyName: i18nc("@label:listbox", "Direction")
    propertyType: TextPropertyBase.Mixed;
    property int direction;
    property int unicodeBidi;

    onPropertiesUpdated: {
        blockSignals = true;
        direction = properties.direction;
        unicodeBidi = properties.unicodeBidi;
        var set = properties.directionState !== KoSvgTextPropertiesModel.PropertyUnset;
        var pType = parentPropertyType === TextPropertyBase.Paragraph? !properties.spanSelection: properties.spanSelection;
        console.log("set", set, "type matches", pType)
        visible = set && pType;
        blockSignals = false;
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

    GridLayout {
        id: row
        columns: 3
        columnSpacing: parent.columnSpacing;
        width: parent.width;

        RevertPropertyButton {
            revertEnabled: properties.directionState === KoSvgTextPropertiesModel.PropertySet;
            onClicked: properties.directionState = KoSvgTextPropertiesModel.PropertyUnset;
        }

        Label {
            text: propertyName;
            elide: Text.ElideRight;
            Layout.fillWidth: true;
        }

        ComboBox {
            id: directionCmb
            Layout.fillWidth: true;
            model: [
                {text: i18nc("@label:inlistbox", "Left to Right"), value: KoSvgText.DirectionLeftToRight},
                {text: i18nc("@label:inlistbox", "Right to Left"), value: KoSvgText.DirectionRightToLeft}
            ]
            textRole: "text";
            valueRole: "value";
            onActivated: direction = currentValue;
        }

        RevertPropertyButton {
            visible: parentPropertyType === TextPropertyBase.Character;
            revertEnabled: properties.unicodeBidiState === KoSvgTextPropertiesModel.PropertySet;
            onClicked: properties.unicodeBidiState = KoSvgTextPropertiesModel.PropertyUnset;
        }

        Label {
            text: i18nc("@label:listbox", "Unicode-Bidi");
            elide: Text.ElideRight;
            Layout.fillWidth: true;
            visible: parentPropertyType === TextPropertyBase.Character;
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
        }
    }
}
