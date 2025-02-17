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
    property QtObject model: glyphModel;
    property GlyphPaletteProxyModel charMapModel: charMapProxyModel;

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

        TabButton {
            text: i18nc("@title:tab", "Glyph Alternates")
        }
        TabButton {
            text: i18nc("@title:tab", "Character Map")
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
                model: root.model;
                property alias rIndex: root.currentIndex;
                property var defaultIndex: modelIndex(-1);
                onRIndexChanged: {
                    // This first line is necessary to reset the model to the root index.
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
                model: root.charMapModel.blockLabels;
                Layout.minimumWidth: 100;
                Layout.maximumWidth: 200;
                Layout.fillHeight: true;
                Layout.fillWidth: true;

                onCurrentIndexChanged: {
                   root.charMapModel.blockFilter = currentIndex;
                }

                clip: true;
                delegate: ItemDelegate {
                    width: charMapFilter !== undefined? charMapFilter.width: 0;
                    text: modelData.name;
                    highlighted: ListView.isCurrentItem;
                    onClicked: {
                        charMapFilter.currentIndex = modelData.value;
                    }
                }
                ScrollBar.vertical: ScrollBar {
                }
            }

            ColumnLayout {
                Layout.minimumWidth: 180;
                Layout.fillHeight: true;
                Layout.fillWidth: true;



                TextField {
                    id: searchInput;
                    onTextEdited: {
                        root.charMapModel.searchText = text;
                    }
                    placeholderText: i18nc("@info:placeholder", "Search...");
                    hoverEnabled: true;
                    Layout.fillWidth: true;
                    Layout.preferredHeight: implicitHeight;
                    padding: 5;

                    ToolTip.text: i18nc("@info:tooltip",
                                        "Searches for characters that match the first letter of the string. Hex search is possible when prepended with \"U+\", (for example, U+0045 will search for 'E')");
                    ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
                    ToolTip.visible: hovered;
                }

                GridView {
                    id: charMap;
                    model: root.charMapModel;
                    focus: true;
                    clip: true;
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

                        palette: TabBar.palette;
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
}
