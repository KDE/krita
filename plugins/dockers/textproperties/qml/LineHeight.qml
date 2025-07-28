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
    propertyTitle: i18nc("@label", "Line Height");
    propertyName: "line-height";
    propertyType: TextPropertyConfigModel.Character;
    toolTip: i18nc("@info:tooltip",
                   "Line Height controls the line height used for the range of text.");
    searchTerms: i18nc("comma separated search terms for the line-height property, matching is case-insensitive",
                       "line-height, line-spacing");
    property alias isNormal: lineHeightNormalCbx.checked;
    property alias lineHeight: converter.dataValue;
    property alias lineHeightUnit: converter.dataUnit;
    property var unitMap: [
        { user: CssQmlUnitConverter.Pt, data: LineHeightModel.Absolute},
        { user: CssQmlUnitConverter.Em, data: LineHeightModel.Em},
        { user: CssQmlUnitConverter.Ex, data: LineHeightModel.Ex},
        { user: CssQmlUnitConverter.Cap, data: LineHeightModel.Cap},
        { user: CssQmlUnitConverter.Ch, data: LineHeightModel.Ch},
        { user: CssQmlUnitConverter.Ic, data: LineHeightModel.Ic},
        { user: CssQmlUnitConverter.Lh, data: LineHeightModel.Lh},
        { user: CssQmlUnitConverter.Percentage, data: LineHeightModel.Percentage},
        { user: CssQmlUnitConverter.Lines, data: LineHeightModel.Lines},
    ]

    CssQmlUnitConverter {
        id: converter;
        dpi: dpi;

        onUserValueChanged: lineHeightSpn.dValue = userValue;
        onUserUnitChanged: lineHeightUnitCmb.currentIndex = lineHeightUnitCmb.indexOfValue(userUnit);
    }

    onPropertiesUpdated: {
        blockSignals = true;
        isNormal = properties.lineHeight.isNormal;
        converter.dpi = canvasDPI;
        converter.setFontMetricsFromTextPropertiesModel(properties, false, true);
        converter.percentageReference = properties.resolvedFontSize(false);
        converter.setDataValueAndUnit(properties.lineHeight.value, properties.lineHeight.unit);

        propertyState = [properties.lineHeightState];
        setVisibleFromProperty();
        blockSignals = false;
    }

    onLineHeightUnitChanged: {
        if (!blockSignals && !isNormal) {
            properties.lineHeight.unit = lineHeightUnit
        }
    }
    onIsNormalChanged: {
        if (!blockSignals) {
            if (isNormal) {
                converter.setFromNormalLineHeight();
            }
            properties.lineHeight.isNormal = isNormal;
        }
    }

    onLineHeightChanged: {
        if (!blockSignals && !isNormal) {
            properties.lineHeight.value = lineHeight;
        }
    }

    onEnableProperty: properties.lineHeightState = KoSvgTextPropertiesModel.PropertySet;

    Component.onCompleted: {
        mainWindow.connectAutoEnabler(lineHeightSpnArea);
        converter.setDataUnitMap(unitMap);
        converter.setDataValueAndUnit(0, 0);
    }

    GridLayout {
        columns: 3;
        columnSpacing: columnSpacing;
        width: parent.width;

        RevertPropertyButton {
            revertState: properties.lineHeightState;
            onClicked: properties.lineHeightState = KoSvgTextPropertiesModel.PropertyUnset;
        }

        Label {
            text: propertyTitle;
            elide: Text.ElideRight;
            Layout.fillWidth: true;
            font.italic: properties.lineHeightState === KoSvgTextPropertiesModel.PropertyTriState;
        }

        CheckBox {
            text: i18nc("@option:check", "Normal")
            id: lineHeightNormalCbx;
            onCheckedChanged: {
                lineHeightSpn.enabled = !checked;
                lineHeightUnitCmb.enabled = !checked;
            }
        }
        Item {
            width: firstColumnWidth;
            height: 1;
        }
        RowLayout {
            Layout.fillHeight: true;
            Layout.fillWidth: true;
            Layout.columnSpan: 2;
            MouseArea {
                id: lineHeightSpnArea;
                function autoEnable() {
                    lineHeightNormalCbx.checked = false;
                }
                Layout.fillWidth: true;
                Layout.fillHeight: true;
                Kis.DoubleSliderSpinBox {
                    id: lineHeightSpn
                    width: parent.width;
                    enabled: !lineHeightNormalCbx.checked;
                    dFrom: 0;
                    dTo: 99;

                    onDValueChanged:if (enabled)  converter.userValue = dValue;
                    palette: lineHeightPalette.palette;
                    blockUpdateSignalOnDrag: true;
                }
            }

            SqueezedComboBox {
                id: lineHeightUnitCmb;
                model: converter.userUnitModel;
                textRole: "description";
                valueRole: "value";
                displayText: converter.symbol;
                enabled: !lineHeightNormalCbx.checked;

                TextMetrics {
                    id: symbolWidth;
                    font: lineHeightUnitCmb.font;
                    text: lineHeightUnitCmb.displayText;
                }
                Layout.preferredWidth: symbolWidth.width+leftPadding+rightPadding+spacing+indicator.width;
                Layout.maximumWidth: implicitWidth;

                PaletteControl {
                    id: lineHeightPalette;
                }
                palette: lineHeightPalette.palette;
                wheelEnabled: true;

                onCurrentValueChanged: converter.userUnit = currentValue;
            }
        }
    }
}
