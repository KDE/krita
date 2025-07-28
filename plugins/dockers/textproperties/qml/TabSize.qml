/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12
import org.krita.flake.text 1.0
import org.krita.components 1.0 as Kis

TextPropertyBase {
    propertyTitle: i18nc("@title:group", "Tab Size");
    propertyName: "tab-size";
    propertyType: TextPropertyConfigModel.Paragraph;
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

        onUserValueChanged: tabSizeSpn.dValue = userValue;
        onUserUnitChanged: tabSizeUnitCmb.currentIndex = tabSizeUnitCmb.indexOfValue(userUnit);
    }

    onPropertiesUpdated: {
        blockSignals = true;
        converter.dpi = canvasDPI;
        converter.setFontMetricsFromTextPropertiesModel(properties);
        converter.setDataValueAndUnit(properties.tabSize.value, properties.tabSize.unit);

        propertyState = [properties.tabSizeState];
        setVisibleFromProperty();
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

    RowLayout {
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: columnSpacing;

        RevertPropertyButton {
            revertState: properties.tabSizeState;
            onClicked: properties.tabSizeState = KoSvgTextPropertiesModel.PropertyUnset;
        }

        Kis.DoubleSliderSpinBox {
            id: tabSizeSpn;
            prefix: propertyTitle + ": ";
            Layout.fillWidth: true;
            dFrom: 0;
            dTo: 99;
            onDValueChanged: converter.userValue = dValue;
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
