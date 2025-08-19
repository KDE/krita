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
    id: baselineShiftBase;
    propertyTitle: i18nc("@label", "Baseline-Shift");
    propertyName: "baseline-shift";
    propertyType: TextPropertyConfigModel.Character;
    toolTip: i18nc("@info:tooltip",
                   "Baseline shift allows moving the text away from the baseline, either by predefined super and subscript values, or by a fixed amount.");
    searchTerms: i18nc("comma separated search terms for the baseline-shift property, matching is case-insensitive",
                       "baseline-shift, superscript, subscript");

    property alias baselineShiftValue: baselineShiftUnitCmb.dataValue;
    property alias baselineShiftUnit: baselineShiftUnitCmb.dataUnit;
    property int baselineShiftMode;

    Connections {
        target: properties;
        function onBaselineShiftModeChanged() {
            updateBaselineShiftMode();
            updateVisibility();
        }
        function onBaselineShiftValueChanged() {
            updateBaselineShiftValue();
            updateVisibility();
        }

        // Fontsize and lineheight affect the metrics.
        function onFontSizeChanged() {
            updateUnits();
        }
        function onFontFamiliesChanged() {
            updateUnits();
        }
        function onLineHeightChanged() {
            updateUnits();
        }

        function onBaselineShiftStateChanged() {
            updateVisibility();
        }
    }
    onPropertiesChanged: {
        updateUnits();
        updateBaselineShiftMode();
        updateBaselineShiftValue();
        updateVisibility();
    }

    function updateUnits() {
        blockSignals = true;
        baselineShiftUnitCmb.setTextProperties(properties);
        blockSignals = false;
    }

    function updateBaselineShiftMode() {
        blockSignals = true;
        baselineShiftMode = properties.baselineShiftMode;
        blockSignals = false;
    }
    function updateBaselineShiftValue() {
        blockSignals = true;
        baselineShiftUnitCmb.setDataValueAndUnit(properties.baselineShiftValue.value, properties.baselineShiftValue.unitType);
        blockSignals = false;
    }

    function updateVisibility() {
        propertyState = [properties.baselineShiftState];
        setVisibleFromProperty();
    }

    onBaselineShiftValueChanged: {
        if (!blockSignals) {
            properties.baselineShiftValue.value = baselineShiftValue;
        }
    }

    onBaselineShiftUnitChanged: {
        if (!blockSignals) {
            properties.baselineShiftValue.unitType = baselineShiftUnit;
        }
    }

    onBaselineShiftModeChanged: {
        baselineShiftCmb.currentIndex = baselineShiftCmb.indexOfValue(baselineShiftMode);
        baselineShiftSpn.enabled = baselineShiftMode == 3;
        if (!blockSignals) {
            properties.baselineShiftMode = baselineShiftMode;
        }
    }

    onEnableProperty: properties.baselineShiftState = KoSvgTextPropertiesModel.PropertySet;

    GridLayout {
        columns: 3;
        columnSpacing: columnSpacing;
        width: parent.width;

        RevertPropertyButton {
            revertState: properties.baselineShiftState;
            onClicked: properties.baselineShiftState = KoSvgTextPropertiesModel.PropertyUnset;
        }

        Label {
            text: propertyTitle;
            elide: Text.ElideRight;
            Layout.fillWidth: true;
            Layout.columnSpan: 2;
            font.italic: properties.baselineShiftState === KoSvgTextPropertiesModel.PropertyTriState;
        }


        Item {
            width: firstColumnWidth;
            height: 1;
        }

        SqueezedComboBox {
            Layout.columnSpan: 2;
            Layout.fillWidth: true;
            model: [
                { text: i18nc("@label:inlistbox", "None"), value: KoSvgText.ShiftNone, icon: ""},
                { text: i18nc("@label:inlistbox", "Length"), value: KoSvgText.ShiftLengthPercentage, icon: ""},
                { text: i18nc("@label:inlistbox", "Super"), value: KoSvgText.ShiftSuper, icon: "qrc:///16_light_format-text-superscript.svg"},
                { text: i18nc("@label:inlistbox", "Sub"), value: KoSvgText.ShiftSub, icon: "qrc:///16_light_format-text-subscript.svg"}
            ]
            id: baselineShiftCmb;
            textRole: "text";
            valueRole: "value";
            iconRole: "icon";
            iconSize: 16;
            onActivated: baselineShiftMode = currentValue;
            wheelEnabled: true;
        }

        Item {
            width: firstColumnWidth;
            height: 1;
        }

        MouseArea {
            id: baselineShiftSpnArea;
            Layout.fillWidth: true;
            Layout.fillHeight: true;
            Kis.DoubleSliderSpinBox {
                id: baselineShiftSpn
                width: parent.width;
                enabled: baselineShiftMode === KoSvgText.ShiftLengthPercentage;
                dFrom: -99;
                dTo: 99;
                onDValueChanged: if (enabled) baselineShiftUnitCmb.userValue = dValue;
                palette: baselinePalette.palette;
                blockUpdateSignalOnDrag: true;
                Kis.ThemedControl {
                    id: baselinePalette;
                }
            }
            onClicked: {
                if (!baselineShiftSpn.enabled) {
                    baselineShiftMode = 3;
                    baselineShiftSpn.forceActiveFocus();
                }
            }
        }

        UnitComboBox {
            id: baselineShiftUnitCmb;
            spinBoxControl: baselineShiftSpn;
            palette: baselinePalette.palette;
            Layout.preferredWidth: minimumUnitBoxWidth;
            Layout.maximumWidth: implicitWidth;
            isFontSize: false;
            enabled: baselineShiftMode === KoSvgText.ShiftLengthPercentage;
            dpi:baselineShiftBase.dpi;
            onUserValueChanged: baselineShiftSpn.dValue = userValue;
        }
    }
}
