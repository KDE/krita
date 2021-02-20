/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.3
import org.krita.sketch 1.0

Item {
    id: base;
    enabled: (visible && opacity > 0.0);

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
    Behavior on opacity { NumberAnimation { duration: Constants.AnimationDuration; } }

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
        color: Settings.theme.color("components/dialog/modalOverlay");

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
            GradientStop { position: 0; color: Settings.theme.color("components/dialog/background/start"); }
            GradientStop { position: 0.4; color: Settings.theme.color("components/dialog/background/stop"); }
        }

        Rectangle {
            id: dialogHeader;

            anchors {
                top: parent.top;
                left: parent.left;
                right: parent.right;
            }

            height: Constants.GridHeight;
            color: Settings.theme.color("components/dialog/header");
            radius: 8;

            Rectangle {
                anchors {
                    left: parent.left;
                    right: parent.right;
                    bottom: parent.bottom;
                }
                height: 8;
                color: Settings.theme.color("components/dialog/header");
            }

            Label {
                id: dialogTitle;
                anchors.centerIn: parent;
                color: Settings.theme.color("components/dialog/headerText");
                font: Settings.theme.font("title");
            }

            Shadow { anchors { left: parent.left; right: parent.right; top: parent.bottom; } }
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
            Behavior on opacity { NumberAnimation { duration: Constants.AnimationDuration; } }
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
                color: Settings.theme.color("components/dialog/progress/border");
            }
            color: Settings.theme.color("components/dialog/progress/background");
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
                color: Settings.theme.color("components/dialog/progress/bar");
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
                    color: Settings.theme.color("components/dialog/button");
                    textColor: Settings.theme.color("components/dialog/buttonText");

                    onClicked: { base.buttonClicked(index); base.hide(); }
                    hasFocus: index === base.currentButton;
                }
            }
        }
    }

    Shadow {
        anchors {
            left: dialogBackground.left;
            leftMargin: dialogBackground.radius;
            right: dialogBackground.right;
            rightMargin: dialogBackground.radius;
            top: dialogBackground.bottom;
        }
    }
}
