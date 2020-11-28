/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.3
import org.krita.sketch 1.0
import org.krita.sketch.components 1.0

Item {
    id: base;
    property bool collapsed: true;

    enabled: !collapsed;

    signal clicked(string file);

    Rectangle {
        id: panel;
        width: parent.width;
        height: parent.height;
        color: Settings.theme.color("panels/openImage/background");
        clip: true;
        radius: Constants.DefaultMargin;

        Rectangle {
            id: header;
            height: Constants.GridHeight;
            width: parent.width;
            z: 2;
            radius: Constants.DefaultMargin;

            gradient: Gradient {
                GradientStop {
                    position: 0
                    color: Settings.theme.color("panels/openImage/header/start");
                }


                GradientStop {
                    position: 1
                    color: Settings.theme.color("panels/openImage/header/stop");
                }
            }

            Rectangle {
                anchors.bottom: parent.bottom;
                anchors.left: parent.left;
                anchors.right: parent.right;
                height: Constants.DefaultMargin;
                color: Settings.theme.color("panels/openImage/header/stop");
            }

            Shadow { width: parent.width; height: Constants.GridHeight / 8; anchors.top: parent.bottom;}

            Label {
                anchors {
                    left: parent.left;
                    leftMargin: Constants.DefaultMargin;
                    verticalCenter: parent.verticalCenter;
                }
                text: "Open Image";
                font.pixelSize: Constants.LargeFontSize;
                color: Settings.theme.color("panels/openImage/header/text");
            }
        }

        RecentFilesList {
            anchors.top: header.bottom;
            anchors.topMargin: -Constants.GridHeight * 0.25;
            width: parent.width;
            height: Constants.GridHeight * 10.5 + 8;

            onClicked: base.clicked(file);
        }
    }

    states: State {
        name: "collapsed";
        when: base.collapsed;
        PropertyChanges { target: panel; x: -base.width; opacity: 0; }
    }

    transitions: Transition {
        NumberAnimation { properties: "x,opacity"; }
    }
}
