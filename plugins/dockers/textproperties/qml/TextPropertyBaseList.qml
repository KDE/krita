/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQml.Models 2.1

ColumnLayout {
    property int propertyType: TextPropertyBase.Character;

    ListModel {
        id: propertyList;
    }

    ListModel {
        id: filteredPropertyList;
    }

    Component.onCompleted: fillPropertyList();

    function fillPropertyList() {
        for (var i = 0; i < propertyWidgetModel.count; i++) {
            propertyWidgetModel.get(i).parentPropertyType = propertyType;
            if (propertyWidgetModel.get(i).propertyType === propertyType ||
                    propertyWidgetModel.get(i).propertyType === TextPropertyBase.Mixed) {
            propertyList.append({"name": propertyWidgetModel.get(i).propertyName,
                                    "toolTip": propertyWidgetModel.get(i).toolTip,
                                    "searchTerms": propertyWidgetModel.get(i).searchTerms,
                                    "item": propertyWidgetModel.get(i)});
            } else {
                propertyWidgetModel.get(i).visible = false;
            }
        }
        updateFilteredPropertyList();
    }

    signal filterChanged;

    onFilterChanged: {
        updateFilteredPropertyList();
    }

    function updateFilteredPropertyList() {
        filteredPropertyList.clear();
        for (var i = 0; i < propertyList.count; i++) {
            var searchText = propertySearch.text;
            if (searchText.length === 0) {
                filteredPropertyList.append(propertyList.get(i));
            } else {
                var searchTerms = propertyList.get(i).searchTerms.split(",");
                if (searchTerms.find(term => term.toLowerCase().includes(searchText.toLowerCase()))) {
                    filteredPropertyList.append(propertyList.get(i));
                }
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

    Frame {
        id: frame;
        Layout.fillHeight: true;
        Layout.fillWidth: true;
        clip: true;
        padding: 0;

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
                /*
                WhiteSpace {
                }
                UnderlinePosition {
                }*/
                HangingPunctuation {
                }
                TabSize {
                }
                FontSize {
                }
                FontFamily {
                    fontFamilyModel: fontFamiliesModel;
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
                /*
            OTLigatures {
            }
            OTPosition {
            }
            OTNumeric {
            }
            OTCaps {
            }
            OTEastAsian {
            }*/
                BaselineShift {
                }
                AlignmentBaseline {
                }
            }
        }
    }

    ComboBox {
        id: addPropertyCmb;
        Layout.fillWidth: true;
        Layout.minimumHeight: implicitHeight;
        model: filteredPropertyList;
        textRole: "name";
        displayText: i18nc("@label:listbox", "Add Property");
        onActivated: {
            model.get(currentIndex).item.enableProperty();
        }
        onPopupChanged: if (!addPropertyCmb.popup.visible) { propertySearch.text = ""; }

        signal searchChanged;
        onSearchChanged: parent.filterChanged();

        delegate: ItemDelegate {
            id: addPropertyDelegate;
            width: addPropertyCmb.width;
            required property string name;
            required property int index;
            required property QtObject item;
            required property string toolTip;
            contentItem: Label {
                enabled: addPropertyDelegate.enabled;
                text: addPropertyDelegate.name;
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
            }
            enabled: !item.visible;
            highlighted: addPropertyCmb.highlightedIndex === index;
            background: Rectangle { color: highlighted? parent.palette.highlight:"transparent"; }

            ToolTip.text: toolTip;
            ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
            ToolTip.visible: highlighted;
        }

        popup: Popup {
            y: addPropertyCmb.height - 1;
            width: addPropertyCmb.width;
            height: childrenRect.height;
            padding: 1;

            contentItem: ColumnLayout {
                clip: true;
                width: parent.width;
                ListView {
                    Layout.fillWidth: true;
                    Layout.fillHeight: true;
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
                    onTextChanged: addPropertyCmb.searchChanged();
                }
            }
        }
    }
}
