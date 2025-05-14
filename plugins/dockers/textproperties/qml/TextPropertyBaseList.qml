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

    ListModel {
        id: propertyList;
    }

    ListModel {
        id: filteredPropertyList;
    }

    TextPropertyConfigFilterModel {
        id : filteredConfigModel;
        sourceModel: textPropertyConfigModel;
        showParagraphProperties: propertyBaseList.propertyType === TextPropertyConfigModel.Paragraph;
        onShowParagraphPropertiesChanged: fillPropertyList();
        filterCaseSensitivity: Qt.CaseInsensitive;
    }

    Component.onCompleted: fillPropertyList();

    function fillPropertyList() {
        for (var i = 0; i < propertyWidgetModel.count; i++) {
            let prop = propertyWidgetModel.get(i);
            textPropertyConfigModel.addProperty(prop.propertyName,
                                                prop.propertyType,
                                                prop.propertyTitle,
                                                prop.toolTip,
                                                prop.searchTerms,
                                                );
            textPropertyConfigModel.loadFromConfiguration();
            console.log("property show", filteredConfigModel.showParagraphProperties);
            if (filteredConfigModel.showParagraphProperties && prop.propertyType === TextPropertyConfigModel.Character) {
                propertyWidgetModel.get(i).visible = false;
            } else if (!filteredConfigModel.showParagraphProperties && prop.propertyType === TextPropertyConfigModel.Paragraph) {
                propertyWidgetModel.get(i).visible = false;
            }
        }
    }

    function updateProperties() {
        for (var i = 0; i < propertyWidgetModel.count; i++) {
            if (propertyWidgetModel.get(i).propertyType === propertyType ||
                    propertyWidgetModel.get(i).propertyType === TextPropertyBase.Mixed) {
                propertyWidgetModel.get(i).propertiesUpdated();
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
            propertyBaseList.enableProperty(name);
            popup.close();
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

        popup: Popup {
            y: addPropertyCmb.height - 1;
            width: addPropertyCmb.width;
            height: contentHeight;
            padding: 2;

            palette: cmbPalette.palette;

            contentItem: ColumnLayout {
                clip: true;
                width: parent.width;
                ListView {
                    clip: true;
                    Layout.preferredHeight: contentHeight;
                    Layout.maximumHeight: 300;
                    Layout.fillWidth: true;
                    model: addPropertyCmb.popup.visible ? addPropertyCmb.delegateModel : null
                    currentIndex: addPropertyCmb.highlightedIndex;

                    ScrollBar.vertical: ScrollBar {
                    }
                }
                MenuSeparator {
                    Layout.fillWidth: true;
                    Layout.preferredHeight: implicitHeight;
                }
                TextField {
                    id: propertySearch;
                    placeholderText: i18nc("@info:placeholder", "Search...");
                    Layout.fillWidth: true;
                    Layout.preferredHeight: implicitHeight;
                    onTextChanged: filteredConfigModel.setFilterRegularExpression(text);
                }
            }
        }
    }
}
