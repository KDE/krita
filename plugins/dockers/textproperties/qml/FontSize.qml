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
    id: fontSizeBase;
    propertyTitle: i18nc("@label:spinbox", "Font Size");
    propertyName: "font-size";
    propertyType: TextPropertyConfigModel.Character;
    visibilityState: TextPropertyConfigModel.AlwaysVisible;
    toolTip: i18nc("@info:tooltip",
                   "Font size allows setting the size of the characters.");
    searchTerms: i18nc("comma separated search terms for the fontsize property, matching is case-insensitive",
                       "size, small, big, medium");

    property alias fontSize: fontSizeUnitCmb.dataValue;
    property alias fontSizeUnit: fontSizeUnitCmb.dataUnit;

    onPropertiesUpdated: {
        blockSignals = true;
        fontSizeUnitCmb.setTextProperties(properties);
        fontSizeUnitCmb.setDataValueAndUnit(properties.fontSize.value, properties.fontSize.unitType);
        propertyState = [properties.fontSizeState];
        setVisibleFromProperty();
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

        Kis.DoubleSliderSpinBox {
            id: fontSizeSpn;
            prefix: propertyTitle+ ": "
            Layout.fillWidth: true;

            dFrom: 0;
            dTo: 999;
            softDFrom: 0;
            softDTo: 72;
            softRangeActive: true;
            blockUpdateSignalOnDrag: true;

            onDValueChanged: fontSizeUnitCmb.userValue = dValue;
        }

        UnitComboBox {
            id: fontSizeUnitCmb;
            spinBoxControl: fontSizeSpn;
            dpi: fontSizeBase.dpi;
            isFontSize: true;
            isLineHeight: false;
            percentageReference: properties.resolvedFontSize(true);
            Layout.preferredWidth: minimumUnitBoxWidth;
            Layout.maximumWidth: implicitWidth;

            onUserValueChanged: fontSizeSpn.dValue = userValue;
        }
    }
}
