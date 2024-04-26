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
    propertyName: i18nc("@label:listbox", "Word Break");
    property int breakType;


    onPropertiesUpdated: {
        blockSignals = true;
        breakType = properties.wordBreak;
        visible = properties.wordBreakState !== KoSvgTextPropertiesModel.PropertyUnset;
        blockSignals = false;
    }

    onBreakTypeChanged: {
        wordBreakCmb.currentIndex = wordBreakCmb.indexOfValue(breakType)
        if (!blockSignals) {
            properties.wordBreak = breakType;
        }
    }

    RowLayout {
        spacing: columnSpacing;
        width: parent.width;

        RevertPropertyButton {
            revertEnabled: properties.wordBreakState === KoSvgTextPropertiesModel.PropertySet;
            onClicked: properties.wordBreakState = KoSvgTextPropertiesModel.PropertyUnset;
        }

        Label {
            text: propertyName;
            elide: Text.ElideRight;
            Layout.fillWidth: true;
        }


        ComboBox {
            id: wordBreakCmb
            model: [
                { text: i18nc("@label:inlistbox", "Normal"), value: KoSvgText.WordBreakNormal},
                { text: i18nc("@label:inlistbox", "Keep-all"), value: KoSvgText.WordBreakKeepAll},
                { text: i18nc("@label:inlistbox", "Break-all"), value: KoSvgText.WordBreakBreakAll}
            ]
            Layout.fillWidth: true;
            textRole: "text";
            valueRole: "value";
            onActivated: breakType = currentValue;
        }
    }
}
