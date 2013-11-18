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
import "components"

Item {
    id: base;
    property QtObject window: null;
    onWindowChanged: Krita.Window = window;
    Flickable {
        id: screenScroller;
        boundsBehavior: Flickable.StopAtBounds;

        anchors {
            top: parent.top;
            left: parent.left;
            right: parent.right;
            bottom: keyboard.top;
        }

        contentWidth: base.width;
        contentHeight: base.height;

        PageStack {
            width: base.width;
            height: base.height;

            onCurrentPageChanged: window.currentSketchPage = (currentPage.pageName !== undefined) ? currentPage.pageName : currentPage.toString();
            initialPage: welcomePage;

            transitionDuration: 500;

            Component { id: welcomePage; WelcomePage { } }

            MouseArea {
                anchors.fill: parent;
                onClicked: parent.focus = true;
            }
        }

        function ensureVisible(item) {
            if (item !== undefined && item !== null) {
                var targetPosition = item.mapToItem(screenScroller, item.x, item.y);
                if (targetPosition.y > base.height * 0.5) {
                    screenScroller.contentY = targetPosition.y - base.height / 2;
                    screenScroller.returnToBounds();
                }
            }
        }
    }

    // This component is used to get around the fact that MainPage takes a very long time to initialise in some cases
    Dialog {
        id: baseLoadingDialog;
        title: "Loading";
        message: "Please wait...";
        textAlign: Text.AlignHCenter;
        modalBackgroundColor: "#ffffff";
        opacity: 1;
        progress: 0;
        visible: false;
        function hideMe() { timer.start(); }
        Timer {
            id: timer;
            interval: 500; running: false; repeat: false;
            onTriggered: { parent.visible = false; baseLoadingDialog.progress = -1; }
        }
    }

    VirtualKeyboard {
        id: keyboard;
        onKeyboardVisibleChanged: if (keyboardVisible) screenScroller.ensureVisible(Settings.focusItem);
    }

    Connections {
        target: Settings;

        onFocusItemChanged: if (keyboard.keyboardVisible) screenScroller.ensureVisible(Settings.focusItem);
    }
}
