/*
 *  SPDX-FileCopyrightText: 2024 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Controls 2.15

ToolTip {
    id: root

    delay: Qt.styleHints.mousePressAndHoldInterval;
    timeout: Qt.styleHints.mousePressAndHoldInterval;

    property Item parentControl
    property bool changingParentControl: false

    onParentChanged: {
        if (changingParentControl) {
            return;
        }
        if (parentControl) {
            parentControl = undefined;
            root.visible = false;
        }
    }

    onParentControlChanged: {
        changingParentControl = true;
        if (root.parentControl) {
            root.parent = root.parentControl;
            root.parentControl.hoverEnabled = true;
            root.visible = Qt.binding(() => { return root.parentControl.hovered });
        } else {
            root.parent = undefined;
            root.visible = false;
        }
        changingParentControl = false;
    }
}
