/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15

Column {
    property int firstColumnWidth: 32;
    property int columnSpacing: 5;

    Rectangle {
        color: sysPalette.base;
        height: 1;
        width: parent.width;
    }
}
