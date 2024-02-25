/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Controls 2.5
import org.krita.sketch 1.0

SvgTextLabel {
    id: glyphLabel;
    openTypeFeatures: model.openType;
    text: model.display;
    padding: height/8;
    clip: true;

    width: GridView.view.cellWidth;
    height: GridView.view.cellHeight;

    property bool currentItem: GridView.isCurrentItem;

    signal glyphClicked(int index);
    signal glyphDoubleClicked(int index);

    Rectangle {
        anchors.fill: parent;
        color: "transparent";
        border.color: parent.currentItem? sysPalette.highlight: sysPalette.alternateBase;
        border.width: parent.currentItem? 2: 1;
    }
    Rectangle {
        anchors.top: parent.top;
        anchors.left: parent.left;
        color: sysPalette.text;
        opacity: 0.3;
        layer.enabled: true
        width: childrenRect.width;
        height: childrenRect.height;
        Text {
            padding: 2;
            text: model.glyphLabel;
            color: sysPalette.base;
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
        color: sysPalette.text;
        opacity: 0.3;
        visible: model.childCount > 1;
    }

    MouseArea {
        anchors.fill: parent;
        id: glyphMouseArea
        onClicked: {
            parent.GridView.view.currentIndex = index;
            glyphClicked(index);
        }
        onDoubleClicked: {
            parent.GridView.view.currentIndex = index;
            glyphDoubleClicked(index);
        }
        hoverEnabled: true;
        ToolTip.text: model.toolTip;
        ToolTip.visible: containsMouse;
        ToolTip.delay: 1000;
    }
}
