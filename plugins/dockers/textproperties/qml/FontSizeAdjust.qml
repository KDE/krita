/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12
import org.krita.flake.text 1.0
import org.krita.components 1.0 as Kis

TextPropertyBase {
    propertyTitle: i18nc("@label:spinbox", "Font Size Adjust");
    propertyName: "font-size-adjust";
    propertyType: TextPropertyConfigModel.Character;
    toolTip: i18nc("@info:tooltip",
                   "Font size adjust allows setting a ratio that the x-height must be matched by.");
    searchTerms: i18nc("comma separated search terms for the fontsize adjust property, matching is case-insensitive",
                       "size, small, big, medium, font-size-adjust");

    property alias fontSizeAdjust: fontSizeAdjustSpn.dValue;

    Connections {
        target: properties;
        function onFontSizeAdjustChanged() {
            updateFontSizeAdjust();
            updateVisibility();
        }

        function onFontSizeAdjustStateChanged() {
            updateVisibility();
        }
    }
    onPropertiesChanged: {
        updateFontSizeAdjust();
        setVisibleFromProperty();
    }

    function updateFontSizeAdjust() {
        if (!fontSizeAdjustSpn.isDragging) {
            blockSignals = true;
            fontSizeAdjust = properties.fontSizeAdjust;
            blockSignals = false;
        }
    }
    function updateVisibility() {
        propertyState = [properties.fontSizeAdjustState];
        setVisibleFromProperty();
    }

    onFontSizeAdjustChanged: {
        if (!blockSignals) {
            properties.fontSizeAdjust = fontSizeAdjust;
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

        Kis.DoubleSliderSpinBox {
            id: fontSizeAdjustSpn
            prefix: propertyTitle+ ": "
            softDFrom: 0
            softDTo: 1
            dFrom: 0;
            dTo: 5;
            softRangeActive: true
            dStepSize: 0.1;
            Layout.preferredWidth: implicitWidth;
            Layout.fillWidth: true;
            //blockUpdateSignalOnDrag: true;
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
