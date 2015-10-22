/* This file is part of the KDE project
 * Copyright (C) 2012 Boudewijn Rempt <boud@kogmbh.com>
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
import org.krita.sketch 1.0 as Krita
import org.krita.sketch.components 1.0

Page {
    id: base;
    signal finished(string file);

    Rectangle {
        anchors.fill: parent;
        color: Settings.theme.color("pages/open/background");
    }

    Header {
        id: header;

        anchors {
            top: parent.top;
            left: parent.left;
            right: parent.right;
        }

        text: "Open Image";

        background: "images/header_krita_sketch.png";

        leftArea: Button {
            width: Krita.Constants.GridWidth;
            height: Krita.Constants.GridHeight;
            image: Settings.theme.icon("back");
            onClicked: pageStack.pop();
        }

        rightArea: Button {
            width: Krita.Constants.GridWidth;
            height: Krita.Constants.GridHeight;
            image: Settings.theme.icon("up");
            onClicked: view.model.path = view.model.parentFolder;
        }

        Label {
            id: location;

            anchors.bottom: parent.bottom;
            anchors.horizontalCenter: parent.horizontalCenter;

            color: Settings.theme.color("pages/open/location");
            text: view.model.path;
        }
    }

    Shadow {
        anchors {
            top: header.bottom;
            left: parent.left;
            right: parent.right;
        }
        z: 5;
    }

    GridView {
        id: view;
        function startNavigation(path) {
            navigating = true;
            model.path = path;
            navigationTimer.start();
        }
        Timer {
            id: navigationTimer;
            interval: 150; running: false; repeat: false;
            onTriggered: view.navigating = false;
        }
        property bool navigating: false;
        anchors {
            top: header.bottom;
            left: parent.left;
            right: parent.right;
            bottom: parent.bottom;
        }

        // TODO: get filter from code, to get all supported types
        model: Krita.FileSystemModel { filter: "*.png *.jpg *.jpeg *.bmp *.kra *.psd *.ora *.tif *.tiff *.exr" }
        delegate: delegate;

        cellWidth: Krita.Constants.GridWidth * 4;
        cellHeight: Krita.Constants.GridHeight * 1.75;

        clip: true;

        ScrollDecorator { }
    }

    Component {
        id: delegate;

        ListItem {
            id: delegateBase;

            width: GridView.view.cellWidth;

            property bool directory: model.fileType === "inode/directory";

            imageShadow: directory ? false : true;
            image.source: directory ? Settings.theme.icon("fileopen-black") : model.icon;
            image.fillMode: directory ? Image.PreserveAspectFit : Image.PreserveAspectCrop;
            image.smooth: true;

            title: model.fileName;
            description: model.fileType !== "inode/directory" ? model.date : "";

            onClicked: {
                if ( GridView.view.navigating ) {
                    return;
                }
                if ( model.fileType === "inode/directory" ) {
                    view.startNavigation(model.path);
                } else {
                    base.finished(model.path);
                }
            }
        }
    }
}
