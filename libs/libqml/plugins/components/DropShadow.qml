/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.3
import org.krita.sketch 1.0

Item {
    id: base;
    property real size: Constants.DefaultMargin * 0.666;

    Image {
        anchors.right: parent.left;
        anchors.bottom: parent.top;

        width: parent.size;
        height: parent.size;

        opacity: 0.5;

        source: "../images/shadow-tl.png";

        fillMode: Image.Stretch;
    }

    Image {
        anchors.horizontalCenter: parent.horizontalCenter;
        anchors.bottom: parent.top;

        width: parent.width;
        height: parent.size;

        opacity: 0.5;

        source: "../images/shadow-tc.png";
        fillMode: Image.Stretch;
    }

    Image {
        anchors.left: parent.right;
        anchors.bottom: parent.top;

        width: parent.size;
        height: parent.size;

        opacity: 0.5;

        source: "../images/shadow-tr.png";
        fillMode: Image.Stretch;
    }

    Image {
        anchors.left: parent.right;
        anchors.verticalCenter: parent.verticalCenter;

        width: parent.size;
        height: parent.height;

        opacity: 0.5;

        source: "../images/shadow-rc.png";
        fillMode: Image.Stretch;
    }

    Image {
        anchors.left: parent.right;
        anchors.top: parent.bottom;

        width: parent.size;
        height: parent.size;

        opacity: 0.5;

        source: "../images/shadow-br.png";
        fillMode: Image.Stretch;
    }

    Image {
        anchors.horizontalCenter: parent.horizontalCenter;
        anchors.top: parent.bottom;

        width: parent.width;
        height: parent.size;

        opacity: 0.5;

        source: "../images/shadow-bc.png";
        fillMode: Image.Stretch;
    }

    Image {
        anchors.right: parent.left;
        anchors.top: parent.bottom;

        width: parent.size;
        height: parent.size;

        opacity: 0.5;

        source: "../images/shadow-bl.png";
        fillMode: Image.Stretch;
    }

    Image {
        anchors.right: parent.left;
        anchors.verticalCenter: parent.verticalCenter;

        width: parent.size;
        height: parent.height;

        opacity: 0.5;

        source: "../images/shadow-lc.png";
        fillMode: Image.Stretch;
    }
}
