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
    property int breakType;

    propertyName: i18nc("@label:listbox", "Line Break");
    toolTip: i18nc("@info:tooltip",
                   "Line Break allows choosing a strictness for the line breaking algorithm. Mostly used for CJK scripts, requires language being set.");
    searchTerms: i18nc("comma separated search terms for the line-break property, matching is case-insensitive",
                       "line-break, line breaking, strictness, kinsoku");
    propertyType: TextPropertyBase.Character;

    onPropertiesUpdated: {
        blockSignals = true;
        breakType = properties.lineBreak;
        visible = properties.lineBreakState !== KoSvgTextPropertiesModel.PropertyUnset;
        blockSignals = false;
    }

    onBreakTypeChanged: {
        lineBreakCmb.currentIndex = lineBreakCmb.indexOfValue(breakType)
        if (!blockSignals) {
            properties.lineBreak = breakType;
        }
    }

    onEnableProperty: properties.lineBreakState = KoSvgTextPropertiesModel.PropertySet;

    RowLayout {
        spacing: columnSpacing;
        width: parent.width;

        RevertPropertyButton {
            revertState: properties.lineBreakState;
            onClicked: properties.lineBreakState = KoSvgTextPropertiesModel.PropertyUnset;
        }

        Label {
            text: propertyName;
            elide: Text.ElideRight;
            Layout.fillWidth: true;
            font.italic: properties.lineBreakState === KoSvgTextPropertiesModel.PropertyTriState;
        }

        ComboBox {
            id: lineBreakCmb
            model: [
                { text: i18nc("@label:inlistbox", "Auto"), value: KoSvgText.LineBreakAuto},
                { text: i18nc("@label:inlistbox", "Loose"), value: KoSvgText.LineBreakLoose},
                { text: i18nc("@label:inlistbox", "Normal"), value: KoSvgText.LineBreakNormal},
                { text: i18nc("@label:inlistbox", "Strict"), value: KoSvgText.LineBreakStrict},
                { text: i18nc("@label:inlistbox", "Anywhere"), value: KoSvgText.LineBreakAnywhere}
            ]
            Layout.fillWidth: true;
            textRole: "text";
            valueRole: "value";
            onActivated: breakType = currentValue;
        }
    }
}
