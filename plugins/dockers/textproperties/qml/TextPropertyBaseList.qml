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
import org.krita.components 1.0 as Kis

ColumnLayout {
    id: propertyBaseList;
    property int propertyType: TextPropertyConfigModel.Character;
    property TextPropertyConfigModel configModel;
    property double canvasDPI: 72.0;
    property KoSvgTextPropertiesModel propertiesModel;
    property var locales: [];

    property alias textConfigButtonVisible: configButton.visible;

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

            prop.parentPropertyType = propertyBaseList.propertyType;
            if (!shouldFilter(prop.propertyName)) {
                prop.setVisibleFromProperty();
            } else {
                prop.visible = false;
            }
        }
        configModel.loadFromConfiguration();
        updatePropertyVisibilityState();
    }

    /// Update properties on the list or hide them.
    function updateProperties() {
        for (var i = 0; i < propertyWidgetModel.count; i++) {
            let prop = propertyWidgetModel.get(i);
            if (shouldFilter(prop.propertyName)) {
                prop.visible = false;
            } else {
                prop.setVisibleFromProperty();
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

            if (shouldFilter(name)) {
                prop.visible = false;
            } else {
                prop.setVisibleFromProperty();
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

    signal callPropertyVisibilityConfig;

    Item {
        id: topPadding;
        height: Math.floor(spacing/2);
    }

    Frame {
        id: frame;
        Layout.fillHeight: true;
        Layout.fillWidth: true;
        clip: true;
        padding: 0;

        Kis.ThemedControl {
            id: paletteControl;
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
                    properties: propertiesModel;
                }
                Direction {
                    dpi: canvasDPI;
                    properties: propertiesModel;
                }
                TextIndent{
                    dpi: canvasDPI;
                    properties: propertiesModel;
                }
                TextAlign{
                    dpi: canvasDPI;
                    properties: propertiesModel;
                }
                DominantBaseline {
                    dpi: canvasDPI;
                    properties: propertiesModel;
                }
                WhiteSpace {
                    dpi: canvasDPI;
                    properties: propertiesModel;
                }
                UnderlinePosition {
                    dpi: canvasDPI;
                    properties: propertiesModel;
                }
                HangingPunctuation {
                    dpi: canvasDPI;
                    properties: propertiesModel;
                }
                TabSize {
                    dpi: canvasDPI;
                    properties: propertiesModel;
                }
                TextRendering {
                    dpi: canvasDPI;
                    properties: propertiesModel;
                }
                FontSize {
                    dpi: canvasDPI;
                    properties: propertiesModel;
                }
                FontSizeAdjust {
                    dpi: canvasDPI;
                    properties: propertiesModel;
                }

                FontFamily {
                    dpi: canvasDPI;
                    properties: propertiesModel;
                    locales: propertyBaseList.locales;
                }
                FontStyle {
                    dpi: canvasDPI;
                    properties: propertiesModel;
                }
                LetterSpacing {
                    dpi: canvasDPI;
                    properties: propertiesModel;
                }
                WordSpacing {
                    dpi: canvasDPI;
                    properties: propertiesModel;
                }
                LineHeight {
                    dpi: canvasDPI;
                    properties: propertiesModel;
                }
                LineBreak {
                    dpi: canvasDPI;
                    properties: propertiesModel;
                }
                WordBreak {
                    dpi: canvasDPI;
                    properties: propertiesModel;
                }
                TextTransform {
                    dpi: canvasDPI;
                    properties: propertiesModel;
                }
                TextDecoration {
                    dpi: canvasDPI;
                    properties: propertiesModel;
                }
                OTLigatures {
                    dpi: canvasDPI;
                    properties: propertiesModel;
                }
                OTPosition {
                    dpi: canvasDPI;
                    properties: propertiesModel;
                }
                OTNumeric {
                    dpi: canvasDPI;
                    properties: propertiesModel;
                }
                OTCaps {
                    dpi: canvasDPI;
                    properties: propertiesModel;
                }
                OTEastAsian {
                    dpi: canvasDPI;
                    properties: propertiesModel;
                }
                OpenTypeFeatureSettings {
                    dpi: canvasDPI;
                    properties: propertiesModel;
                }
                FontKerning {
                    dpi: canvasDPI;
                    properties: propertiesModel;
                }
                BaselineShift {
                    dpi: canvasDPI;
                    properties: propertiesModel;
                }
                AlignmentBaseline {
                    dpi: canvasDPI;
                    properties: propertiesModel;
                }
                Language {
                    dpi: canvasDPI;
                    properties: propertiesModel;
                }
            }
        }
    }

    RowLayout {
        Layout.fillWidth: true;
        StackLayout {
            Layout.fillWidth: true;
            Layout.maximumHeight: Math.max(addPropertyCmb.implicitHeight, filterPropertyLn.implicitHeight);

            currentIndex: filteredConfigModel.isFiltering? 0: 1;
            TextField {
                id: filterPropertyLn;
                Layout.fillWidth: true;
                Layout.maximumHeight: implicitHeight;
                placeholderText: i18nc("@info:placeholder", "Filter...");

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
                Layout.fillWidth: true;
                Layout.maximumHeight: implicitHeight;
                onVisibleChanged: {
                    if (visible) {
                        filteredConfigModel.setFilterRegularExpression("");
                    }
                }
                model: filteredConfigModel;
                textRole: "display";
                displayText: i18nc("@label:listbox", "Add Property");
                onPopupChanged: if (!addPropertyCmb.popup.visible) { propertySearch.text = ""; }

                Kis.ThemedControl {
                    id: cmbPalette;
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

                    Keys.onDownPressed: {
                        addPropertyCmb.incrementCurrentIndex();
                    }
                    Keys.onUpPressed: {
                        addPropertyCmb.decrementCurrentIndex();
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
                        Kis.ThemedControl {
                            id: addPropertyPalette;
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

                    Kis.ToolTipBase {
                        parent: addPropertyDelegate;
                        text: addPropertyDelegate.toolTip;
                        visible: addPropertyDelegate.highlighted && addPropertyCmb.popup.visible;
                    }

                    onClicked: addPropertyCmb.enableProperty(name);
                }
                popup.y: addPropertyCmb.height - 1;
                popup.palette: cmbPalette.palette;
            }
        }
        ToolButton {
            id: configButton;
            icon.source: "qrc:///light_configure.svg"
            icon.color: palette.text;
            icon.width: visible? 16: 0;
            icon.height: 16;
            text: i18nc("@label:button", "Configure");
            display: AbstractButton.IconOnly;

            Kis.ToolTipBase {
                text: configButton.text;
                visible: configButton.highlighted;
            }

            palette: paletteControl.palette;

            onClicked: callPropertyVisibilityConfig();
        }

    }
}
