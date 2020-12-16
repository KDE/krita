/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
