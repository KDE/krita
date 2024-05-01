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
    }

    titleItem: ComboBox {
        id: mainFamilyCmb;
        model: fontFamilyModel;
        Layout.fillWidth: true;
        onActivated: {if (fontFamilies.length >0) {
                fontFamilies[0] = currentText;
            }
        }
    }

    contentItem: GridLayout {
        columns: 2
        anchors.left: parent.left
        anchors.right: parent.right
        columnSpacing: columnSpacing;

        RevertPropertyButton {
            revertEnabled: properties.fontFamiliesState === KoSvgTextPropertiesModel.PropertySet;
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

