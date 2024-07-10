/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12
import org.krita.flake.text 1.0

CollapsibleGroupProperty {
    propertyName: i18nc("@label", "Font Family");
    propertyType: TextPropertyBase.Character;
    toolTip: i18nc("@info:tooltip",
                   "Font family allows selecting a list of fonts that should be used for the current text. The first font family is the primary font used, while each font family after that is used for fallback.");
    searchTerms: i18nc("comma separated search terms for the font-family property, matching is case-insensitive",
                       "font-family, typeface, font, fallback");

    property var fontFamilies: [];
    property var fontFamilyModel : [];

    onPropertiesUpdated: {
        blockSignals = true;
        fontFamilies = properties.fontFamilies;
        visible = properties.fontFamiliesState !== KoSvgTextPropertiesModel.PropertyUnset;
        blockSignals = false;
    }

    onFontFamiliesChanged: {
        if (blockSignals) {
            if (fontFamilies.length >0) {
                mainFamilyCmb.currentIndex = mainFamilyCmb.find(fontFamilies[0]);
            }
            familyListView.model = fontFamilies;
        } else {
            properties.fontFamilies = fontFamilies;
        }
        mainWindow.slotUpdateStylesModel();
    }

    titleItem: ComboBox {
        id: mainFamilyCmb;
        model: fontFamilyModel;
        Layout.fillWidth: true;
        onActivated: {if (fontFamilies.length >0) {
                fontFamilies[0] = currentText;
            }
        }
        textRole: "name";

        delegate: ItemDelegate {
            required property var model;
            contentItem: Label {
                text: model.name;
                Layout.fillWidth: true;
                height:implicitHeight;
            }
            width: fontResourceView.listWidth;
            highlighted: mainFamilyCmb.highlightedIndex === model.index;
            background: Rectangle { color: highlighted? parent.palette.highlight:"transparent"; }
        }
        popup: Popup {
            y: mainFamilyCmb.height - 1;
            x: mainFamilyCmb.width - width;
            width: contentWidth;
            height: contentHeight;
            padding: 1;

            contentItem: FontResourceView {
                id: fontResourceView;
                fontModel: mainFamilyCmb.delegateModel;
                tagModel: fontTagModel;
                currentIndex: mainFamilyCmb.highlightedIndex;
            }
            palette: mainFamilyCmb.palette;
        }
        wheelEnabled: true;
    }

    onEnableProperty: properties.fontFamiliesState = KoSvgTextPropertiesModel.PropertySet;

    contentItem: GridLayout {
        columns: 2
        anchors.left: parent.left
        anchors.right: parent.right
        columnSpacing: columnSpacing;

        RevertPropertyButton {
            revertState: properties.fontFamiliesState;
            onClicked: properties.fontFamiliesState = KoSvgTextPropertiesModel.PropertyUnset;
        }

        ScrollView {
            id: fullFamilyList;
            Layout.fillWidth: true;
            Layout.preferredHeight: ItemDelegate.implicitHeight * 3;
            background: Rectangle {
                color: sysPalette.alternateBase;
                border.color: sysPalette.text;
                border.width: 1;
            }
            ListView {
                id: familyListView;
                anchors.fill: parent;
                model: []
                delegate: ItemDelegate {
                    text: modelData;
                    width: parent.width;
                }
            }
        }


    }
}

