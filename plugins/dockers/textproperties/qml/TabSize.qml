/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12
import org.krita.flake.text 1.0

TextPropertyBase {
    propertyName: i18nc("@title:group", "Tab Size");
    propertyType: TextPropertyBase.Paragraph;
    toolTip: i18nc("@info:tooltip",
                   "Tab Size allows defining the size of tabulation characters.");
    searchTerms: i18nc("comma separated search terms for the tab-size property, matching is case-insensitive",
                       "tab-size");
    property alias tabSize: converter.dataValue;
    property alias tabSizeUnit: converter.dataUnit;

    property var unitMap: [
        { user: CssQmlUnitConverter.Pt, data: TabSizeModel.Absolute},
        { user: CssQmlUnitConverter.Em, data: TabSizeModel.Em},
        { user: CssQmlUnitConverter.Ex, data: TabSizeModel.Ex},
        { user: CssQmlUnitConverter.Cap, data: TabSizeModel.Cap},
        { user: CssQmlUnitConverter.Ch, data: TabSizeModel.Ch},
        { user: CssQmlUnitConverter.Ic, data: TabSizeModel.Ic},
        { user: CssQmlUnitConverter.Lh, data: TabSizeModel.Lh},
        { user: CssQmlUnitConverter.Spaces, data: TabSizeModel.Spaces},
    ]

    CssQmlUnitConverter {
        id: converter;
        dpi: dpi;
        dataMultiplier: tabSizeSpn.multiplier;
        userValue: tabSizeSpn.value;
        userUnit: tabSizeUnitCmb.currentValue;

        onUserValueChanged: tabSizeSpn.value = userValue;
        onUserUnitChanged: tabSizeUnitCmb.currentIndex = tabSizeUnitCmb.indexOfValue(userUnit);
    }

    onPropertiesUpdated: {
        blockSignals = true;
        converter.dpi = canvasDPI;
        converter.setFontMetricsFromTextPropertiesModel(properties);
        converter.setDataValueAndUnit(properties.tabSize.unit, properties.tabSize.value);
        visible = properties.tabSizeState !== KoSvgTextPropertiesModel.PropertyUnset;
        blockSignals = false;
    }

    onTabSizeChanged: {
        if (!blockSignals) {
            properties.tabSize.value = tabSize;
        }
    }

    onTabSizeUnitChanged: {
        if (!blockSignals) {
            properties.tabSize.unit = tabSizeUnit;
        }
    }

    Component.onCompleted: {
        converter.setDataUnitMap(unitMap);
        converter.setDataValueAndUnit(0, 0);
    }

    onEnableProperty: properties.tabSizeState = KoSvgTextPropertiesModel.PropertySet;

    GridLayout {
        columns: 3
        anchors.left: parent.left
        anchors.right: parent.right
        columnSpacing: columnSpacing;

        RevertPropertyButton {
            revertState: properties.tabSizeState;
            onClicked: properties.tabSizeState = KoSvgTextPropertiesModel.PropertyUnset;
        }
        Label {
            text: propertyName
            Layout.columnSpan: 2;
            elide: Text.ElideRight;
            Layout.fillWidth: true;
            font.italic: properties.tabSizeState === KoSvgTextPropertiesModel.PropertyTriState;
        }

        Item {
            width: 1;
            height: 1;
        }

        DoubleSpinBox {
            id: tabSizeSpn;
            Layout.fillWidth: true;
            from: 0;
            to: 100 * multiplier;
        }
        SqueezedComboBox {
            id: tabSizeUnitCmb;
            model:converter.userUnitModel;
            textRole: "description";
            valueRole: "value";
            displayText: converter.symbol;
            wheelEnabled: true;
            Layout.preferredWidth: symbolWidth.width+leftPadding+rightPadding+spacing+indicator.width;
            TextMetrics {
                id: symbolWidth;
                font: tabSizeUnitCmb.font;
                text: tabSizeUnitCmb.displayText;
            }
            Layout.maximumWidth: implicitWidth;



            onCurrentValueChanged: converter.userUnit = currentValue;
        }

    }
}
