/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQml.Models 2.15
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.15
import org.krita.tools.text 1.0

Rectangle {
    id: root;
    anchors.fill: parent;
    property QtObject model;
    property QtObject charMapModel;
    property QtObject charMapFilterModel;

    property int currentIndex: 0;
    property var fontFamilies: [];
    property double fontSize :10.0;
    property int fontWeight : 400;
    property int fontStyle;
    SystemPalette {
        id: sysPalette;
        colorGroup: SystemPalette.Active
    }
    color: sysPalette.window

    TabBar {
        id: tabs;
        anchors.top: parent.top;
        anchors.left: parent.left;
        anchors.right: parent.right;

        /// How are we going to translate this?
        TabButton {
            text: "Glyph Alternates"
        }
        TabButton {
            text: "Character Map"
        }
    }


    StackLayout {
        id: stack;
        currentIndex: tabs.currentIndex;
        anchors.left: parent.left;
        anchors.right: parent.right;
        anchors.top: tabs.bottom;
        anchors.bottom: parent.bottom;
        // Glyph alts.
        GridView {
            id: glyphAlts;
            model: DelegateModel {
                id: glyphAltModel
                model: root.model
                property alias rIndex: root.currentIndex;
                property var defaultIndex: modelIndex(-1);
                onRIndexChanged: {
                    rootIndex = defaultIndex;
                    rootIndex = modelIndex(root.currentIndex);
                }

                delegate: GlyphDelegate {
                    textColor: sysPalette.text;
                    fillColor: sysPalette.base;
                    fontFamilies: root.fontFamilies;
                    fontSize: root.fontSize;
                    fontStyle: root.fontStyle;
                    fontWeight: root.fontWeight;
                    onGlyphDoubleClicked: (index)=> {mainWindow.slotInsertRichText(root.currentIndex, index, true)};
                }

            }
            focus: true;
            clip: true;

            cellWidth: (parent.width - glyphAltScroll.implicitBackgroundWidth)/8;
            cellHeight: cellWidth;

            ScrollBar.vertical: ScrollBar {
                id: glyphAltScroll;
            }

        }

        // Charmap.
        RowLayout {
            Layout.fillHeight: true;
            Layout.fillWidth: true;
            spacing: 1;
            ListView {
                id: charMapFilter
                model: charMapFilterModel;
                Layout.minimumWidth: 100;
                Layout.maximumWidth: 200;
                Layout.fillHeight: true;
                Layout.fillWidth: true;

                onCurrentIndexChanged: {
                    mainWindow.slotChangeFilter(currentIndex);
                }

                clip: true;
                delegate: ItemDelegate {
                    width: parent.width;
                    text: model.display;
                    highlighted: ListView.isCurrentItem;
                    onClicked: {
                        charMapFilter.currentIndex = index;
                    }
                }
            }

            GridView {
                id: charMap;
                model: root.charMapModel;
                focus: true;
                clip: true;
                Layout.minimumWidth: 180;
                Layout.fillHeight: true;
                Layout.fillWidth: true;

                cellWidth: (width - charMapScroll.implicitBackgroundWidth)/8;
                cellHeight: cellWidth;

                delegate: GlyphDelegate {
                    textColor: sysPalette.text;
                    fillColor: sysPalette.base;
                    fontFamilies: root.fontFamilies;
                    fontSize: root.fontSize;
                    fontStyle: root.fontStyle;
                    fontWeight: root.fontWeight;
                    onGlyphDoubleClicked: (index)=> {mainWindow.slotInsertRichText(index)};
                    onGlyphClicked: (index)=> {
                                        if (model.childCount > 1){
                                            charMapAltGlyphs.open()
                                        }
                                    }
                }

                ScrollBar.vertical: ScrollBar {
                    id: charMapScroll;
                }

                Popup {
                    id: charMapAltGlyphs
                    x: 0
                    y: 0
                    width: charMap.width
                    height: charMap.cellHeight*3;
                    modal: true
                    focus: true
                    padding: 2;
                    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
                    GridView {
                        anchors.fill: parent;
                        clip: true;

                        cellWidth: (width - charMapAltGlyphsScroll.implicitBackgroundWidth)/8;
                        cellHeight: cellWidth;

                        ScrollBar.vertical: ScrollBar {
                            id: charMapAltGlyphsScroll;
                        }

                        model: DelegateModel {
                            model: charMap.model
                            property alias rIndex: charMap.currentIndex;
                            property var defaultIndex: modelIndex(-1);
                            onRIndexChanged: {
                                rootIndex = defaultIndex;
                                rootIndex = modelIndex(rIndex);
                            }

                            delegate: GlyphDelegate {
                                textColor: sysPalette.text;
                                fillColor: sysPalette.base;
                                fontFamilies: root.fontFamilies;
                                fontSize: root.fontSize;
                                fontStyle: root.fontStyle;
                                fontWeight: root.fontWeight;
                                onGlyphDoubleClicked: (index)=> {
                                                          mainWindow.slotInsertRichText(charMap.currentIndex, index);
                                                          charMapAltGlyphs.close()
                                                      };
                                onGlyphClicked: (index)=> {
                                                    mainWindow.slotInsertRichText(charMap.currentIndex, index);
                                                    charMapAltGlyphs.close()
                                                };
                            }

                        }
                    }
                }

            }
        }
    }
}
