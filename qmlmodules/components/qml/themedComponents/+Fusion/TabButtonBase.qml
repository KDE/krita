/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Controls 2.15

/*
 By default, Fusion's tabbuttons don't look anything like their QWidgets
 variant. This tries to fix that.
*/
TabButton {
    id: control;
    bottomInset: -10;
    clip: true;
    y: checked ? 0 : 2;

    height: implicitHeight + (checked ? 2 : 0);

    background:Rectangle {
        readonly property bool checked: control.checked;

        color: checked? control.palette.window: Qt.darker(parent.palette.window, 1.1);
        border.color: Qt.darker(parent.palette.window, 1.4);
        radius: 1;
        Rectangle {
            x: 1; y: 1
            width: parent.width - 2
            height: parent.height - 2
            border.color: Qt.lighter(parent.color, 1.3);
            color: parent.color;
            radius: 2;
        }
        Rectangle {
            x: 2; y: 2
            width: parent.width - 4
            height: parent.height;
            color: parent.color;
            radius: 3;
        }
    }
}
