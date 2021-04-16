/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Boudewijn Rempt <boud@kogmbh.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.3
import org.krita.sketch 1.0
import org.krita.sketch.components 1.0

Page {
    id: base;

    property Item view;
    property bool updateCurrentFile: true;

    signal finished(string file, string type);

    Rectangle {
        anchors.fill: parent;
        color: Settings.theme.color("pages/save/background");
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
            image: Settings.theme.icon("close");
            onClicked: mainWindow.hideFileSaveAsDialog();
        }

        rightArea: Button {
            width: Constants.GridWidth;
            height: Constants.GridHeight;
            image: Settings.theme.icon("up");
            onClicked: view.model.path = view.model.parentFolder;
        }

        Label {
            id: location;

            anchors.bottom: parent.bottom;
            anchors.horizontalCenter: parent.horizontalCenter;

            color: Settings.theme.color("pages/save/location");
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
        anchors {
            top: header.bottom;
            left: parent.left;
            right: parent.right;
            bottom: footer.top;
        }

        model: FileSystemModel { filter: "*.png *.jpg *.jpeg *.bmp *.kra *.psd *.ora *.tif *.tiff *.exr" }
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
        color: Settings.theme.color("pages/save/footer")

        Row {
            anchors {
                fill: parent;
                leftMargin: Constants.GridWidth * 0.25;
                rightMargin: Constants.GridWidth * 0.25;
            }
            spacing: Constants.DefaultMargin;

            PanelTextField {
                id: fileNameField;

                anchors.verticalCenter: parent.verticalCenter;

                height: Constants.GridHeight * 0.75;
                width: Constants.GridWidth * 7.25;

                placeholder: "File Name";
            }

            Rectangle {
                anchors.bottom: parent.bottom;

                width: childrenRect.width;
                height: fileType.state == "expanded" ?
                    Constants.GridHeight * 3.5:
                    Constants.GridHeight;

                radius: Constants.GridHeight * 0.25;

                color: Settings.theme.color("pages/save/footer");

                ExpandingListView {
                    id: fileType;

                    anchors.bottom: parent.bottom;
                    anchors.bottomMargin: Constants.GridHeight * 0.125;

                    height: Constants.GridHeight * 0.75;
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
                height: Math.min(Constants.GridHeight, Constants.GridWidth);
                width: height

                image: Settings.theme.icon("filesave");

                onClicked: {
                    if ( fileNameField.text != "" ) {
                        var filePath = "%1/%2.%3".arg(view.model.path).arg(fileNameField.text).arg(fileType.model.get(fileType.currentIndex).type);
                        if(Krita.fileExists(filePath)) {
                            confirmOverwrite.show();
                        }
                        else {
                            //base.finished( filePath, fileType.model.get(fileType.currentIndex).mime );
                            mainWindow.slotSaveAs(filePath, fileType.model.get(fileType.currentIndex).mime );
                        }
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

            property bool directory: model.icon === "inode/directory";

            imageShadow: directory ? false : true;
            image.source: directory ? Settings.theme.icon("fileopen-black") : model.icon;
            image.fillMode: directory ? Image.PreserveAspectFit : Image.PreserveAspectCrop;
            image.smooth: true;

            title: model.fileName;
            description: model.icon !== "inode/directory" ? model.date : "";

            onClicked: {
                if ( model.icon === "inode/directory" ) {
                    GridView.view.model.path = model.path;
                } else {
                    fileNameField.text = model.fileName.substring(0, model.fileName.lastIndexOf('.'));
                }
            }
        }
    }

    Dialog {
        id: confirmOverwrite;
        title: "File already exists";
        message: "A file with the name %1.%2 already exists in this folder. Do you wish to overwrite?".arg(fileNameField.text).arg(fileType.model.get(fileType.currentIndex).type);
        buttons: [ "Overwrite", "Cancel" ];
        onButtonClicked: {
            switch(button) {
            case 0:
                var filePath = "%1/%2.%3".arg(view.model.path).arg(fileNameField.text).arg(fileType.model.get(fileType.currentIndex).type);
                base.finished( filePath, fileType.model.get(fileType.currentIndex).mime );
                break;
            case 1:
                // do nothing, just dismiss dialog
                break;
            default:
                console.debug("Nope, shouldn't be here. How did you press a button that doesn't exist?");
                break;
            }
            confirmOverwrite.hide();
            pageStack.pop();
        }
    }
}
