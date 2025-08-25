/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12
import org.krita.flake.text 1.0
import org.krita.components 1.0 as Kis

TextPropertyBase {
    id: root;
    propertyTitle: i18nc("@label", "OpenType Features");
    propertyName: "ot-features";
    propertyType: TextPropertyConfigModel.Character;
    toolTip: i18nc("@info:tooltip",
                   "Configure Open Type features.");
    searchTerms: i18nc("comma separated search terms for the font-feature-settings property, matching is case-insensitive",
                       "font-feature-settings, OpenType");

    OpenTypeFeatureModel {
        id: fontFeatureModel;
        onOpenTypeFeaturesChanged: {
            if (!blockSignals) {
                properties.fontFeatureSettings = openTypeFeatures;
            }
        }
    }

    Connections {
        target: properties;
        function onFontFeatureSettingsChanged() {
            updateFontFeatures();
            updateVisibility();
        }
        function onFontFamiliesChanged() {
            updateFontFeatureModel();
            updateVisibility();
        }
        function onFontFeatureSettingsStateChanged() {
            updateVisibility();
        }
    }
    onPropertiesChanged: {
        updateFontFeatureModel();
        updateVisibility();
    }

    function updateFontFeatureModel() {
        blockSignals = true;
        // Setting text properties model will also set the opentype features.
        fontFeatureModel.setFromTextPropertiesModel(properties);
        blockSignals = false;
    }

    function updateFontFeatures() {
        blockSignals = true;
        fontFeatureModel.openTypeFeatures = properties.fontFeatureSettings;
        blockSignals = false;
    }

    function updateVisibility() {
        propertyState = [properties.fontFeatureSettingsState];
        setVisibleFromProperty();
    }

    onEnableProperty: properties.fontFeatureSettingsState = KoSvgTextPropertiesModel.PropertySet;
    GridLayout {
        columns: 2;
        columnSpacing: columnSpacing;
        rowSpacing: columnSpacing;
        width: parent.width;

        RevertPropertyButton {
            revertState: properties.fontFeatureSettingsState;
            onClicked: properties.fontFeatureSettingsState = KoSvgTextPropertiesModel.PropertyUnset;
        }

        Label {
            text: propertyTitle;
            Layout.fillWidth: true;
        }

        Item {
            width: firstColumnWidth;
            height: 1;
        }

        ListView {
            id: featureView;
            Layout.fillWidth: true;
            Layout.preferredHeight: contentHeight;
            Layout.minimumHeight: noContentLabel.contentHeight;
            spacing: columnSpacing;

            Label {
                id: noContentLabel;
                text: i18n("No OpenType features are currently set.");
                wrapMode: Text.WordWrap;
                anchors.fill: parent;
                anchors.horizontalCenter: parent.horizontalCenter;
                visible: parent.count === 0;
                padding: columnSpacing
            }

            model: fontFeatureModel;
            delegate: RowLayout {
                id: activeFeatureDelegate;
                required property var model;
                required property int index;
                property string display: model.display;
                property string toolTip: model.toolTip;
                property string sample: model.sample;
                property string tag: model.tag;
                property var parameters: model.parameters;
                width: featureView.width;

                ComboBox {
                    id: activeFeatureControl;
                    Layout.fillWidth: true;
                    model: activeFeatureDelegate.parameters;
                    valueRole: "value";
                    textRole: "display";

                    currentIndex: activeFeatureDelegate.model.edit;

                    contentItem: OpenTypeFeatureDelegate {
                        display: activeFeatureControl.currentText;
                        toolTip: activeFeatureDelegate.toolTip;
                        sample: activeFeatureDelegate.sample;
                        tag: activeFeatureDelegate.tag;
                        fontFamilies: properties.fontFamilies;
                        fontWeight: properties.fontWeight;
                        fontStyle: properties.fontStyle.style;
                        fontSlant: properties.fontStyle.value;
                        fontWidth: properties.fontWidth;
                        fontAxesValues: properties.axisValues;
                        featureValue: activeFeatureDelegate.model.edit;
                        enableMouseEvents: false;
                    }

                    Kis.ToolTipBase {
                        text: activeFeatureDelegate.toolTip;
                        visible: activeFeatureControl.hovered;
                    }

                    delegate: OpenTypeFeatureDelegate {
                        width: ListView.view.width;
                        required property var modelData;
                        display: modelData.display;
                        toolTip: display;
                        sample: activeFeatureDelegate.sample;
                        tag: activeFeatureDelegate.tag;
                        fontFamilies: properties.fontFamilies;
                        fontWeight: properties.fontWeight;
                        fontStyle: properties.fontStyle.style;
                        fontSlant: properties.fontStyle.value;
                        fontWidth: properties.fontWidth;
                        fontAxesValues: properties.axisValues;
                        featureValue: modelData.value;
                        onFeatureClicked: {
                            activeFeatureControl.currentIndex = index;
                            activeFeatureControl.popup.close();
                        }
                    }

                    onCurrentValueChanged: {
                        activeFeatureDelegate.model.edit = currentValue;
                    }
                }


                ToolButton {
                    id: removeFeature;
                    icon.width: 22;
                    icon.height: 22;
                    icon.source: "qrc:///22_light_list-remove.svg"
                    onClicked: fontFeatureModel.removeFeature(parent.tag);
                    ToolTip.text: i18n("Remove feature.");
                    ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval;
                    ToolTip.visible: hovered;
                }
            }
        }

        Item {
            width: firstColumnWidth;
            height: 1;
        }
        ComboBox {
            id: cmbAvailableFeatures;

            property OpenTypeFeatureFilterModel availableFilter: OpenTypeFeatureFilterModel {
                sourceModel: fontFeatureModel.allFeatureModel();
                filterAvailable: true;
            }

            model: availableFilter;
            textRole: "display";
            valueRole: "tag";
            Layout.fillWidth: true;

            delegate: OpenTypeFeatureDelegate {
                required property var model;
                required property int index;
                display: model.display;
                toolTip: model.toolTip;
                sample: model.sample;
                width: ListView.view.width;
                tag: model.tag;
                fontFamilies: properties.fontFamilies;
                fontWeight: properties.fontWeight;
                fontStyle: properties.fontStyle.style;
                fontSlant: properties.fontStyle.value;
                fontWidth: properties.fontWidth;
                fontAxesValues: properties.axisValues;
                featureValue: 0;
                onFeatureClicked: (mouse) => {
                    cmbAvailableFeatures.currentIndex = index;
                    fontFeatureModel.addFeature(tag);
                    cmbAvailableFeatures.popup.close();
                }
            }

            contentItem: TextField {
                id: featureTxtEdit;
                verticalAlignment: Text.AlignVCenter
                font: cmbAvailableFeatures.font
                color: cmbAvailableFeatures.palette.text
                selectionColor: cmbAvailableFeatures.palette.highlight
                selectedTextColor: cmbAvailableFeatures.palette.highlightedText;
                placeholderText: i18nc("@info:placeholder", "Search...");

                property bool blockSignals: false;

                property OpenTypeFeatureFilterModel filterModel: OpenTypeFeatureFilterModel {
                    sourceModel: fontFeatureModel.allFeatureModel();
                    filterCaseSensitivity: Qt.CaseInsensitive;
                }

                selectByMouse: true;

                onTextChanged: {
                    if (text !== "") {
                        filterModel.setFilterRegularExpression(text);
                        completerPopup.open();
                    } else {
                        completerPopup.close();
                        filterModel.setFilterRegularExpression(text);
                    }
                }

                onAccepted: {
                    blockSignals = true;
                    let name = filterModel.firstValidTag();
                    if (typeof name != 'undefined') {
                        fontFeatureModel.addFeature(name);
                    }
                    finalize();
                    blockSignals = false;
                }

                function finalize() {
                    text = "";
                }

                Popup {
                    id: completerPopup;
                    y: -height;
                    x: 0;
                    padding: 0;
                    width: featureTxtEdit.width;
                    height: contentItem.implicitHeight;
                    property string highlightedTag;
                    contentItem: ListView {
                        model: featureTxtEdit.filterModel;
                        clip: true;
                        implicitHeight: Math.min(contentHeight, 300);
                        width: completerPopup.width;

                        ScrollBar.vertical: ScrollBar {
                        }

                        delegate: OpenTypeFeatureDelegate {
                            required property var model;
                            required property int index;
                            display: model.display;
                            toolTip: model.toolTip;
                            sample: model.sample;
                            width: ListView.view.width;
                            tag: model.tag;
                            fontFamilies: properties.fontFamilies;
                            fontWeight: properties.fontWeight;
                            fontStyle: properties.fontStyle.style;
                            fontSlant: properties.fontStyle.value;
                            fontWidth: properties.fontWidth;
                            fontAxesValues: properties.axisValues;
                            featureValue: 0;
                            onFeatureClicked: (mouse) => {
                                                  fontFeatureModel.addFeature(tag);
                                                  featureTxtEdit.finalize();
                                              }
                        }
                    }
                }
            }
        }
    }
}

