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

import QtQuick 2.3
import org.krita.sketch 1.0

Item {
    id: base;
    width: parent ? parent.width : 0;
    height: Constants.GridHeight;

    property alias text: input.text;
    property alias placeholder: placeholder.text;
    property alias validator: input.validator;
    property alias inputMask: input.inputMask;
    property alias acceptableInput: input.acceptableInput;

    property alias background: fill.color;
    property alias border: fill.border;
    property alias radius: fill.radius;

    property alias numeric: input.numeric;
    property alias nextFocus: input.nextFocus;

    property alias textInput: input; // To allow KeyNavigation to reference TextInput

    signal focusLost();
    signal accepted();

    Item {
        anchors.fill: parent;
        anchors.margins: Constants.DefaultMargin;

        Rectangle {
            id: fill;
            anchors.fill: parent;

            border.width: 2;
            border.color: Settings.theme.color("components/textField/border/normal");
            color: Settings.theme.color("components/textField/background");

            radius: height / 2;

            Label {
                id: placeholder;
                anchors {
                    left: parent.left;
                    leftMargin: base.height / 4;
                    rightMargin: base.height / 4;
                    verticalCenter: parent.verticalCenter;
                }
                color: Settings.theme.color("components/textField/placeholder");
            }

            TextInput {
                id: input;

                property bool numeric: false;
                DoubleValidator { id: numberValidator }
                validator: numeric ? numberValidator : null;
                property Item nextFocus: null;

                anchors {
                    left: parent.left;
                    leftMargin: base.height / 4;
                    right: parent.right;
                    rightMargin: base.height / 4;
                    verticalCenter: parent.verticalCenter;
                }
                font: Settings.theme.font("application");
                onFocusChanged: {
                    if (focus === false) {
                        Qt.inputMethod.hide()
                        Settings.focusItem = null;
                        base.focusLost();
                    } else {
                        Settings.focusItem = input;
                    }
                }
                onAccepted: base.accepted();
            }

            states: [
                State {
                    name: "error";
                    when: !input.acceptableInput
                    PropertyChanges { target: fill; border.color: Settings.theme.color("components/textField/border/error"); }
                },
                State {
                    name: "focus";
                    when: input.focus;
                    PropertyChanges { target: fill; border.color: Settings.theme.color("components/textField/border/focus"); }
                }
            ]
        }
    }

    MouseArea {
        anchors.fill: parent;
        onClicked: input.focus = true;
    }

    states: State {
        name: "input";
        when: input.focus || input.text != "";

        PropertyChanges { target: placeholder; opacity: 0.75; }
        AnchorChanges { target: placeholder; anchors.left: undefined; anchors.right: parent.right }
    }

    transitions: Transition {
        ParallelAnimation {
            NumberAnimation { duration: 100; properties: "opacity"; }
            AnchorAnimation { duration: 100; }
        }
    }
}
