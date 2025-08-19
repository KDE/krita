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
    propertyTitle: i18nc("@label:listbox", "Word Break");
    propertyName: "word-break";
    propertyType: TextPropertyConfigModel.Character;
    toolTip: i18nc("@info:tooltip",
                   "Word Break allows fine-tuning the line breaking by toggling whether to only break at words or also allow breaking at characters. Useful for Korean or Ethiopian.");
    searchTerms: i18nc("comma separated search terms for the word-break property, matching is case-insensitive",
                       "word-break, line breaking");
    property int breakType;

    Connections {
        target: properties;
        function onWordBreakChanged() {
            updateWordBreak();
            updateVisibility();
        }

        function onWordBreakStateChanged() {
            updateVisibility();
        }
    }
    onPropertiesChanged: {
        updateWordBreak();
        updateVisibility();
    }

    function updateVisibility() {
        propertyState = [properties.wordBreakState];
        setVisibleFromProperty();
    }

    function updateWordBreak() {
        blockSignals = true;
        breakType = properties.wordBreak;
        blockSignals = false;
    }

    onBreakTypeChanged: {
        wordBreakCmb.currentIndex = wordBreakCmb.indexOfValue(breakType)
        if (!blockSignals) {
            properties.wordBreak = breakType;
        }
    }

    onEnableProperty: properties.wordBreakState = KoSvgTextPropertiesModel.PropertySet;

    RowLayout {
        spacing: columnSpacing;
        width: parent.width;

        RevertPropertyButton {
            revertState: properties.wordBreakState;
            onClicked: properties.wordBreakState = KoSvgTextPropertiesModel.PropertyUnset;
        }

        Label {
            text: propertyTitle;
            elide: Text.ElideRight;
            Layout.fillWidth: true;
            font.italic: properties.wordBreakState === KoSvgTextPropertiesModel.PropertyTriState;
        }


        SqueezedComboBox {
            id: wordBreakCmb
            model: [
                { text: i18nc("@label:inlistbox", "Normal"), value: KoSvgText.WordBreakNormal,
                    toolTip:i18nc("@info:tooltip", "No additional adjustments are made to the regular line break algorithm.")},
                { text: i18nc("@label:inlistbox", "Keep-all"), value: KoSvgText.WordBreakKeepAll,
                toolTip:i18nc("@info:tooltip", "Soft breaks will only be allowed inside words, typically delimited by spaces.")},
                { text: i18nc("@label:inlistbox", "Break-all"), value: KoSvgText.WordBreakBreakAll,
                    toolTip:i18nc("@info:tooltip", "Soft breaks will occur along any grapheme.")}
            ]
            Layout.fillWidth: true;
            textRole: "text";
            valueRole: "value";
            toolTipRole: "toolTip";
            onActivated: breakType = currentValue;
            wheelEnabled: true;
        }
    }
}
