/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQml.Models 2.1
import org.krita.flake.text 1.0

ColumnLayout {
    id: propertyBaseList;
    property int propertyType: TextPropertyConfigModel.Character;
    property TextPropertyConfigModel configModel;
    property double canvasDPI: 72.0;

    ListModel {
        id: propertyList;
    }

    ListModel {
        id: filteredPropertyList;
    }

    TextPropertyConfigFilterModel {
        id : filteredConfigModel;
        sourceModel: configModel;
        showParagraphProperties: propertyBaseList.propertyType === TextPropertyConfigModel.Paragraph;
        onShowParagraphPropertiesChanged: fillPropertyList();
        filterCaseSensitivity: Qt.CaseInsensitive;

        property bool isFiltering: configModel.shouldFilter;

        onFilteredNamesChanged: {
            if (isFiltering) {
                updatePropertyVisibilityState();
            }
        }
    }

    onConfigModelChanged: {if (configModel != null) fillPropertyList();}

    /// Initialize the property list and add each property to the TextPropertyConfigModel.
    function fillPropertyList() {
        for (var i = 0; i < propertyWidgetModel.count; i++) {
            let prop = propertyWidgetModel.get(i);
            configModel.addProperty(prop.propertyName,
                                    prop.propertyType,
                                    prop.propertyTitle,
                                    prop.toolTip,
                                    prop.searchTerms,
                                    prop.visibilityState
                                    );

            if (testShowProperty(prop.propertyType) && !shouldFilter(prop.propertyName)) {
                prop.setVisibleFromProperty();
            } else {
                prop.visible = false;
            }
        }
        configModel.loadFromConfiguration();
        updatePropertyVisibilityState();
    }

    /// Test if the current propertyType is the same as the one for this list.
    /// Used to filter out paragraph-properties from character property list and vice-versa.
    function testShowProperty(propertyType) {
        if (filteredConfigModel.showParagraphProperties && propertyType === TextPropertyConfigModel.Character) {
            return false;
        } else if (!filteredConfigModel.showParagraphProperties && propertyType === TextPropertyConfigModel.Paragraph) {
            return false;
        }
        return true;
    }

    /// Update properties on the list or hide them.
    function updateProperties() {
        for (var i = 0; i < propertyWidgetModel.count; i++) {
            let prop = propertyWidgetModel.get(i);
            if (testShowProperty(prop.propertyType)) {
                prop.propertiesUpdated();
                if (shouldFilter(prop.propertyName)) {
                    prop.visible = false;
                }
            } else {
                prop.visible = false;
            }
        }
    }

    /// Check if we're filtering at all...
    function shouldFilter(name) {
        if (!filteredConfigModel.isFiltering) return false;
        if (filteredConfigModel.filteredNames.length === 0) return false;
        if (filteredConfigModel.filteredNames.indexOf(name) < 0) {
            return true;
        }
        return false;
    }

    /// Update the visibility state from the config.
    function updatePropertyVisibilityState() {
        for (var i = 0; i < propertyWidgetModel.count; i++) {
            let prop = propertyWidgetModel.get(i);
            let name = prop.propertyName;
            prop.defaultVisibilityState = configModel.defaultVisibilityState;
            prop.visibilityState = configModel.visibilityStateForName(name);

            if (testShowProperty(prop.propertyType)) {
                if (shouldFilter(name)) {
                    prop.visible = false;
                } else {
                    prop.setVisibleFromProperty();
                }
            } else {
                prop.visible = false;
            }
        }
    }

    /// when the text property with the current propertyName is visible
    function isVisible(name) {
        for (var i = 0; i < propertyWidgetModel.count; i++) {
            let prop = propertyWidgetModel.get(i);
            if (name === prop.propertyName) {
                return prop.visible;
            }
        }
    }

    /// Enable the property with the current property name.
    function enableProperty(name) {
        for (var i = 0; i < propertyWidgetModel.count; i++) {
            if (name === propertyWidgetModel.get(i).propertyName) {
                propertyWidgetModel.get(i).enableProperty();
                break;
            }
        }
    }

    Frame {
        id: frame;
        Layout.fillHeight: true;
        Layout.fillWidth: true;
        clip: true;
        padding: 0;

        PaletteControl {
            id: paletteControl;
            colorGroup: frame.enabled? SystemPalette.Active: SystemPalette.Disabled;
        }
        palette: paletteControl.palette;

        ListView {
            id: propertiesWidgetView

            anchors.fill: parent;
            ScrollBar.vertical: ScrollBar {
            }

            model: ObjectModel {
                id: propertyWidgetModel;
                WritingMode {
                    dpi: canvasDPI;
                }
                Direction {
                    dpi: canvasDPI;
                }
                TextIndent{
                    dpi: canvasDPI;
                }
                TextAlign{
                    dpi: canvasDPI;
                }
                DominantBaseline {
                    dpi: canvasDPI;
                }
                WhiteSpace {
                    dpi: canvasDPI;
                }
                UnderlinePosition {
                    dpi: canvasDPI;
                }
                HangingPunctuation {
                    dpi: canvasDPI;
                }
                TabSize {
                    dpi: canvasDPI;
                }
                TextRendering {
                    dpi: canvasDPI;
                }
                FontSize {
                    dpi: canvasDPI;
                }
                FontSizeAdjust {
                    dpi: canvasDPI;
                }

                FontFamily {
                    dpi: canvasDPI;
                }
                FontStyle {
                    dpi: canvasDPI;
                }
                LetterSpacing {
                    dpi: canvasDPI;
                }
                WordSpacing {
                    dpi: canvasDPI;
                }
                LineHeight {
                    dpi: canvasDPI;
                }
                LineBreak {
                    dpi: canvasDPI;
                }
                WordBreak {
                    dpi: canvasDPI;
                }
                TextTransform {
                    dpi: canvasDPI;
                }
                TextDecoration {
                    dpi: canvasDPI;
                }
                OTLigatures {
                    dpi: canvasDPI;
                }
                OTPosition {
                    dpi: canvasDPI;
                }
                OTNumeric {
                    dpi: canvasDPI;
                }
                OTCaps {
                    dpi: canvasDPI;
                }
                OTEastAsian {
                    dpi: canvasDPI;
                }
                OpenTypeFeatureSettings {
                    dpi: canvasDPI;
                }
                FontKerning {
                    dpi: canvasDPI;
                }
                BaselineShift {
                    dpi: canvasDPI;
                }
                AlignmentBaseline {
                    dpi: canvasDPI;
                }
                Language {
                    dpi: canvasDPI;
                }
            }
        }
    }

    RowLayout {
        Layout.fillWidth: true;
        TextField {
            id: filterPropertyLn;
            Layout.fillWidth: true;
            Layout.minimumHeight: implicitHeight;
            placeholderText: i18nc("@info:placeholder", "Filter...");

            visible: filteredConfigModel.isFiltering;
            onVisibleChanged: {
                if (visible) {
                    text = "";
                }
            }

            onTextChanged: {
                if (visible) {
                    filteredConfigModel.setFilterRegularExpression(text);
                }
            }
        }

        ComboBox {
            id: addPropertyCmb;
            visible: !filteredConfigModel.isFiltering;
            onVisibleChanged: {
                if (visible) {
                    filteredConfigModel.setFilterRegularExpression("");
                }
            }

            Layout.fillWidth: true;
            Layout.minimumHeight: implicitHeight;
            model: filteredConfigModel;
            textRole: "display";
            displayText: i18nc("@label:listbox", "Add Property");
            onPopupChanged: if (!addPropertyCmb.popup.visible) { propertySearch.text = ""; }

            PaletteControl {
                id: cmbPalette;
                colorGroup: addPropertyCmb.enabled? SystemPalette.Active: SystemPalette.Disabled;
            }
            palette: cmbPalette.palette;

            function enableProperty(name) {
                propertySearch.blockSignals = true;
                propertySearch.text = "";
                propertyBaseList.enableProperty(name);
                popup.close();
                propertySearch.blockSignals = false;
            }

            contentItem: TextField {
                id: propertySearch;
                placeholderText: i18nc("@info:placeholder", "Add Property...");

                verticalAlignment: Text.AlignVCenter
                font: addPropertyCmb.font
                color: addPropertyCmb.palette.text
                selectionColor: addPropertyCmb.palette.highlight
                selectedTextColor: addPropertyCmb.palette.highlightedText;

                property bool blockSignals: false;

                selectByMouse: true;

                onTextChanged: {
                    if (text !== "") {
                        filteredConfigModel.setFilterRegularExpression(text);
                        addPropertyCmb.popup.open();
                    } else {
                        addPropertyCmb.popup.close();
                        filteredConfigModel.setFilterRegularExpression(text);
                    }
                }

                onAccepted: {
                    blockSignals = true;
                    let name = filteredConfigModel.filteredNames[addPropertyCmb.highlightedIndex]
                    if (typeof name != 'undefined') {
                        addPropertyCmb.enableProperty(name);
                    }
                    text = "";
                    blockSignals = false;
                }
            }

            delegate: ItemDelegate {
                id: addPropertyDelegate;
                width: addPropertyCmb.width;
                required property var model;
                required property string title;
                required property string name;
                required property int index;
                required property string toolTip;
                contentItem: Label {
                    enabled: addPropertyDelegate.enabled;
                    PaletteControl {
                        id: addPropertyPalette;
                        colorGroup: parent.enabled? SystemPalette.Active: SystemPalette.Disabled;
                    }
                    palette: addPropertyPalette.palette;
                    color: addPropertyDelegate.highlighted? palette.highlightedText: palette.text;
                    text: addPropertyDelegate.title;
                    elide: Text.ElideRight;
                    verticalAlignment: Text.AlignVCenter
                }
                enabled: !isVisible(name);
                highlighted: addPropertyCmb.highlightedIndex === index;
                background: Rectangle { color: highlighted? parent.palette.highlight:"transparent"; }

                ToolTip.text: toolTip;
                ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
                ToolTip.visible: highlighted;

                onClicked: addPropertyCmb.enableProperty(name);
            }
            popup.y: addPropertyCmb.height - 1;
            popup.palette: cmbPalette.palette;
        }

        ToolButton {
            id: configButton;
            icon.source: "qrc:///light_configure.svg"
            icon.color: palette.text;
            icon.width: 16;
            icon.height: 16;
            text: i18nc("@label:button", "Configure");
            display: AbstractButton.IconOnly;
            ToolTip.text: text;
            ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
            ToolTip.visible: highlighted;

            palette: paletteControl.palette;

            onClicked: mainWindow.callModalTextPropertyConfigDialog();
        }
    }
}
