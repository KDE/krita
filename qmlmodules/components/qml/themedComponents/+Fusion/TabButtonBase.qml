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
    clip: true;

    padding: 2;
    horizontalPadding: 4
    spacing: 6
    icon.width: 16
    icon.height: 16

    z: checked;

    background:Rectangle {
        readonly property bool checked: control.checked;

        color: checked? control.palette.window: Qt.darker(parent.palette.window, 1.1);
        border.color: Qt.darker(parent.palette.window, 1.4);
        radius: 2;
        y: checked ? 0 : 2;
        height: control.height+5;
        Rectangle {
            x: 1; y: 1
            width: parent.width - 2
            height: parent.height - 2
            border.color: control.palette.light.hslLightness > 0.63? control.palette.window: control.palette.light;
            color: parent.color;
            radius: parent.radius+1;
        }
    }
}
