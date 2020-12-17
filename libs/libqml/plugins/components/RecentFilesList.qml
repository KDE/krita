/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.3
import org.krita.sketch 1.0

Item {
    id: base;
    signal clicked(string file);

    RecentImagesModel {
        id: recentImagesModel;
        recentFileManager: RecentFileManager;
    }

    ListView {
        id: view;
        anchors.left: parent.left;
        anchors.right: parent.right;
        anchors.top: parent.top;

        height: parent.height - Constants.GridHeight * 2;

        clip: true;

        delegate: ListItem {
            width: ListView.view.width;

            title: model.text;
            description: model.url;

            image.source: model.image;
            image.smooth: true;
            image.fillMode: Image.PreserveAspectCrop;
            imageShadow: true;

            gradient: Gradient {
                GradientStop { position: 0; color: Settings.theme.color("components/recentFilesList/start") }
                GradientStop { position: 0.4; color: Settings.theme.color("components/recentFilesList/stop"); }
            }

            onClicked: {
                base.clicked(model.url);
            }
        }

        model: recentImagesModel;

        ScrollDecorator { }
    }

    ListItem {
        anchors.top: view.bottom;

        title: "Open Image";
        image.source: Settings.theme.icon("fileopen-black");
        image.asynchronous: false;

        gradient: Gradient {
            GradientStop { position: 0; color: Settings.theme.color("components/recentFilesList/start") }
            GradientStop { position: 0.4; color: Settings.theme.color("components/recentFilesList/stop"); }
        }

        onClicked: base.clicked("");
    }
}
