/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Controls 2.15

/**
  Dummy Control to work around QQuickPalette and QPalette being two completely different things.
  */
Control {
    id: paletteControl;

    property alias colorGroup: activePalette.colorGroup

    SystemPalette {
        id: activePalette;
        colorGroup: SystemPalette.Active
    }


    palette.alternateBase: activePalette.alternateBase;
    palette.base: activePalette.base;
    palette.button: activePalette.button;
    palette.buttonText: activePalette.buttonText;
    palette.dark: activePalette.dark;
    palette.highlight: activePalette.highlight;
    palette.highlightedText: activePalette.highlightedText;
    palette.light: activePalette.light;

    palette.link: activePalette.link;
    palette.linkVisited: activePalette.linkVisited;
    palette.mid: activePalette.mid;
    palette.midlight: activePalette.midlight;
    palette.shadow: activePalette.shadow;
    palette.text: activePalette.text;
    palette.window: activePalette.window;
    palette.windowText: activePalette.windowText;

    palette.toolTipBase: activePalette.alternateBase;
    palette.toolTipText: activePalette.text;
    palette.brightText: Qt.lighter(activePalette.text);

}
