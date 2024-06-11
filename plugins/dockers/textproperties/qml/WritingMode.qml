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
    propertyName: i18nc("@label:listbox", "Writing Mode");
    propertyType: TextPropertyBase.Paragraph;
    toolTip: i18nc("@info:tooltip",
                   "Writing Mode sets whether the text flows horizontally or vertically, and in the latter case, whether the block flows right to left or left to right.");
    searchTerms: i18nc("comma separated search terms for the writing-mode property, matching is case-insensitive",
                       "writing-mode, horizontal, vertical, top-to-bottom");
    property int writingMode;

    onPropertiesUpdated: {
        blockSignals = true;
        writingMode = properties.writingMode;
        visible = properties.writingModeState !== KoSvgTextPropertiesModel.PropertyUnset;
        blockSignals = false;
    }

    onWritingModeChanged: {
        writingModeCmb.currentIndex = writingModeCmb.indexOfValue(writingMode)
        if (!blockSignals) {
            properties.writingMode = writingMode;
        }
    }

    onEnableProperty: properties.writingModeState = KoSvgTextPropertiesModel.PropertySet;

    RowLayout {
        spacing: columnSpacing;
        width: parent.width;

        RevertPropertyButton {
            revertState: properties.writingModeState;
            onClicked: properties.writingModeState = KoSvgTextPropertiesModel.PropertyUnset;
        }

        Label {
            text: propertyName;
            elide: Text.ElideRight;
            Layout.fillWidth: true;
            font.italic: properties.writingModeState === KoSvgTextPropertiesModel.PropertyTriState;
        }


        ComboBox {
            id: writingModeCmb
            model: [
                {text: i18nc("@label:inlistbox", "Horizontal"), value: KoSvgText.HorizontalTB},
                {text: i18nc("@label:inlistbox", "Vertical, Right to Left"), value: KoSvgText.VerticalRL},
                {text: i18nc("@label:inlistbox", "Vertical, Left To Right"), value: KoSvgText.VerticalLR}
            ]
            Layout.fillWidth: true;
            textRole: "text";
            valueRole: "value";
            onActivated: writingMode = currentValue;
            wheelEnabled: true;
        }
    }
}
