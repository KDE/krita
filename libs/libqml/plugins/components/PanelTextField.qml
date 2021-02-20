/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Dan Leinir Turthra Jensen <admin@leinir.dk>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.3
import org.krita.sketch 1.0

TextField {
    property bool enabled: true;
    height: Constants.DefaultFontSize + Constants.DefaultMargin * 4;

    border.color: Settings.theme.color("components/panelTextField/border");
    background: Settings.theme.color("components/panelTextField/background");

    Rectangle {
        id: enabledVisualiser;
        anchors.fill: parent;
        color: enabled ?
            Settings.theme.color("components/panelTextField/enabled") :
            Settings.theme.color("components/panelTextField/disabled");
    }
}
