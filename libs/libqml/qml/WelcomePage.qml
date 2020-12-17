/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.3
import org.krita.sketch 1.0
import org.krita.sketch.components 1.0
import "panels"

Page {
    property string pageName: "WelcomePage"

    Rectangle {
        anchors.fill: parent;
        color: Settings.theme.color("pages/welcome/background");
    }

//    Header {
//        id: header;

//        anchors {
//            top: parent.top;
//            left: parent.left;
//            right: parent.right;
//        }

//        text: (window.applicationName !== undefined) ? window.applicationName : "KRITA SKETCH";

//        leftArea: Image {
//            width: Constants.GridWidth * 0.5;
//            height: parent.height;
//            source: Settings.theme.image("kritasketch.png");
//            fillMode: Image.PreserveAspectFit;

//            sourceSize.width: width;
//        }

//        rightArea: [
//            Button {
//                id: minimizeButton;

//                anchors.verticalCenter: parent.verticalCenter;
//                width: Constants.GridWidth * 0.75;
//                height: Constants.GridHeight * 0.75;

//                tooltip: "Minimize";

//                image: Settings.theme.icon("minimize");
//                onClicked: Krita.Window.minimize();
//            },
//            Button {
//                id: closeButton;

//                anchors.verticalCenter: parent.verticalCenter;
//                width: Constants.GridWidth * 0.75;
//                height: Constants.GridHeight * 0.75;

//                tooltip: "Close";

//                image: Settings.theme.icon("close");
//                onClicked: Krita.Window.close();
//            }
//        ]

//        Image {
//            anchors.fill: parent;
//            source: Settings.theme.image("header_krita_sketch_light.png");
//        }
//    }

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
                    color: Settings.theme.color("pages/welcome/open/header/start");
                }


                GradientStop {
                    position: 1
                    color: Settings.theme.color("pages/welcome/open/header/stop");
                }
            }


            Label {
                anchors {
                    left: parent.left;
                    leftMargin: Constants.DefaultMargin;
                    verticalCenter: parent.verticalCenter;
                }
                text: "Recent Images";
                font: Settings.theme.font("title");
                color: Settings.theme.color("pages/welcome/open/header/text");
            }

            Shadow { width: parent.width; height: Constants.GridHeight / 8; anchors.top: parent.bottom; }
        }

        Rectangle {
            height: Constants.GridHeight;
            width: parent.width / 3;

            gradient: Gradient {
                GradientStop {
                    position: 0
                    color: Settings.theme.color("pages/welcome/create/header/start");
                }


                GradientStop {
                    position: 1
                    color: Settings.theme.color("pages/welcome/create/header/stop");
                }
            }

            Label {
                anchors {
                    left: parent.left;
                    leftMargin: Constants.DefaultMargin;
                    verticalCenter: parent.verticalCenter;
                }
                text: "Create New";
                font: Settings.theme.font("title");
                color: Settings.theme.color("pages/welcome/create/header/text");
            }

            Shadow { width: parent.width; height: Constants.GridHeight / 8; anchors.top: parent.bottom; }
        }

        Rectangle {
            height: Constants.GridHeight;
            width: parent.width / 3;

            gradient: Gradient {
                GradientStop {
                    position: 0
                    color: Settings.theme.color("pages/welcome/news/header/start");
                }


                GradientStop {
                    position: 1
                    color: Settings.theme.color("pages/welcome/news/header/stop");
                }
            }

            Label {
                anchors {
                    left: parent.left;
                    leftMargin: Constants.DefaultMargin;
                    verticalCenter: parent.verticalCenter;
                }
                text: "Community News";
                font: Settings.theme.font("title");
                color: Settings.theme.color("pages/welcome/news/header/text");
            }

            Shadow { width: parent.width; height: Constants.GridHeight / 8; anchors.top: parent.bottom; }
        }
    }

    Row {
        id: listsRow;
        anchors.bottom: parent.bottom;
        width: parent.width;

        RecentFilesList {
            width: parent.width / 3 - 4;
            height: Constants.GridHeight * (Constants.GridRows - 3);

            onClicked: d.openImage(file);
        }

        Divider { height: Constants.GridHeight * (Constants.GridRows - 3); }

        NewImageList {
            width: parent.width / 3 - 8;
            height: Constants.GridHeight * (Constants.GridRows - 3);
            onClicked: d.createNewImage(options);
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

        function createNewImage(options) {
            if(options !== undefined) {
                if(options.template !== undefined) {
                    Settings.currentFile = Krita.ImageBuilder.createImageFromTemplate(options);
                    Settings.temporaryFile = true;
                } else if(options.source === undefined) {
                    Settings.currentFile = Krita.ImageBuilder.createBlankImage(options);
                    Settings.temporaryFile = true;
                } else if(options.source == "clipboard") {
                    Settings.currentFile = Krita.ImageBuilder.createImageFromClipboard();
                    Settings.temporaryFile = true;
                }
            } else {
                pageStack.push(customImagePage);
            }
        }

        function openImage(file) {
            if(file !== "") {
                Settings.currentFile = file;
            } else {
                pageStack.push(openImagePage);
            }
        }
    }

    Connections {
        target: Settings;

        onCurrentFileChanged: if (!d.mainPageActive) {
            d.mainPageActive = true;
            pageStack.push( main );
        }
    }
    Component { id: main; MainPage { } }
    Component { id: help; HelpPage { } }
    Component { id: openImagePage; OpenImagePage { onFinished: { pageStack.pop(); d.openImage(file); } } }
    Component { id: customImagePage; CustomImagePage { onFinished: d.createNewImage(options); } }

}
