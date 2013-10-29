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

PageStack {
    id: base;
    initialPage: createNewListPage;
    clip: true;

    signal clicked();

    Component { id: createNewListPage; Page {
        ListView {
            anchors.fill: parent;

            delegate: ListItem {
                width: ListView.view.width;

                title: model.name;
                image: model.image;
                imageShadow: false;

                gradient: Gradient {
                    GradientStop { position: 0; color: "#FBFBFB"; }
                    GradientStop { position: 0.4; color: "#F0F0F0"; }
                }

                onClicked: {
                    switch(model.bnrole) {
                        case "a5p": {
                            base.clicked();
                            Settings.currentFile = Krita.ImageBuilder.createBlankImage(600, 875, 150);
                            Settings.temporaryFile = true;
                        }
                        case "a5l": {
                            base.clicked();
                            Settings.currentFile = Krita.ImageBuilder.createBlankImage(875, 600, 150);
                            Settings.temporaryFile = true;
                        }
                        case "a4p": {
                            base.clicked();
                            Settings.currentFile = Krita.ImageBuilder.createBlankImage(1200, 1750, 150);
                            Settings.temporaryFile = true;
                        }
                        case "a4l": {
                            base.clicked();
                            Settings.currentFile = Krita.ImageBuilder.createBlankImage(1750, 1200, 150);
                            Settings.temporaryFile = true;
                        }
                        case "screen": {
                            base.clicked();
                            Settings.currentFile = Krita.ImageBuilder.createBlankImage(Krita.Window.width, Krita.Window.height, 72);
                            Settings.temporaryFile = true;
                        }
                        case "custom": {
                            pageStack.push( createNewPage );
                        }
                        case "clip": {
                            base.clicked();
                            Settings.currentFile = Krita.ImageBuilder.createImageFromClipboard();
                            Settings.temporaryFile = true;
                        }
                        case "webcam": {
                            base.clicked();
                            Settings.currentFile = Krita.ImageBuilder.createImageFromWebcam();
                            Settings.temporaryFile = true;
                        }
                    }
                }
            }

            model: ListModel {
                ListElement { bnrole: "a4p";    name: "Blank Image (A4 Portrait)"; image: "../images/svg/icon-A4portrait-black.svg" }
                ListElement { bnrole: "a4l";    name: "Blank Image (A4 Landscape)"; image: "../images/svg/icon-A4landscape-black.svg" }
//                 ListElement { bnrole: "a5p";    name: "Blank Image (A5 Portrait)"; image: "../images/svg/icon-A4portrait-black.svg" }
//                 ListElement { bnrole: "a5l";    name: "Blank Image (A5 Landscape)"; image: "../images/svg/icon-A4landscape-black.svg" }
                ListElement { bnrole: "screen"; name: "Blank Image (Screen Size)"; image: "../images/svg/icon-filenew-black.svg" }
                ListElement { bnrole: "custom"; name: "Custom Size"; image: "../images/svg/icon-filenew-black.svg" }
                ListElement { bnrole: "clip";   name: "From Clipboard"; image: "../images/svg/icon-fileclip-black.svg" }
//                 ListElement { bnrole: "webcam"; name: "From Camera"; image: "../images/svg/icon-camera-black.svg" }
            }

            ScrollDecorator { }
        }
    } }

    Component { id: createNewPage; Page {
        Column {
            anchors {
                fill: parent;
                topMargin: Constants.GridHeight * 0.35;
                leftMargin: Constants.GridWidth * 0.25;
                rightMargin: Constants.GridWidth * 0.25;
            }
            spacing: Constants.GridHeight * 0.75;
            Item {
                width: parent.width;
                height: Constants.GridHeight;
                Image {
                    id: titleImage;
                    anchors.left: parent.left;
                    anchors.verticalCenter: parent.verticalCenter;
                    source: "../images/svg/icon-filenew-black.svg"
                }
                Label {
                    anchors.left: titleImage.right;
                    anchors.leftMargin: Constants.DefaultMargin;
                    anchors.verticalCenter: parent.verticalCenter;
                    text: "Custom Size";
                    font.pixelSize: Constants.LargeFontSize;
                }
            }
            TextField {
                id: width;
                height: Constants.GridHeight;
                placeholder: "Width";
                validator: IntValidator{bottom: 0; top: 10000;}
                numeric: true;
                nextFocus: height;

                Component.onCompleted: text = Krita.Window.width;
            }
            TextField {
                id: height;
                height: Constants.GridHeight;
                placeholder: "Height"
                validator: IntValidator{bottom: 0; top: 10000;}
                numeric: true;
                nextFocus: resolution;

                Component.onCompleted: text = Krita.Window.height;
            }
            TextField {
                id: resolution;
                height: Constants.GridHeight;
                placeholder: "Resolution"
                text: "72";
                validator: IntValidator{bottom: 0; top: 600;}
                numeric: true;
            }
            //Item { width: parent.width; height: Constants.GridHeight; }
            Row {
                width: parent.width;
                Button {
                    width: parent.width / 2;
                    color: "transparent";
                    image: "../images/svg/icon-cancel-black.svg";
                    onClicked: pageStack.pop();
                }
                Button {
                    width: parent.width / 2;
                    color: "transparent";
                    image: "../images/svg/icon-apply-black.svg";
                    onClicked: {
                        if (width.acceptableInput && height.acceptableInput && resolution.acceptableInput) {
                            base.clicked();
                            Settings.currentFile = Krita.ImageBuilder.createBlankImage(parseInt(width.text), parseInt(height.text), parseInt(resolution.text));
                            Settings.temporaryFile = true;
                        }
                    }
                }
            }
        }
    } }
}
