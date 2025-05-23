/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Controls 2.15

/**
  Dummy Control to work around QQuickPalette and QPalette being two completely different things.
  This could probably be replaced with Kirigami.Theme if we start using Kirigami.

  You will need to manually setup this control and its color group for each control that can be disabled,
  to ensure that the correct palette is retrieved. Qt6 has something more complex for this, where the
  active/inactive(unfocused)/disabled palettes can all be set, but in Qt5 this is the best we've got.
  */
Control {
    id: paletteControl;

    property alias colorGroup: activePalette.colorGroup;
    colorGroup: parent && typeof parent != 'undefined'?
                    parent.enabled?
                        parent.activeFocus? SystemPalette.Active: SystemPalette.Inactive:
                                            SystemPalette.Disabled:
                                                SystemPalette.Active;

    SystemPalette {
        id: activePalette;
        colorGroup: SystemPalette.Active;

        onButtonTextChanged: paletteControl.palette.buttonText = buttonText;
    }

    palette.alternateBase: activePalette.alternateBase;
    palette.base: activePalette.base;
    palette.button: activePalette.button;
    palette.buttonText: activePalette.buttonText;
    palette.dark: activePalette.dark;
    palette.highlight: activePalette.highlight;
    palette.highlightedText: activePalette.highlightedText;
    palette.light: activePalette.light;

    //Link and LinkVisited are both disabled because they cannot be retrieved from the system palette.
    //palette.link: activePalette.link;
    //palette.linkVisited: activePalette.linkVisited;
    palette.mid: activePalette.mid;
    palette.midlight: activePalette.midlight;
    palette.shadow: activePalette.shadow;
    palette.text: activePalette.text;
    palette.window: activePalette.window;
    palette.windowText: activePalette.windowText;

    // None of the following are available from SystemPalette either...
    palette.toolTipBase: activePalette.alternateBase;
    palette.toolTipText: activePalette.text;
    palette.brightText: Qt.lighter(activePalette.text);

    Component.onCompleted: {
        // For some bizare reason, system palette doesn't change properly for button text on load.
        // Therefore, we manually need to setup the connection inside this control.
        palette.buttonText = activePalette.buttonText;
    }

}
