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

ListView {
    id: base;
    clip: true;

    signal clicked();
    signal customImageClicked();

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
                    base.customImageClicked();
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
        ListElement { bnrole: "custom"; name: "Custom Image"; image: "../images/svg/icon-filenew-black.svg" }
        ListElement { bnrole: "clip";   name: "From Clipboard"; image: "../images/svg/icon-fileclip-black.svg" }
//                 ListElement { bnrole: "webcam"; name: "From Camera"; image: "../images/svg/icon-camera-black.svg" }
    }

    ScrollDecorator { }
}
