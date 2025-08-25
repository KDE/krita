/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Window 2.15

/**
  This object tests whether the current active item is a child of the
  parent of this item. This can be used to figure out if a collection
  of controls is being interacted with by the user.

  This is the qt5 version.
  */
Item {
    property bool inFocus: false;

    Window.onActiveFocusItemChanged: {
        let root = (parent && typeof parent !== 'undefined')? parent: this;
        /// Quick test to check this docker is in focus.
        let currentFocusItem = Window.activeFocusItem;
        let anyFocus = (typeof currentFocusItem !== "undefined" || currentFocusItem);
        if (anyFocus) {
            anyFocus = false;
            let testFocus = currentFocusItem;
            while (typeof testFocus !== "undefined" && testFocus) {
                if (testFocus === root) {
                    anyFocus = true;
                    break;
                }
                testFocus = testFocus.parent;
            }
        }
        inFocus = anyFocus;
    }
}
