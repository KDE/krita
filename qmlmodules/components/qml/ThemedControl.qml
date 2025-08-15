/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Controls 2.15
import org.krita.components 1.0 as Kis

/**
  Dummy Control to work around QQuickPalette and QPalette being two completely different things.
  This could probably be replaced with Kirigami.Theme if we start using Kirigami.

  You will need to manually setup this control and its color group for each control that can be disabled,
  to ensure that the correct palette is retrieved. Qt6 has something more complex for this, where the
  active/inactive(unfocused)/disabled palettes can all be set, but in Qt5 this is the best we've got.
  */
Control {
    id: themedControl;

    // There doesn't seem to be any relevant difference between inactive and active, so we don't bother distinguishing between them.
    property int colorGroup: enabled? SystemPalette.Active: SystemPalette.Disabled;

    property Kis.Theme theme: Kis.Theme {
        window.state: colorGroup;
        view.state: colorGroup;
        button.state: colorGroup;
        selection.state: colorGroup;
        tooltip.state: colorGroup;
    }

    palette.alternateBase: theme.view.alternateBackgroundColor;
    palette.base: theme.view.backgroundColor;
    palette.button: theme.button.backgroundColor;
    palette.buttonText: theme.button.textColor;
    palette.dark: theme.window.darkShadeColor;
    palette.highlight: theme.selection.backgroundColor;
    palette.highlightedText: theme.selection.textColor;
    palette.light: theme.window.lightShadeColor;


    palette.mid: theme.window.midShadeColor;
    palette.midlight: theme.window.midlightShadeColor;
    palette.shadow: theme.window.shadowShadeColor;
    palette.text: theme.view.textColor;
    palette.window: theme.window.backgroundColor;
    palette.windowText: theme.window.textColor;

    palette.link: theme.view.linkColor;
    palette.linkVisited: theme.view.linkVisitedColor;
    palette.toolTipBase: theme.tooltip.backgroundColor;
    palette.toolTipText: theme.tooltip.textColor;
    palette.brightText: Qt.lighter(theme.view.textColor);

}
