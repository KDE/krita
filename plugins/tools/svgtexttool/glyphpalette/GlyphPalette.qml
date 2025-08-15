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
import org.krita.components 1.0 as Kis

Control {
    id: root;
    anchors.fill: parent;
    property QtObject model: glyphModel;
    property GlyphPaletteProxyModel charMapModel: charMapProxyModel;

    property alias currentIndex: glyphAlts.parentIndex;
    property var fontFamilies: [];
    property double fontSize :10.0;
    property double fontWeight : 400;
    property double fontWidth : 100;
    property int fontStyle: 0;
    property double fontSlant: 0.0;
    property var fontAxesValues: ({});
    property string language: "";

    Kis.ThemedControl {
        id: pal;
    }
    palette: pal.palette;
    background: Rectangle {
        color: root.palette.window;
    }

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
        GlyphPaletteAlts {
            id: glyphAlts;
            glyphModel: root.model;
            useCharMap: false;
            fontFamilies: root.fontFamilies;
            fontSize: root.fontSize;
            fontWeight: root.fontWeight;
            fontStyle: root.fontStyle;
            fontWidth: root.fontWidth;
            fontSlant: root.fontSlant;
            fontAxesValues: root.fontAxesValues;
            language: root.language;
            replace: true;
            Layout.fillHeight: true;
            Layout.fillWidth: true;
        }

        // Charmap.
        RowLayout {
            id: charMapLayout;
            Layout.fillHeight: true;
            Layout.fillWidth: true;
            spacing: 1;

            Frame {
                Layout.minimumWidth: 100;
                Layout.maximumWidth: 200;
                Layout.fillHeight: true;
                Layout.fillWidth: true;
                background: Rectangle {
                    color: root.palette.base;
                }
                ListView {
                    id: charMapFilter
                    model: root.charMapModel.blockLabels;
                    anchors.fill: parent;

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
                        palette: root.palette;
                    }
                    ScrollBar.vertical: ScrollBar {
                    }
                }
            }

            ColumnLayout {
                id: rightSide;
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

                Frame {
                    Layout.fillHeight: true;
                    Layout.fillWidth: true;
                    background: Rectangle {
                        color: root.palette.base;
                    }
                    padding:0;
                    GridView {
                        id: charMap;
                        model: root.charMapModel;
                        focus: true;
                        clip: true;
                        anchors.fill: parent;


                        cellWidth: (width - charMapScroll.implicitBackgroundWidth)/8;
                        cellHeight: cellWidth;

                        delegate: GlyphDelegate {
                            fillColor: "transparent";
                            fontFamilies: root.fontFamilies;
                            fontSize: root.fontSize;
                            fontStyle: root.fontStyle;
                            fontWeight: root.fontWeight;
                            language: root.language;
                            fontWidth: root.fontWidth;
                            fontSlant: root.fontSlant;
                            fontAxesValues: root.fontAxesValues === undefined? {}: root.fontAxesValues;
                            onGlyphDoubleClicked: (index, mouse)=> {
                                                      mainWindow.slotInsertRichText(index, -1, false, true)
                                                  };
                            onGlyphClicked: (index, mouse)=> {
                                                if (model.childCount > 1){
                                                    var entryPos = mapToItem(root, 0+width, 0);
                                                    mainWindow.slotShowPopupPalette(index, entryPos.x, entryPos.y, width, height)
                                                }
                                            }
                        }

                        ScrollBar.vertical: ScrollBar {
                            id: charMapScroll;
                        }
                    }
                }
            }
        }
    }
}
