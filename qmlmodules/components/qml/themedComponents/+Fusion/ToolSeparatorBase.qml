/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Controls 2.15

/*
  Fusion's default ToolSeparator has a bug when using the horizontal version,
  the lightened line is not handled properly, so here we fix that.
  */
ToolSeparator {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    padding: vertical ? 6 : 2
    verticalPadding: vertical ? 2 : 6

    contentItem: Rectangle {
        implicitWidth: control.vertical ? 2 : 8
        implicitHeight: control.vertical ? 8 : 2
        color: Qt.darker(control.palette.window, 1.1)

        Rectangle {
            x: control.vertical? 1: 0;
            y: control.vertical? 0: 1;
            width: control.vertical ? 1: parent.width;
            height: control.vertical ? parent.height: 1;
            color: Qt.lighter(control.palette.window, 1.1)
        }
    }

}
