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

import QtQuick 1.1
import "components"
import "panels"

Page {
    property string pageName: "WelcomePage"
    Header {
        id: header;

        anchors {
            top: parent.top;
            left: parent.left;
            right: parent.right;
        }

        text: (window.applicationName !== undefined) ? window.applicationName : "KRITA SKETCH";

        leftArea: Image {
            width: Constants.GridWidth * 0.5;
            height: parent.height;
            source: "images/kritasketch.png";
            fillMode: Image.PreserveAspectFit;

            sourceSize.width: width;
        }

        rightArea: [
            Button {
                id: minimizeButton;

                anchors.verticalCenter: parent.verticalCenter;
                width: Constants.GridWidth * 0.75;
                height: Constants.GridHeight * 0.75;

                image: "images/svg/icon-minimize.svg"
                highlightColor: Constants.Theme.HighlightColor;
                onClicked: Krita.Window.minimize();
            },
            Button {
                id: closeButton;

                anchors.verticalCenter: parent.verticalCenter;
                width: Constants.GridWidth * 0.75;
                height: Constants.GridHeight * 0.75;

                image: "images/svg/icon-close.svg"
                highlightColor: Constants.Theme.HighlightColor;
                onClicked: Krita.Window.close();
            }
        ]

        Image {
            anchors.fill: parent;
            source: "images/header_krita_sketch_light.png";
        }
    }

    Row {
        id: headersRow;
        anchors.top: header.bottom;
        width: parent.width;

        Rectangle {
            height: Constants.GridHeight;
            width: parent.width / 3;

            gradient: Gradient {
                GradientStop {
                    position: 0
                    color: "#707070"
                }


                GradientStop {
                    position: 1
                    color: "#565656"
                }
            }


            Label {
                anchors {
                    left: parent.left;
                    leftMargin: Constants.DefaultMargin;
                    verticalCenter: parent.verticalCenter;
                }
                text: "Recent Images";
                font.pixelSize: Constants.LargeFontSize;
                color: "white";
            }

            Image { source: "./images/shadow-smooth.png"; width: parent.width; height: Constants.GridHeight / 8; anchors.top: parent.bottom;}

        }

        Rectangle {
            height: Constants.GridHeight;
            width: parent.width / 3;

            gradient: Gradient {
                GradientStop {
                    position: 0
                    color: "#565656"
                }


                GradientStop {
                    position: 1
                    color: "#707070"
                }
            }

            Label {
                anchors {
                    left: parent.left;
                    leftMargin: Constants.DefaultMargin;
                    verticalCenter: parent.verticalCenter;
                }
                text: "Create New";
                font.pixelSize: Constants.LargeFontSize;
                color: "white";
            }

            Image { source: "./images/shadow-smooth.png"; width: parent.width; height: Constants.GridHeight / 8; anchors.top: parent.bottom;}

        }

        Rectangle {
            height: Constants.GridHeight;
            width: parent.width / 3;

            gradient: Gradient {
                GradientStop {
                    position: 0
                    color: "#707070"
                }


                GradientStop {
                    position: 1
                    color: "#565656"
                }
            }

            Label {
                anchors {
                    left: parent.left;
                    leftMargin: Constants.DefaultMargin;
                    verticalCenter: parent.verticalCenter;
                }
                text: "Community News";
                font.pixelSize: Constants.LargeFontSize;
                color: "white";
            }

            Image { source: "./images/shadow-smooth.png"; width: parent.width; height: Constants.GridHeight / 8; anchors.top: parent.bottom;}

        }
    }

    Row {
        id: listsRow;
        anchors.bottom: parent.bottom;
        width: parent.width;

        RecentFilesList {
            width: parent.width / 3 - 4;
            height: Constants.GridHeight * (Constants.GridRows - 3);

            onOpenClicked: pageStack.push( openImage );
            onItemClicked: baseLoadingDialog.visible = true;
        }

        Divider { height: Constants.GridHeight * (Constants.GridRows - 3); }

        NewImageList {
            width: parent.width / 3 - 8;
            height: Constants.GridHeight * (Constants.GridRows - 3);
            onClicked: baseLoadingDialog.visible = true;
        }

        Divider { height: Constants.GridHeight * (Constants.GridRows - 3); }

        NewsList {
            width: parent.width / 3 - 4;
            height: Constants.GridHeight * (Constants.GridRows - 3);
        }
    }

    QtObject {
        id: d;

        property bool mainPageActive: false;
    }

    Connections {
        target: Settings;

        onCurrentFileChanged: if (!d.mainPageActive) {
            d.mainPageActive = true;
            pageStack.push( main );
            baseLoadingDialog.hideMe();
        }
    }

    Component { id: main; MainPage { } }
    Component { id: help; HelpPage { } }
    Component { id: openImage; OpenImagePage { onItemClicked: baseLoadingDialog.visible = true; } }
}
