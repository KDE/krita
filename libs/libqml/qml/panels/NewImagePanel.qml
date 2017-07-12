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
import org.krita.sketch.components 1.0

Item {
    id: base;
    property bool collapsed: true;

    enabled: !collapsed;

    signal clicked(variant options);

    Rectangle {
        id: panel;
        width: parent.width;
        height: parent.height;
        color: Settings.theme.color("panels/newImage/background");
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
                    color: Settings.theme.color("panels/newImage/header/start");
                }


                GradientStop {
                    position: 1
                    color: Settings.theme.color("panels/newImage/header/stop");
                }
            }

            Rectangle {
                anchors {
                    bottom: parent.bottom;
                    left: parent.left;
                    right: parent.right;
                }
                height: Constants.DefaultMargin;
                color: Settings.theme.color("panels/newImage/header/stop");
            }


            Shadow { width: parent.width; height: Constants.GridHeight / 8; anchors.top: parent.bottom;}

            Label {
                anchors {
                    left: parent.left;
                    leftMargin: Constants.DefaultMargin;
                    verticalCenter: parent.verticalCenter;
                }
                text: "Create New";
                font: Settings.theme.font("panelHeader");
                color: Settings.theme.color("panels/newImage/header/text");
            }
        }

        NewImageList {
            anchors.top: header.bottom;
            width: parent.width;
            height: Constants.GridHeight * 9;

            onClicked: base.clicked(options);
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
