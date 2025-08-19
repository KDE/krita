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

CollapsibleGroupProperty {
    id: textIndentBase;
    propertyTitle: i18nc("@title:group", "Text Indent");
    propertyName: "text-indent";
    propertyType: TextPropertyConfigModel.Paragraph;
    toolTip: i18nc("@info:tooltip",
                   "Text Indent allows setting indentation at the line start. Only works when the text is wrapping.");
    searchTerms: i18nc("comma separated search terms for the text-indent property, matching is case-insensitive",
                       "text-indent");

    property alias textIndentValue: textIndentUnitCmb.dataValue;
    property alias hanging: indentHangingCkb.checked;
    property alias eachLine: eachLineCkb.checked;
    property alias textIndentUnit: textIndentUnitCmb.dataUnit;

    Connections {
        target: properties;
        function onTextIndentChanged() {
            updateTextIndentValue();
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

        function onTextIndentStateChanged() {
            updateVisibility();
        }
    }
    onPropertiesChanged: {
        updateUnits();
        updateTextIndentValue();
        updateVisibility();
    }

    function updateUnits() {
        blockSignals = true;
        textIndentUnitCmb.setTextProperties(properties);
        blockSignals = false;
    }

    function updateTextIndentValue() {
        blockSignals = true;
        textIndentUnitCmb.setDataValueAndUnit(properties.textIndent.length.value, properties.textIndent.length.unitType);
        hanging = properties.textIndent.hanging;
        eachLine = properties.textIndent.eachLine;
        blockSignals = false;
    }

    function updateVisibility() {
        propertyState = [properties.textIndentState];
        setVisibleFromProperty();
    }

    onTextIndentValueChanged: {
        if (!blockSignals) {
            properties.textIndent.length.value = textIndentValue;
        }
    }

    onTextIndentUnitChanged: {
        if (!blockSignals) {
            properties.textIndent.length.unitType = textIndentUnit;
        }
    }

    onHangingChanged: {
        if (!blockSignals) {
            properties.textIndent.hanging = hanging;
        }
    }

    onEachLineChanged: {
        if (!blockSignals) {
            properties.textIndent.eachLine = eachLine;
        }
    }

    onEnableProperty: properties.textIndentState = KoSvgTextPropertiesModel.PropertySet;

    titleItem: RowLayout {
        width: parent.width;
        height: childrenRect.height;
        spacing: columnSpacing;

        Kis.DoubleSliderSpinBox {
            prefix: propertyTitle + ": ";
            id: textIndentSpn;
            Layout.fillWidth: true;
            dFrom: 0;
            dTo: 999;
            onDValueChanged: textIndentUnitCmb.userValue = dValue;
            blockUpdateSignalOnDrag: true;
        }
        /// Note: percentage calculation in the default unitcombobox isn't great for textIndent as it assumes 100% = fontsize,
        /// While spec-wise, it's the inline length that defines percentage.
        UnitComboBox {
            id: textIndentUnitCmb;
            dpi: textIndentBase.dpi;
            spinBoxControl: textIndentSpn;
            onUserValueChanged: textIndentSpn.dValue = userValue;
            Layout.preferredWidth: minimumUnitBoxWidth;
            Layout.maximumWidth: implicitWidth;
        }
    }

    contentItem: GridLayout {
        columns: 2
        anchors.left: parent.left
        anchors.right: parent.right
        columnSpacing: columnSpacing;

        RevertPropertyButton {
            revertState: properties.textIndentState;
            onClicked: properties.textIndentState = KoSvgTextPropertiesModel.PropertyUnset;
        }


        CheckBox {
            id: indentHangingCkb;
            text: i18nc("@option:check", "Hanging indentation")
            Layout.fillWidth: true
        }

        Item {
            width: firstColumnWidth;
            height: firstColumnWidth;
            Layout.columnSpan: 1;
        }

        CheckBox {
            id: eachLineCkb;
            text: i18nc("@option:check", "Indent after hardbreaks")
            Layout.fillWidth: true
        }

    }
}


