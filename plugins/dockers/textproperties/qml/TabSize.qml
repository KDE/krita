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
    propertyName: i18nc("@title:group", "Tab Size");
    propertyType: TextPropertyBase.Paragraph;
    toolTip: i18nc("@info:tooltip",
                   "Tab Size allows defining the size of tabulation characters.");
    searchTerms: i18nc("comma separated search terms for the tab-size property, matching is case-insensitive",
                       "tab-size");
    property alias tabSize: tabSizeSpn.value;
    property int tabSizeUnit: TabSizeModel.Spaces;

    onPropertiesUpdated: {
        blockSignals = true;
        tabSize = properties.tabSize.value * tabSizeSpn.multiplier;
        tabSizeUnit = properties.tabSize.unit;
        visible = properties.tabSizeState !== KoSvgTextPropertiesModel.PropertyUnset;
        blockSignals = false;
    }

    onTabSizeChanged: {
        if (!blockSignals) {
            properties.tabSize.value = tabSize / tabSizeSpn.multiplier;
        }
    }

    onTabSizeUnitChanged: {
        tabSizeUnitCmb.currentIndex = tabSizeUnitCmb.indexOfValue(tabSizeUnit);
        if (!blockSignals) {
            properties.tabSize.unit = tabSizeUnit;
        }
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
        ComboBox {
            id: tabSizeUnitCmb;
            property QtObject spinBoxControl: tabSizeSpn;
            model: [
                {text: i18nc("@label:inlistbox", "Spaces"), value: TabSizeModel.Spaces},
                {text: i18nc("@label:inlistbox", "Pt"), value: TabSizeModel.Absolute},
                {text: i18nc("@label:inlistbox", "Em"), value: TabSizeModel.Em},
                {text: i18nc("@label:inlistbox", "Ex"), value: TabSizeModel.Ex}
            ]
            textRole: "text";
            valueRole: "value";
            Layout.fillWidth: true;
            wheelEnabled: true;

            onActivated: {
                if (currentValue === TabSizeModel.Spaces) {
                    tabSizeUnit = currentValue;
                    spinBoxControl.value = properties.tabSize.value * tabSizeSpn.multiplier;
                } else {

                    var currentValueInPt = 0;
                    if (tabSizeUnit === TabSizeModel.Absolute) {
                        currentValueInPt = spinBoxControl.value;
                    } else if (tabSizeUnit === TabSizeModel.Ex) {
                        currentValueInPt = spinBoxControl.value * properties.resolvedXHeight(false);
                    } else { // EM, Lines
                        currentValueInPt = spinBoxControl.value * properties.resolvedFontSize(false);
                    }

                    var newValue = 0;
                    if (currentValue === TabSizeModel.Absolute) {
                        newValue = currentValueInPt
                    } else if (currentValue === TabSizeModel.Ex) {
                        newValue = currentValueInPt / properties.resolvedXHeight(false);
                    } else { // Em
                        newValue = currentValueInPt / properties.resolvedFontSize(false);
                    }
                    tabSizeUnit = currentValue;
                    spinBoxControl.value = newValue;
                }
            }
        }

    }
}
