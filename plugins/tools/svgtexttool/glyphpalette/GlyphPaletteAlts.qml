/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Controls 2.5
import QtQml.Models 2.15
import org.krita.tools.text 1.0
import org.krita.components 1.0 as Kis

Control {
    id: root;
    property QtObject glyphModel;
    property int parentIndex: 0;
    property int columns: 8;
    property bool replace: false;
    property bool useCharMap: true;

    property var fontFamilies: [];
    property double fontSize: 10.0;
    property double fontWeight: 400;
    property double fontWidth: 100;
    property int fontStyle: 0;
    property double fontSlant: 0.0;
    property var fontAxesValues;
    property string language: "";

    Kis.ThemedControl {
        id: pal;
    }
    palette: pal.palette;

    background: Rectangle {
        color: root.palette.base;
    }

    GridView {
        anchors.fill: parent;

        model: DelegateModel {
            id: glyphAltModel
            model: root.glyphModel;
            property alias parentIndex: root.parentIndex;
            property var defaultIndex: modelIndex(-1);
            onParentIndexChanged: {
                // This first line is necessary to reset the model to the root of the glyph model.
                rootIndex = defaultIndex;
                rootIndex = modelIndex(root.parentIndex);
            }

            delegate: GlyphDelegate {
                fillColor: "transparent";
                fontFamilies: root.fontFamilies;
                fontSize: root.fontSize;
                fontStyle: root.fontStyle;
                fontWeight: root.fontWeight;
                fontWidth: root.fontWidth;
                fontSlant: root.fontSlant;
                fontAxesValues: root.fontAxesValues === undefined? {}: root.fontAxesValues;
                language: root.language;
                onGlyphClicked: (index, mouse)=> {
                                    if (!root.replace) {
                                        mainWindow.slotInsertRichText(root.parentIndex, index, root.replace, root.useCharMap)
                                    }
                                };
                onGlyphDoubleClicked: (index, mouse)=> {mainWindow.slotInsertRichText(root.parentIndex, index, root.replace, root.useCharMap)};
            }
        }
        focus: true;
        clip: true;

        cellWidth: width/root.columns;
        cellHeight: cellWidth;

        ScrollBar.vertical: ScrollBar {
            id: glyphAltScroll;
        }
    }
}
