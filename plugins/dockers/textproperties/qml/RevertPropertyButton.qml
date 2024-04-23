/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Controls 2.0

Item {
    width: firstColumnWidth;
    height: firstColumnWidth;

    property alias revertEnabled: revert.enabled;
    property bool inheritable: true;

    ToolButton {
        id: revert;
        icon.width: 22;
        icon.height: 22;
        icon.color: sysPalette.text;
        opacity: enabled? 1.0: 0.5;
        display: AbstractButton.IconOnly
        icon.source: parent.inheritable? "qrc:///light_edit-undo.svg": "qrc:///22_light_trash-empty.svg";
    }
}
