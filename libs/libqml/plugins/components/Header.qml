/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.3
import org.krita.sketch 1.0

Image {
    id: base;

    property alias text: title.text;
    property alias leftArea: left.children;
    property alias rightArea: right.children;

    property alias background: base.source;

    height: Constants.GridHeight * 2;
    width: parent.width;

    source: Settings.theme.image("header_krita_sketch.png");
    smooth: true;

    Row {
        id: left;

        anchors {
            left: parent.left;
            leftMargin: Constants.GridWidth * 0.5;
            top: parent.top;
            topMargin: Constants.GridHeight * 0.5;
            bottom: parent.bottom;
            bottomMargin: Constants.GridHeight * 0.5;
        }
    }

    Label {
        id: title;
        anchors.centerIn: parent;

        font.styleName: "Bold";
        font.pixelSize: Constants.HugeFontSize;
        color: Settings.theme.color("components/header");
    }

    Row {
        id: right;

        anchors {
            right: parent.right;
            rightMargin: Constants.GridWidth * 0.5;
            top: parent.top;
            topMargin: Constants.GridHeight * 0.5;
            bottom: parent.bottom;
            bottomMargin: Constants.GridHeight * 0.5;
        }
    }
}
