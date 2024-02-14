/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQml.Models 2.12
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.15
import org.krita.tools.text 1.0

Rectangle {
    id: root;
    anchors.fill: parent;
    property string titleText;
    property QtObject model;
    property alias currentIndex: charMap.currentIndex;
    property var fontFamilies: [];
    property double fontSize :10.0;
    property int fontWeight : 400;
    property int fontStyle;
    SystemPalette {
        id: palette;
        colorGroup: SystemPalette.Active
    }
    color: palette.window

    Text {
        id: title;
        text: titleText;
        color: palette.text;

        anchors.left: parent.left;
        anchors.right: parent.right;
        anchors.top: parent.top;
    }

    TabBar {
        id: tabs;
        anchors.left: parent.left;
        anchors.right: parent.right;
        anchors.top: title.bottom;

        TabButton {
            text: "Glyph Alternates"
        }
        TabButton {
            text: "Character Map"
        }
    }

    Component {
        id: glyphDelegate;
        SvgTextLabel {
            textColor: palette.windowText;
            fillColor: palette.base;
            fontFamilies: root.fontFamilies;
            fontSize: root.fontSize;
            fontStyle: root.fontStyle;
            fontWeight: root.fontWeight;
            openTypeFeatures: model.openType;
            text: model.display;
            padding: height/8;

            width: charMap.cellWidth;
            height: charMap.cellHeight;

            property bool currentItem: GridView.isCurrentItem;
            Rectangle {
                anchors.fill: parent;
                color: "transparent";
                border.color: parent.currentItem? palette.highlight: palette.alternateBase;
            }
            MouseArea {
                anchors.fill: parent;
                onClicked: {
                    //parent.currentIndex = index;
                    console.log(model.toolTip)
                }
                hoverEnabled: true;
                ToolTip.text: model.toolTip;
                ToolTip.visible: containsMouse;
            }
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
                onRIndexChanged: {
                    console.log("start");
                    rootIndex = parentModelIndex()
                    rootIndex = modelIndex(root.currentIndex);
                    console.log(rootIndex , root.currentIndex);
                }

                delegate: glyphDelegate;
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
        GridView {
            id: charMap;
            model: root.model;
            focus: true;
            clip: true;

            cellWidth: (parent.width - charMapScroll.implicitBackgroundWidth)/8;
            cellHeight: cellWidth;

            delegate: glyphDelegate;

            ScrollBar.vertical: ScrollBar {
                id: charMapScroll;
            }

        }
    }
}
