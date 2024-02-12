/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls 2.5

Rectangle {
    id: root;
    anchors.fill: parent;
    property string titleText;
    property QtObject model;
    property alias currentIndex: charMap.currentIndex;
    SystemPalette {
        id: palette;
        colorGroup: SystemPalette.Active
    }
    color: palette.window

    Text {
        id: title
        text: titleText
        color: palette.windowText
        anchors.left: parent.left;
        anchors.right: parent.right;
        anchors.top: parent.top;
    }

    ScrollView {
        anchors.left: parent.left;
        anchors.right: parent.right;
        anchors.top: title.bottom;
        anchors.bottom: parent.bottom;
        GridView {
            id: charMap;
            model: root.model;
            focus: true;

            anchors.fill: parent;
            cellWidth: 25;
            cellHeight: 25;

            delegate: Rectangle {
                width: charMap.cellWidth;
                height: charMap.cellHeight;

                color: palette.base
                border.color: GridView.isCurrentItem? palette.highlight: palette.alternateBase;

                Text {
                    clip: true;
                    text: model.display
                    color: palette.windowText
                    anchors.centerIn: parent;
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
