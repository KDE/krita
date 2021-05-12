/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.3
import org.krita.sketch 1.0

Text {
    verticalAlignment: Text.AlignVCenter;
    color: Settings.theme.color("components/label");
    // Do not use font from the theme, as it can cause font size issues, like the font is not properly adapted on resize,
    // or like if krita is started with touchdocker floating, open/save dialogs will have tiny small fonts...
    // So, using the values from Constants directly in font.pixelSize ensures to have them always dynamically adapted
    //font: Settings.theme.font("application");
    font.family: "Source Sans Pro";
    font.styleName: "Regular";
    font.pixelSize: Constants.DefaultFontSize;
    elide: Text.ElideRight;
}
