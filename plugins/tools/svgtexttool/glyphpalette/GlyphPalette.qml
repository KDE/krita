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
            textColor: palette.text;
            fillColor: palette.base;
            fontFamilies: root.fontFamilies;
            fontSize: root.fontSize;
            fontStyle: root.fontStyle;
            fontWeight: root.fontWeight;
            openTypeFeatures: model.openType;
            text: model.display;
            padding: height/8;
            clip: true;

            width: charMap.cellWidth;
            height: charMap.cellHeight;

            property bool currentItem: GridView.isCurrentItem;
            Rectangle {
                anchors.fill: parent;
                color: "transparent";
                border.color: parent.currentItem? palette.highlight: palette.alternateBase;
                border.width: parent.currentItem? 2: 1;
            }
            Rectangle {
                anchors.top: parent.top;
                anchors.left: parent.left;
                color: palette.text;
                opacity: 0.3;
                layer.enabled: true
                width: childrenRect.width;
                height: childrenRect.height;
                Text {
                    padding: 2;
                    text: model.glyphLabel;
                    color: palette.base;
                    font.pointSize: 9;
                }
                visible: glyphMouseArea.containsMouse;
            }
            Rectangle {
                anchors.top: parent.top;
                anchors.right: parent.right;
                width: 8;
                height: 8;
                radius: 4;
                color: palette.text;
                opacity: 0.3;
                visible: model.childCount > 1;
            }

            MouseArea {
                anchors.fill: parent;
                id: glyphMouseArea
                onClicked: {
                    parent.GridView.view.currentIndex = index;
                    console.log(model.toolTip)
                }
                hoverEnabled: true;
                ToolTip.text: model.toolTip;
                ToolTip.visible: containsMouse;
                ToolTip.delay: 1000;
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
                property var defaultIndex: modelIndex(-1);
                onRIndexChanged: {
                    rootIndex = defaultIndex;
                    rootIndex = modelIndex(root.currentIndex);
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
