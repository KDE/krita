/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __KO_ICON_UTILS_H
#define __KO_ICON_UTILS_H

#include "kritawidgetutils_export.h"

class QIcon;
class QString;

namespace KoIconUtils
{

enum StdSizes {
    SizeSmall = 16, SizeSmallMedium = 22, SizeMedium = 32, SizeLarge = 48,
    SizeHuge = 64, SizeEnormous = 128
};

enum Group {
    NoGroup = -1,
    Desktop = 0,
    FirstGroup = 0,
    Toolbar,
    MainToolbar,
    Small,
    Panel,
    Dialog,
    LastGroup,
    User
};

/**
 * Load a themed icon using its base name. Use it in
 * Krita instead of previous themedIcon()
 */
KRITAWIDGETUTILS_EXPORT QIcon themedIcon(const QString &name);
}

#endif /* __KIS_ICON_UTILS_H */
