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
import org.krita.sketch 1.0

Item {
    id: base;

    property alias title: dialogTitle.text;
    property alias message: dialogText.text;
    property variant buttons: null;
    property alias modalBackgroundColor: modalFill.color;
    property alias textAlign: dialogText.horizontalAlignment;
    property int progress: -1;

    property int defaultButton: 0;
    property int currentButton: defaultButton;

    signal buttonClicked(int button);
    signal canceled();

    function show(message) {
        if (message) {
            base.message = message;
        }
        base.opacity = 1;
    }

    function hide(message) {
        if (message) {
            base.message = message;
        }
        base.opacity = 0;
        base.currentButton = base.defaultButton; 
    }

    anchors.fill: parent;
    z: 99; //Just to make sure we're always on top.
    opacity: 0;
    Behavior on opacity { NumberAnimation { duration: 500; } }

    MouseArea {
        anchors.fill: parent;
        onClicked: {
            // Don't allow people to click away a progress bar...
            // horrible things could happen if they do so (inconsistent states and what not)
            if (progress !== -1)
                return;
            base.canceled();
            base.hide();
        }
    }
    SimpleTouchArea {
        anchors.fill: parent;
        onTouched: {
            // Don't allow people to click away a progress bar...
            // horrible things could happen if they do so (inconsistent states and what not)
            if (progress !== -1)
                return;
            base.canceled();
            base.hide();
        }
    }

    Rectangle {
        id: modalFill;

        anchors.fill: parent;
        color: "#80000000";

        Keys.enabled: base.visible && base.opacity === 1;
        Keys.onEscapePressed: {
                // Don't allow people to escape from a progress bar...
                // horrible things could happen if they do so (inconsistent states and what not)
                if (progress !== -1)
                    return;
                base.canceled();
                base.hide();
            }
        Keys.onEnterPressed: { base.buttonClicked(base.currentButton); base.hide(); }
        Keys.onReturnPressed: { base.buttonClicked(base.currentButton); base.hide(); }
        focus: Keys.enabled;
        Keys.onTabPressed: {
            base.currentButton += 1;
            if (base.currentButton >= base.buttons.length) {
                base.currentButton = 0;
            }
        }
    }

    Rectangle {
        id: dialogBackground;

        anchors.centerIn: parent;

        width: parent.width / 2;
        height: Constants.GridHeight * 2 + dialogText.height + 32;

        radius: 8;

        gradient: Gradient {
            GradientStop { position: 0; color: "#F7F8FC"; }
            GradientStop { position: 0.4; color: "#F0F0FA"; }
        }

        Rectangle {
            id: dialogHeader;

            anchors {
                top: parent.top;
                left: parent.left;
                right: parent.right;
            }

            height: Constants.GridHeight;
            color: "#9AA1B2";
            radius: 8;

            Rectangle {
                anchors {
                    left: parent.left;
                    right: parent.right;
                    bottom: parent.bottom;
                }
                height: 8;
                color: "#9AA1B2";
            }

            Label {
                id: dialogTitle;
                anchors.centerIn: parent;
                color: "white";
                font.pixelSize: Constants.LargeFontSize;
            }

            Image {
                anchors.left: parent.left;
                anchors.right: parent.right;
                anchors.top: parent.bottom;

                source: "../images/shadow-smooth.png";
            }
        }

        Label {
            id: dialogText;

            anchors {
                left: parent.left;
                leftMargin: 8;
                right: parent.right;
                rightMargin: 8;
            }

            y: dialogHeader.height + 16;
            elide: Text.ElideNone;
            wrapMode: Text.Wrap;
        }

        Rectangle {
            id: progressBase;
            opacity: (progress > 0 && progressBar.width > 0) ? 1 : 0;
            Behavior on opacity { NumberAnimation { duration: 500; } }
            anchors {
                top: dialogText.bottom;
                horizontalCenter: parent.horizontalCenter;
                margins: 8;
            }
            height: Constants.LargeFontSize + 4;
            width: 208;
            radius: height / 2;
            border {
                width: 1;
                color: "silver";
            }
            color: "white";
            Rectangle {
                id: progressBar;
                anchors {
                    top: parent.top;
                    left: parent.left;
                    margins: 4;
                }
                radius: height / 2;
                width: progress >= 0 ? (progress * 2) + 1: 100;
                height: parent.height - 7;
                Behavior on width { PropertyAnimation { duration: 100; } }
                color: "gray";
            }
        }
        BusyIndicator {
            id: busy;
            anchors.fill: progressBase;
            opacity: (progress > -1 && progressBase.opacity === 0) ? 1 : 0;
            running: opacity === 1;
            Behavior on opacity { PropertyAnimation { duration: 100; } }
        }

        Row {
            id: buttonRow;

            anchors {
                left: parent.left;
                leftMargin: 8;
                right: parent.right;
                bottom: parent.bottom;
                bottomMargin: 8;
            }

            height: Constants.GridHeight;
            spacing: 8;

            Repeater {
                model: base.buttons;

                delegate: Button {
                    width: (parent.width / base.buttons.length) - 8;
                    height: parent.height;
                    radius: 8;

                    text: modelData;
                    color: "#9AA1B2";
                    textColor: "white";

                    onClicked: { base.buttonClicked(index); base.hide(); }
                    hasFocus: index === base.currentButton;
                }
            }
        }
    }

    Image {
        anchors {
            left: dialogBackground.left;
            leftMargin: dialogBackground.radius;
            right: dialogBackground.right;
            rightMargin: dialogBackground.radius;
            top: dialogBackground.bottom;
        }

        source: "../images/shadow-smooth.png";
    }
}