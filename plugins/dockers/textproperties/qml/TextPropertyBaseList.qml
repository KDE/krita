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
    }

    onConfigModelChanged: {if (configModel != null) fillPropertyList();}

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

            if (testShowProperty(prop.propertyType)) {
                prop.setVisibleFromProperty();
            } else {
                prop.visible = false;
            }
        }
        textPropertyConfigModel.loadFromConfiguration();
        updatePropertyVisibilityState();
    }

    function testShowProperty(propertyType) {
        if (filteredConfigModel.showParagraphProperties && propertyType === TextPropertyConfigModel.Character) {
            return false;
        } else if (!filteredConfigModel.showParagraphProperties && propertyType === TextPropertyConfigModel.Paragraph) {
            return false;
        }
        return true;
    }

    function updateProperties() {
        for (var i = 0; i < propertyWidgetModel.count; i++) {
            if (testShowProperty(propertyWidgetModel.get(i).propertyType)) {
                propertyWidgetModel.get(i).propertiesUpdated();
            } else {
                propertyWidgetModel.get(i).visible = false;
            }
        }
    }

    function updatePropertyVisibilityState() {
        for (var i = 0; i < propertyWidgetModel.count; i++) {
            let name = propertyWidgetModel.get(i).propertyName;
            propertyWidgetModel.get(i).defaultVisibilityState = configModel.defaultVisibilityState;
            propertyWidgetModel.get(i).visibilityState = configModel.visibilityStateForName(name);
            if (testShowProperty(propertyWidgetModel.get(i).propertyType)) {
                propertyWidgetModel.get(i).setVisibleFromProperty();
            } else {
                propertyWidgetModel.get(i).visible = false;
            }
        }
    }

    function isVisible(name) {
        for (var i = 0; i < propertyWidgetModel.count; i++) {
            let prop = propertyWidgetModel.get(i);
            if (name === prop.propertyName) {
                return prop.visible;
            }
        }
    }

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
                }
                Direction {
                }
                TextIndent{
                }
                TextAlign{
                }
                DominantBaseline {
                }
                WhiteSpace {
                }
                UnderlinePosition {
                }
                HangingPunctuation {
                }
                TabSize {
                }
                TextRendering {}
                FontSize {
                }
                FontSizeAdjust {
                }

                FontFamily {
                }
                FontStyle {
                }
                LetterSpacing {
                }
                WordSpacing {
                }
                LineHeight {
                }
                LineBreak {
                }
                WordBreak {
                }
                TextTransform {
                }
                TextDecoration {
                }
                OTLigatures {
                }
                OTPosition {
                }
                OTNumeric {
                }
                OTCaps {
                }
                OTEastAsian {
                }
                OpenTypeFeatureSettings {
                }
                FontKerning {
                }
                BaselineShift {
                }
                AlignmentBaseline {
                }
                Language {
                }
            }
        }
    }

    ComboBox {
        id: addPropertyCmb;
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
}
