/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick 2.3
import org.krita.sketch 1.0
import org.krita.sketch.components 1.0

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
            id: mainPageStack;

            width: base.width;
            height: base.height;

            onCurrentPageChanged: window.currentSketchPage = (currentPage.pageName !== undefined) ? currentPage.pageName : currentPage.toString();
            initialPage: openImagePage;

            transitionDuration: Constants.AnimationDuration;

            Component { id: openImagePage; OpenImagePage { } }

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

    VirtualKeyboard {
        id: keyboard;
        onKeyboardVisibleChanged: if (keyboardVisible) screenScroller.ensureVisible(Settings.focusItem);
    }

    Connections {
        target: Settings;

        onFocusItemChanged: if (keyboard.keyboardVisible) screenScroller.ensureVisible(Settings.focusItem);
    }
}
