/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Controls 2.5
import org.krita.tools.text 1.0
import org.krita.components 1.0 as Kis

Control {
    id: root;

    width: GridView.view.cellWidth;
    height: GridView.view.cellHeight;

    property alias fontFamilies: glyphLabel.fontFamilies;
    property alias fontSize: glyphLabel.fontSize;
    property alias fontStyle: glyphLabel.fontStyle;
    property alias fontWeight: glyphLabel.fontWeight;
    property alias fontWidth: glyphLabel.fontWidth;
    property alias fontSlant: glyphLabel.fontSlant;
    property alias fontAxesValues: glyphLabel.fontAxesValues;
    property alias language: glyphLabel.language;
    property alias fillColor: glyphLabel.fillColor;
    property alias textColor: glyphLabel.textColor;

    required property var model;
    required property int index;

    Kis.ThemedControl {
        id: pal;
    }
    palette: pal.palette;

    background: Rectangle {
        color: hovered? root.palette.highlight: root.palette.base;
    }

    signal glyphClicked(int index, QtObject mouse);
    signal glyphDoubleClicked(int index, QtObject mouse);
    property bool currentItem: GridView.isCurrentItem;

    Kis.SvgTextLabel {
        id: glyphLabel;
        openTypeFeatures: model.openType;
        text: model.display;
        padding: height/8;
        anchors.fill: parent;
        clip: true;
        textColor: hovered? root.palette.highlightedText: root.palette.text;



        Rectangle {
            anchors.fill: parent;
            color: "transparent";
            border.color: parent.currentItem? root.palette.highlight: root.palette.mid;
            border.width: parent.currentItem? 2: 1;
        }
        Rectangle {
            anchors.top: parent.top;
            anchors.left: parent.left;
            color: root.palette.highlightedText;
            opacity: 0.6;
            layer.enabled: true
            width: childrenRect.width;
            height: childrenRect.height;
            Text {
                padding: 2;
                text: model.glyphLabel;
                color: root.palette.highlight;
                font.pointSize: 9;
            }
            visible: root.hovered;
        }
        Rectangle {
            anchors.top: parent.top;
            anchors.right: parent.right;
            width: 8;
            height: 8;
            radius: 4;
            color: parent.textColor;
            opacity: 0.3;
            visible: model.childCount > 1;
        }
    }
    MouseArea {
        anchors.fill: parent;
        id: glyphMouseArea
        onClicked: (mouse)=>{
                       root.GridView.view.currentIndex = index;
                       glyphClicked(index, mouse);
                   }
        onDoubleClicked: (mouse)=>{
                             root.GridView.view.currentIndex = index;
                             glyphDoubleClicked(index, mouse);
                         }
        ToolTip.text: model.toolTip;
        ToolTip.visible: containsMouse;
        ToolTip.delay: 1000;
    }
}
