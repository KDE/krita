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
    propertyTitle: i18nc("@label:listbox", "Writing Mode");
    propertyName: "writing-mode";
    propertyType: TextPropertyConfigModel.Paragraph;
    toolTip: i18nc("@info:tooltip",
                   "Writing Mode sets whether the text flows horizontally or vertically, and in the latter case, whether the block flows right to left or left to right.");
    searchTerms: i18nc("comma separated search terms for the writing-mode property, matching is case-insensitive",
                       "writing-mode, horizontal, vertical, top-to-bottom");
    property int writingMode;

    Connections {
        target: properties;
        function onWritingModeChanged() {
            updateWritingMode();
            updateVisibility();
        }

        function onWritingModeStateChanged() {
            updateVisibility();
        }
    }
    onPropertiesChanged: {
        updateWritingMode();
        updateVisibility();
    }

    function updateWritingMode() {
        blockSignals = true;
        writingMode = properties.writingMode;
        blockSignals = false;
    }

    function updateVisibility() {
        propertyState = [properties.writingModeState];
        setVisibleFromProperty();
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
            text: propertyTitle;
            elide: Text.ElideRight;
            Layout.fillWidth: true;
            font.italic: properties.writingModeState === KoSvgTextPropertiesModel.PropertyTriState;
        }


        SqueezedComboBox {
            id: writingModeCmb
            model: [
                {text: i18nc("@label:inlistbox", "Horizontal"), value: KoSvgText.HorizontalTB, icon: "qrc:///16_light_format-text-direction-horizontal-tb.svg"},
                {text: i18nc("@label:inlistbox", "Vertical, Right to Left"), value: KoSvgText.VerticalRL, icon: "qrc:///16_light_format-text-direction-vertical-rl.svg"},
                {text: i18nc("@label:inlistbox", "Vertical, Left To Right"), value: KoSvgText.VerticalLR, icon: "qrc:///16_light_format-text-direction-vertical-lr.svg"}
            ]
            Layout.fillWidth: true;
            textRole: "text";
            valueRole: "value";
            iconRole: "icon";
            iconSize: 16;
            onActivated: writingMode = currentValue;
        }
    }
}
