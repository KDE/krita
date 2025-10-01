/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.12
import org.krita.flake.text 1.0
import org.krita.components 1.0 as Kis

TextPropertyBase {
    id: textAreaBase;
    propertyTitle: i18nc("@label:title", "Text Area");
    propertyName: "text-area";
    propertyType: TextPropertyConfigModel.Paragraph;
    toolTip: i18nc("@info:tooltip",
                   "Options for modifying the text area padding and margins.");
    searchTerms: i18nc("comma separated search terms for the text-area property, matching is case-insensitive",
                       "text-in-shape, padding, margin");
    property alias shapePadding: shapePaddingUnitCmb.dataValue;
    property alias shapePaddingUnit: shapePaddingUnitCmb.dataUnit;
    property alias shapeMargin: shapeMarginUnitCmb.dataValue;
    property alias shapeMarginUnit: shapeMarginUnitCmb.dataUnit;

    Connections {
        target: properties;
        function onShapePaddingChanged() {
            updateShapePadding();
            updateVisibility();
        }
        function onShapeMarginChanged() {
            updateShapeMargin();
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

        function onShapePaddingStateChanged() {
            updateVisibility();
        }
        function onShapeMarginStateChanged() {
            updateVisibility();
        }
    }
    onPropertiesChanged: {
        updateUnits();
        updateShapePadding();
        updateShapeMargin();
        updateVisibility();
    }

    function updateUnits() {
        blockSignals = true;
        shapePaddingUnitCmb.setTextProperties(properties);
        shapeMarginUnitCmb.setTextProperties(properties);
        blockSignals = false;
    }

    function updateShapePadding() {
        if (!shapePaddingSpn.dragging) {
            blockSignals = true;
            shapePaddingUnitCmb.setDataValueAndUnit(properties.shapePadding.value, properties.shapePadding.unitType);
            blockSignals = false;
        }
    }
    function updateShapeMargin() {
        if (!shapeMarginSpn.dragging) {
            blockSignals = true;
            shapeMarginUnitCmb.setDataValueAndUnit(properties.shapeMargin.value, properties.shapeMargin.unitType);
            blockSignals = false;
        }
    }

    function updateVisibility() {
        propertyState = [properties.shapePaddingState, properties.shapeMarginState];
        setVisibleFromProperty();
    }

    onShapePaddingChanged: {
        if (!blockSignals) {
            properties.shapePadding.value = shapePadding;
        }
    }

    onShapeMarginChanged: {
        if (!blockSignals) {
            properties.shapeMargin.value = shapeMargin;
        }
    }

    onShapePaddingUnitChanged: {
        if (!blockSignals) {
            properties.shapePadding.unitType = shapePaddingUnit;
        }
    }

    onShapeMarginUnitChanged: {
        if (!blockSignals) {
            properties.shapeMargin.unitType = shapeMarginUnit;
        }
    }

    onEnableProperty: properties.shapePaddingState = KoSvgTextPropertiesModel.PropertySet;

    GridLayout {
        columnSpacing: textAreaBase.columnSpacing;
        width: parent.width;
        columns: 3

        RevertPropertyButton {
            revertState: properties.shapePaddingState;
            onClicked: properties.shapePaddingState = KoSvgTextPropertiesModel.PropertyUnset;
        }


        Kis.DoubleSliderSpinBox {
            id: shapePaddingSpn;
            prefix: i18nc("@label:spinbox", "Shape Padding: ")
            Layout.fillWidth: true;

            dFrom: 0;
            dTo: 99;
            softDFrom: 0;
            softDTo: 10;
            dStepSize: 0.1;
            softRangeActive: true;

            onDValueChanged: shapePaddingUnitCmb.userValue = dValue;
            blockUpdateSignalOnDrag: true;
        }

        UnitComboBox {
            id: shapePaddingUnitCmb
            spinBoxControl: shapePaddingSpn;
            isFontSize: false;
            dpi:textAreaBase.dpi;
            onUserValueChanged: shapePaddingSpn.dValue = userValue;
            Layout.preferredWidth: minimumUnitBoxWidth;
            Layout.maximumWidth: implicitWidth;
            allowPercentage: false;
        }

        RevertPropertyButton {
            revertState: properties.shapeMarginState;
            onClicked: properties.shapeMarginState = KoSvgTextPropertiesModel.PropertyUnset;
        }

        Kis.DoubleSliderSpinBox {
            id: shapeMarginSpn;
            prefix: i18nc("@label:spinbox", "Shape Margin: ")
            Layout.fillWidth: true;

            dFrom: 0;
            dTo: 99;
            softDFrom: 0;
            softDTo: 10;
            dStepSize: 0.1;
            softRangeActive: true;

            onDValueChanged: shapeMarginUnitCmb.userValue = dValue;
            blockUpdateSignalOnDrag: true;
        }

        UnitComboBox {
            id: shapeMarginUnitCmb
            spinBoxControl: shapeMarginSpn;
            isFontSize: false;
            dpi:textAreaBase.dpi;
            onUserValueChanged: shapeMarginSpn.dValue = userValue;
            Layout.preferredWidth: minimumUnitBoxWidth;
            Layout.maximumWidth: implicitWidth;
            allowPercentage: false;
        }
    }
}
