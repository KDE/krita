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

    propertyTitle: i18nc("@label:listbox", "Line Break");
    propertyName: "line-break";
    toolTip: i18nc("@info:tooltip",
                   "Line Break allows choosing a strictness for the line breaking algorithm. Mostly used for CJK scripts, requires language being set.");
    searchTerms: i18nc("comma separated search terms for the line-break property, matching is case-insensitive",
                       "line-break, line breaking, strictness, kinsoku");
    propertyType: TextPropertyConfigModel.Character;

    Connections {
        target: properties;
        function onLineBreakChanged() {
            updateLineBreak();
            updateVisibility();
        }

        function onLineBreakStateChanged() {
            updateVisibility();
        }
    }
    onPropertiesChanged: {
        updateLineBreak();
        updateVisibility();
    }

    function updateVisibility() {
        propertyState = [properties.lineBreakState];
        setVisibleFromProperty();
    }

    function updateLineBreak() {
        blockSignals = true;
        breakType = properties.lineBreak;
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
            text: propertyTitle;
            elide: Text.ElideRight;
            Layout.maximumWidth: implicitWidth;
            font.italic: properties.lineBreakState === KoSvgTextPropertiesModel.PropertyTriState;
        }

        SqueezedComboBox {
            id: lineBreakCmb
            model: [
                { text: i18nc("@label:inlistbox", "Auto"), value: KoSvgText.LineBreakAuto,
                    toolTip:i18nc("@info:tooltip", "No particular adjustments are made to the line break algorithm")},
                { text: i18nc("@label:inlistbox", "Loose"), value: KoSvgText.LineBreakLoose,
                    toolTip:i18nc("@info:tooltip", "While typically defines a looser algorithm, currently same as auto.")},
                { text: i18nc("@label:inlistbox", "Normal"), value: KoSvgText.LineBreakNormal,
                    toolTip:i18nc("@info:tooltip", "Same as auto. Soft breaks will be allowed before small kana if text language is Japanese.")},
                { text: i18nc("@label:inlistbox", "Strict"), value: KoSvgText.LineBreakStrict,
                    toolTip:i18nc("@info:tooltip", "Soft breaks will not be allowed before small kana.")},
                { text: i18nc("@label:inlistbox", "Anywhere"), value: KoSvgText.LineBreakAnywhere,
                    toolTip:i18nc("@info:tooltip", "Soft breaks will occur along any grapheme.")}
            ]
            Layout.fillWidth: true;
            textRole: "text";
            valueRole: "value";
            toolTipRole: "toolTip";
            onActivated: breakType = currentValue;
        }
    }
}
