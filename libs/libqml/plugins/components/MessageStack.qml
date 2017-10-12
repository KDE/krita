/* This file is part of the KDE project
 * Copyright (C) 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

import QtQuick 2.3
import org.krita.sketch 1.0

Item {
    id: base;
    height: messageStack.height;
    width: messageStack.width;
    function showMessage(message, iconName, url) {
        var block = messageBlock.createObject(messageStack);
        block.text = message;
        block.url = url;
    }

    Component {
        id: messageBlock;
        Item {
            id: message;
            property alias text: label.text;
            property alias url: button.url;
            SequentialAnimation on opacity {
                PauseAnimation { duration: (url === "") ? 500 : 5000; }
                NumberAnimation { to: 0; duration: 2000; }
                ScriptAction { script: message.destroy(); }
            }
            height: Constants.GridHeight;
            width: label.width + Constants.GridHeight / 2;
            Rectangle {
                anchors.fill: parent;
                color: Settings.theme.color("components/messageStack/background");
                border {
                    width: 2;
                    color: Settings.theme.color("components/messageStack/border");
                }
                radius: height / 2;
                opacity: 0.5;
            }
            Label {
                id: label;
                x: Constants.GridHeight / 4;
                anchors.verticalCenter: parent.verticalCenter;
                color: Settings.theme.color("components/messageStack/text");
                font: Settings.theme.font("title");
            }
            Rectangle {
                id: button;
                property string url: "";
                visible: (url !== "");
                anchors {
                    left: parent.right;
                    leftMargin: Constants.DefaultMargin;
                    verticalCenter: parent.verticalCenter;
                }
                height: parent.height;
                width: height;
                color: Settings.theme.color("components/messageStack/button/fill");
                border {
                    width: 2;
                    color: Settings.theme.color("components/messageStack/button/border");
                }
                radius: height / 2;
                opacity: 0.5;
            }
            Button {
                anchors.fill: button;
                visible: button.visible;
                text: "";
                color: "transparent";
                image: Settings.theme.icon("web");
                onClicked: Qt.openUrlExternally(button.url);
            }
        }
    }

    Column {
        id: messageStack;
        anchors {
            bottom: parent.bottom;
        }
        height: childrenRect.height;
    }
}
