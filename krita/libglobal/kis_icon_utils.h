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

#ifndef __KIS_ICON_UTILS_H
#define __KIS_ICON_UTILS_H

#include "kritaglobal_export.h"

class QAbstractButton;
class QComboBox;
class QAction;
class QObject;

#include <QIcon>
#include <KoIcon.h>

namespace KisIconUtils
{
    /**
     * Load a themed icon using its base name. Use it in
     * Krita instead of previous KisIconUtils::loadIcon()
     */
    KRITAGLOBAL_EXPORT QIcon loadIcon(const QString &name);

    /**
     * This function updates an icon of \p object depending on its
     * type. See updateIcon() overrides to see the supported types
     */
    KRITAGLOBAL_EXPORT void updateIconCommon(QObject *object);

    /**
     * Update an icon of \p button according to the current theme
     */
    KRITAGLOBAL_EXPORT void updateIcon(QAbstractButton *button);

    /**
     * Update an icon of \p comboBox according to the current theme
     */
    KRITAGLOBAL_EXPORT void updateIcon(QComboBox *comboBox);

    /**
     * Update an icon of \p action according to the current theme
     */
    KRITAGLOBAL_EXPORT void updateIcon(QAction *action);
}

#endif /* __KIS_ICON_UTILS_H */
