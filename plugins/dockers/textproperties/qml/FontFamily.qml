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
                mainFamilyCmb.updateCurrentIndex();
            }
            familyListView.model = fontFamilies;
        } else {
            properties.fontFamilies = fontFamilies;
        }
        mainWindow.slotUpdateStylesModel();
    }

    titleItem: RowLayout {
        width: parent.width;
        height: childrenRect.height;
        Label {
            id: propertyTitle;
            text: propertyName;
            verticalAlignment: Text.AlignVCenter
            color: sysPalette.text;
            elide: Text.ElideRight;
            Layout.maximumWidth: contentWidth;

        }
        FontResourceDropdown {
            id: mainFamilyCmb;
            sourceModel: fontFamilyModel;
            Layout.fillWidth: true;
            onActivated: {
                if (fontFamilies.length >0) {
                    fontFamilies[0] = currentText;
                } else {
                    fontFamilies = [ currentText ];
                }
            }
            Connections {
                target: mainFamilyCmb.modelWrapper;
                function onModelSortUpdated() { mainFamilyCmb.updateCurrentIndex() }
            }
            function updateCurrentIndex() {
                currentIndex = find(mainWindow.wwsFontFamilyName(fontFamilies[0]))
            }
        }

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
            Layout.preferredHeight: contentHeight;
            ListView {
                id: familyListView;
                anchors.fill: parent;
                model: []

                delegate: RowLayout {
                    id: fontListDelegate;
                    spacing: 5;
                    width: parent.width;
                    property int dIndex: index;
                    FontResourceDropdown {
                        id: fontCmb;
                        sourceModel: fontFamilyModel;
                        Layout.fillWidth: true;
                        onActivated: {
                            fontFamilies[fontListDelegate.dIndex] = currentText;
                        }
                        Component.onCompleted: {
                            if (fontListDelegate.dIndex === 0) {
                                modelWrapper = mainFamilyCmb.modelWrapper;
                            }
                            updateCurrentIndex();
                        }
                        Connections {
                            target: fontCmb.modelWrapper;
                            function onModelSortUpdated() { fontCmb.updateCurrentIndex() }
                        }
                        function updateCurrentIndex() {
                            currentIndex = find(mainWindow.wwsFontFamilyName(fontFamilies[fontListDelegate.dIndex]))
                        }
                    }
                    ToolButton {
                        id: removeFont;
                        icon.width: 22;
                        icon.height: 22;
                        icon.source: "qrc:///22_light_list-remove.svg"
                        onClicked: fontFamilies.splice(fontListDelegate.dIndex, 1);
                        ToolTip.text: i18n("Remove family");
                        ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval;
                        ToolTip.visible: hovered;
                    }
                }
                Label {
                    text: i18n("Family list is empty.");
                    wrapMode: Text.WordWrap;
                    anchors.fill: parent;
                    anchors.horizontalCenter: parent.horizontalCenter;
                    visible: parent.count === 0;
                }
            }
        }

        Item {
            width: 1;
            height: 1;
        }
        Button {
            id: addFamilyButtons;
            Layout.fillWidth: true;
            text: i18n("Add Fallback Family");
            onClicked: fontFamilies.push("sans-serif");
        }


    }
}

