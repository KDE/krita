/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Controls 2.15
import org.krita.flake.text 1.0

SqueezedComboBox {
    id: root;
    property QtObject spinBoxControl;
    property bool isFontSize: false;
    property bool isLineHeight: false;
    property alias dataUnit: converter.dataUnit;
    property alias dataValue: converter.dataValue;
    property alias dpi: converter.dpi;
    property alias userValue: converter.userValue;

    property bool allowPercentage: true;
    property alias percentageReference: converter.percentageReference;

    wheelEnabled: true;

    TextMetrics {
        id: symbolWidth;
        font: root.font;
        text: displayText;
    }

    property int minimumUnitBoxWidth: symbolWidth.width+leftPadding+rightPadding+spacing+indicator.width;

    displayText: converter.symbol;

    function setTextProperties(properties) {
        converter.setFontMetricsFromTextPropertiesModel(properties, isFontSize, isLineHeight);
    }
    function setDataValueAndUnit(value, unit) {
        converter.setDataValueAndUnit(value, unit);
    }

    CssQmlUnitConverter {
        id: converter;

        onUserUnitChanged: root.currentIndex = root.indexOfValue(userUnit);
    }

    Component.onCompleted: {
        var userUnitModel = [
                    { user: CssQmlUnitConverter.Pt, data: 0},
                    { user: CssQmlUnitConverter.Em, data: 2},
                    { user: CssQmlUnitConverter.Ex, data: 3},
                    { user: CssQmlUnitConverter.Cap, data: 4},
                    { user: CssQmlUnitConverter.Ch, data: 5},
                    { user: CssQmlUnitConverter.Ic, data: 6},
                    { user: CssQmlUnitConverter.Lh, data: 7}
                ];
        if (allowPercentage) {
            // Not great, but listmodels don't seem to allow adding i18n strings as properties, because they are in functions?
            userUnitModel.push( { user: CssQmlUnitConverter.Percentage, data: 1})
        }
        converter.setDataUnitMap(userUnitModel);
        converter.setDataValueAndUnit(0, 0);
    }

    model: converter.userUnitModel;
    textRole: "description";
    valueRole: "value";
    onCurrentValueChanged: {
        converter.userUnit = currentValue;
    }
}
