/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12
import org.krita.flake.text 1.0

TextPropertyBase {
    propertyName: i18nc("@label:spinbox", "Font Size Adjust");
    propertyType: TextPropertyBase.Character;
    toolTip: i18nc("@info:tooltip",
                   "Font size adjust allows setting a ratio that the x-height must be matched by.");
    searchTerms: i18nc("comma separated search terms for the fontsize adjust property, matching is case-insensitive",
                       "size, small, big, medium, font-size-adjust");

    property alias fontSizeAdjust: fontSizeAdjustSpn.value;

    onPropertiesUpdated: {
        blockSignals = true;
        fontSizeAdjust = properties.fontSizeAdjust * fontSizeAdjustSpn.multiplier;
        visible = properties.fontSizeAdjustState !== KoSvgTextPropertiesModel.PropertyUnset;
        blockSignals = false;
    }
    onFontSizeAdjustChanged: {
        if (!blockSignals) {
            properties.fontSizeAdjust = fontSizeAdjust/fontSizeAdjustSpn.multiplier;
        }
    }

    onEnableProperty: properties.fontSizeAdjustState = KoSvgTextPropertiesModel.PropertySet;

    RowLayout {
        spacing: columnSpacing;
        width: parent.width;


        RevertPropertyButton {
            revertState: properties.fontSizeAdjustState;
            onClicked: properties.fontSizeAdjustState = KoSvgTextPropertiesModel.PropertyUnset;
        }

        Label {
            text: propertyName;
            elide: Text.ElideRight;
            Layout.fillWidth: true;
            font.italic: properties.fontSizeAdjustState === KoSvgTextPropertiesModel.PropertyTriState;
        }

        DoubleSpinBox {
            id: fontSizeAdjustSpn;
            editable: true;
            Layout.fillWidth: true;

            from: 0;
            to: 999 * multiplier;
            stepSize: 100;
        }

        Button {
            text: i18nc("@label:button", "Calculate");
            ToolTip.text: i18nc("@info:tooltip", "Calculate the x-height to font size ratio for the first font family.");
            onClicked: {
                let ratio = properties.resolvedXHeight(true)/properties.resolvedFontSize(true);
                fontSizeAdjust = ratio * fontSizeAdjustSpn.multiplier;
            }
        }
    }
}
