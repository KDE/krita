/* This file is part of the KDE project
 * Copyright (C) 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 * Copyright (C) 2013 Dan Leinir Turthra Jensen <admin@leinir.dk>
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
    height: Constants.GridHeight * 4;

    property alias text: input.text;
    property alias placeholder: placeholder.text;

    property alias background: fill.color;
    property alias border: fill.border;
    property alias radius: fill.radius;

    property alias nextFocus: input.nextFocus;

    signal focusLost();
    signal accepted();

    Item {
        anchors.fill: parent;
        anchors.margins: Constants.DefaultMargin;

        Rectangle {
            id: fill;
            anchors.fill: parent;

            border.width: 2;
            border.color: Settings.theme.color("components/textFieldMultiline/border");
            color: Settings.theme.color("components/textFieldMultiline/background");

            radius: Constants.GridHeight / 2;

            Label {
                id: placeholder;
                anchors {
                    left: parent.left;
                    leftMargin: fill.radius / 2;
                    rightMargin: fill.radius / 2;
                    top: parent.top;
                    topMargin: fill.radius / 2;
                }
                color: Constants.Theme.SecondaryTextColor;
            }

            Item {
                anchors.fill: parent;
                anchors.margins: parent.radius / 2;
                Flickable {
                    id: flick;
                    anchors {
                        fill: parent;
                    }
                    clip: true;
                    contentWidth: input.paintedWidth
                    contentHeight: input.paintedHeight
                    function ensureVisible(r)
                    {
                        if (contentX >= r.x)
                            contentX = r.x;
                        else if (contentX+width <= r.x+r.width)
                            contentX = r.x+r.width-width;
                        if (contentY >= r.y)
                            contentY = r.y;
                        else if (contentY+height <= r.y+r.height)
                            contentY = r.y+r.height-height;
                    }
                    TextEdit {
                        id: input;
                        width: flick.width;
                        height: flick.height;
                        wrapMode: TextEdit.WrapAtWordBoundaryOrAnywhere;
                        property Item nextFocus: null;
                        onCursorRectangleChanged: flick.ensureVisible(cursorRectangle)

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
                    }
                }
                ScrollDecorator {
                    flickableItem: flick;
                }
            }
        }
    }

    MouseArea {
        anchors.fill: parent;
        onClicked: input.focus = true;
    }

    states: State {
        name: "input";
        when: input.focus || input.text != "";

        PropertyChanges { target: placeholder; opacity: 0.5; }
        AnchorChanges { target: placeholder; anchors.left: undefined; anchors.right: parent.right }
    }

    transitions: Transition {
        ParallelAnimation {
            NumberAnimation { duration: 100; properties: "opacity"; }
            AnchorAnimation { duration: 100; }
        }
    }
}
