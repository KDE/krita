/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12
import org.krita.flake.text 1.0
import org.krita.components 1.0 as Kis

CollapsibleGroupProperty {
    propertyTitle: i18nc("@label", "Font Family");
    propertyName: "font-family";
    propertyType: TextPropertyConfigModel.Character;
    visibilityState: TextPropertyConfigModel.AlwaysVisible;
    toolTip: i18nc("@info:tooltip",
                   "Font family allows selecting a list of fonts that should be used for the current text. The first font family is the primary font used, while each font family after that is used for fallback.");
    searchTerms: i18nc("comma separated search terms for the font-family property, matching is case-insensitive",
                       "font-family, typeface, font, fallback");

    property var fontFamilies: [];

    onPropertiesUpdated: {
        blockSignals = true;
        fontFamilies = properties.fontFamilies;
        propertyState = [properties.fontFamiliesState];
        setVisibleFromProperty();
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
    }

    titleItem: RowLayout {
        width: parent.width;
        height: childrenRect.height;
        Label {
            id: propertyTitleLabel;
            text: propertyTitle;
            verticalAlignment: Text.AlignVCenter;
            elide: Text.ElideRight;
            Layout.maximumWidth: contentWidth;

        }
        FontResourceDropdown {
            id: mainFamilyCmb;
            Layout.fillWidth: true;
            onActivated: {
                if (fontFamilies.length >0) {
                    fontFamilies[0] = text;
                } else {
                    fontFamilies = [ text ];
                }
            }
            Kis.ThemedControl {
                id: mainCmbPalette;
            }
            palette: mainCmbPalette.palette;
            function updateCurrentIndex() {
                var name = mainWindow.wwsFontFamilyName(fontFamilies[0]);
                if (name !== modelWrapper.resourceFilename) {
                    modelWrapper.currentTag = 0;
                    modelWrapper.currentIndex = -1;
                }
                modelWrapper.setResourceToFileName(name)
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
                        Layout.fillWidth: true;
                        onActivated: {
                            fontFamilies[fontListDelegate.dIndex] = text;
                        }
                        Kis.ThemedControl {
                            id: fontCmbPalette;
                        }
                        palette: fontCmbPalette.palette;
                        Component.onCompleted: {
                            if (fontListDelegate.dIndex === 0) {
                                modelWrapper = mainFamilyCmb.modelWrapper;
                            } else {
                                updateCurrentIndex();
                            }
                        }
                        function updateCurrentIndex() {
                            modelWrapper.setResourceToFileName(mainWindow.wwsFontFamilyName(fontFamilies[fontListDelegate.dIndex]))
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

