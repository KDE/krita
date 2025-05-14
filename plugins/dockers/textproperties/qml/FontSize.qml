/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.12
import org.krita.flake.text 1.0
import org.krita.components 1.0

TextPropertyBase {
    propertyTitle: i18nc("@label:spinbox", "Font Size");
    propertyName: "font-size";
    propertyType: TextPropertyBase.Character;
    toolTip: i18nc("@info:tooltip",
                   "Font size allows setting the size of the characters.");
    searchTerms: i18nc("comma separated search terms for the fontsize property, matching is case-insensitive",
                       "size, small, big, medium");

    property alias fontSize: fontSizeUnitCmb.dataValue;
    property alias fontSizeUnit: fontSizeUnitCmb.dataUnit;

    onPropertiesUpdated: {
        blockSignals = true;
        fontSizeUnitCmb.dpi = canvasDPI;
        fontSizeUnitCmb.setTextProperties(properties);
        fontSizeUnitCmb.setDataValueAndUnit(properties.fontSize.value, properties.fontSize.unitType);
        visible = properties.fontSizeState !== KoSvgTextPropertiesModel.PropertyUnset;
        blockSignals = false;
    }
    onFontSizeChanged: {
        if (!blockSignals) {
            properties.fontSize.value = fontSize;
        }
    }
    onFontSizeUnitChanged: {
        if (!blockSignals) {
            properties.fontSize.unitType = fontSizeUnit;
        }
    }

    onEnableProperty: properties.fontSizeState = KoSvgTextPropertiesModel.PropertySet;

    RowLayout {
        spacing: columnSpacing;
        width: parent.width;

        RevertPropertyButton {
            revertState: properties.fontSizeState;
            onClicked: properties.fontSizeState = KoSvgTextPropertiesModel.PropertyUnset;
        }

        Label {
            text: propertyTitle;
            elide: Text.ElideRight;
            Layout.fillWidth: true;
            font.italic: properties.fontSizeState === KoSvgTextPropertiesModel.PropertyTriState;
        }

        DoubleSpinBox {
            id: fontSizeSpn;
            editable: true;
            Layout.fillWidth: true;

            from: 0;
            to: 99999 * multiplier;
            stepSize: 100;

            onValueChanged: fontSizeUnitCmb.userValue = value;
        }

        UnitComboBox {
            id: fontSizeUnitCmb;
            spinBoxControl: fontSizeSpn;
            isFontSize: true;
            isLineHeight: false;
            percentageReference: properties.resolvedFontSize(true);
            Layout.preferredWidth: minimumUnitBoxWidth;
            Layout.maximumWidth: implicitWidth;

            onUserValueChanged: fontSizeSpn.value = userValue;
        }
    }
}
