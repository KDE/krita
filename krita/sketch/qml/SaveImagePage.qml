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

import QtQuick 1.1
import org.krita.sketch 1.0 as Krita
import "components"

Page {
    id: base;

    property Item view;
    property bool updateCurrentFile: true;

    Rectangle {
        anchors.fill: parent;
    }

    Header {
        id: header;

        anchors {
            top: parent.top;
            left: parent.left;
            right: parent.right;
        }

        text: "Save Image";
        background: "images/header_krita_sketch.png";

        leftArea: Button {
            width: Constants.GridWidth;
            height: Constants.GridHeight;
            highlightColor: Constants.Theme.HighlightColor;
            image: "images/svg/icon-back.svg";
            onClicked: pageStack.pop();
        }

        rightArea: Button {
            width: Constants.GridWidth;
            height: Constants.GridHeight;
            image: "images/svg/icon-up.svg";
            onClicked: view.model.path = view.model.parentFolder;
        }

        Label {
            id: location;

            anchors.bottom: parent.bottom;
            anchors.horizontalCenter: parent.horizontalCenter;

            color: "white";
            text: view.model.path;
        }
    }

    Image {
        anchors.top: header.bottom;
        anchors.left: parent.left;
        anchors.right: parent.right;
        source: "images/shadow-smooth.png";
        z: 5;
    }

    GridView {
        id: view;
        anchors {
            top: header.bottom;
            left: parent.left;
            right: parent.right;
            bottom: footer.top;
        }

        model: Krita.FileSystemModel { filter: "*.png *.jpg *.jpeg *.bmp *.kra *.psd *.ora *.tif *.tiff *.exr" }
        delegate: delegate;

        cellWidth: Constants.GridWidth * 4;
        cellHeight: Constants.GridHeight * 1.75;

        clip: true;

        MouseArea {
            anchors.fill: parent;
            onPressed: { parent.focus = true; mouse.accepted = false; }
        }

        ScrollDecorator { }
    }

    Rectangle {
        id: footer;

        anchors {
            bottom: parent.bottom;
            left: parent.left;
            right: parent.right;
        }

        height: Constants.GridHeight;
        color: "#000000"

        Row {
            anchors {
                fill: parent;
                leftMargin: Constants.GridWidth * 0.25;
                rightMargin: Constants.GridWidth * 0.25;
            }

            PanelTextField {
                id: fileNameField;

                anchors.verticalCenter: parent.verticalCenter;

                height: Constants.GridHeight * 0.75;
                width: Constants.GridWidth * 7.75;

                placeholder: "File Name";
            }

            Rectangle {
                anchors.bottom: parent.bottom;
                anchors.bottomMargin: 1;

                width: childrenRect.width;
                height: childrenRect.height + Constants.GridHeight * 0.5 - 1;

                radius: Constants.GridHeight * 0.25;

                color: "#000000"

                ExpandingListView {
                    id: fileType;

                    anchors.bottom: parent.bottom;
                    anchors.bottomMargin: Constants.GridHeight * 0.25;

                    height: Constants.GridHeight * 0.5;
                    width: Constants.GridWidth * 3.25;

                    expandedHeight: Constants.GridHeight * 3;
                    expandToTop: true;

                    model: ListModel {
                        ListElement { text: "Krita Image"; type: "kra"; mime: "application/x-krita"; }
                        ListElement { text: "OpenRaster Image"; type: "ora"; mime: "image/openraster"; }
                        ListElement { text: "PhotoShop Image"; type: "psd"; mime: "image/vnd.adobe.photoshop"; }
                        ListElement { text: "PNG Image"; type: "png"; mime: "image/png"; }
                        ListElement { text: "BMP Image"; type: "bmp"; mime: "image/bmp"; }
                        ListElement { text: "JPEG Image"; type: "jpg"; mime: "image/jpeg"; }
                    }
                }
            }

            Button {
                anchors.verticalCenter: parent.verticalCenter;
                height: Constants.GridHeight * 0.75;
                width: Constants.GridWidth * 0.5;

                image: "images/svg/icon-filesave.svg";

                onClicked: {
                    if ( fileNameField.text != "" ) {
                        var filePath = "%1/%2.%3".arg(view.model.path).arg(fileNameField.text).arg(fileType.model.get(fileType.currentIndex).type);
                        base.view.saveAs( filePath, fileType.model.get(fileType.currentIndex).mime );

                        if (base.updateCurrentFile) {
                            // The current file is updated by the saveAs call above
                            Settings.temporaryFile = false;
                        }
                        pageStack.pop();
                        savingDialog.show("Saving image to " + filePath);
                    }
                }
            }
        }
    }

    Component {
        id: delegate;

        ListItem {
            id: delegateBase;

            width: GridView.view.cellWidth;
            z: 10;

            image: model.fileType != "inode/directory" ? model.icon : "images/svg/icon-fileopen-black.svg";
            imageShadow: model.fileType != "inode/directory" ? true : false;
            imageFillMode: model.fileType != "inode/directory" ? Image.PreserveAspectCrop : Image.PreserveAspectFit;
            imageSmooth: model.fileType != "inode/directory" ? false : true;

            title: model.fileName;
            description: model.fileType != "inode/directory" ? model.date : "";

            onClicked: {
                if ( model.fileType == "inode/directory" ) {
                    GridView.view.model.path = model.path;
                } else {
                    fileNameField.text = model.fileName.substring(0, model.fileName.lastIndexOf('.'));
                }
            }
        }
    }
}
