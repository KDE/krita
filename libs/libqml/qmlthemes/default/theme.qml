/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2014 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.3
import org.krita.sketch 1.0
import "colors.js" as Colors
import "sizes.js" as Sizes
import "fonts.js" as Fonts

Theme {
    id: "default"
    name: "Default"

    colors: Colors.values;
    sizes: Sizes.values;
    fonts: Fonts.values;

    iconPath: "icons/"
    imagePath: "images/"
    fontPath: "fonts/"
}
