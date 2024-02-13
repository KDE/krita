/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.0
import QtQuick.Controls 2.5
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

    ScrollView {
        anchors.left: parent.left;
        anchors.right: parent.right;
        anchors.top: title.bottom;
        anchors.bottom: parent.bottom;

        property int scrollWidth: ScrollBar.implicitWidth;
        GridView {
            id: charMap;
            model: root.model;
            focus: true;

            anchors.fill: parent;
            cellWidth: (root.width - parent.scrollWidth)/8;
            cellHeight: cellWidth;

            delegate:SvgTextLabel {
                textColor: palette.windowText;
                fillColor: palette.base;
                fontFamilies: root.fontFamilies;
                fontSize: root.fontSize;
                fontStyle: root.fontStyle;
                fontWeight: root.fontWeight;
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
                        charMap.currentIndex = index;
                        console.log(model.toolTip)
                    }
                    hoverEnabled: true;
                    ToolTip.text: model.toolTip;
                    ToolTip.visible: containsMouse;
                }
            }


        }
    }
}
