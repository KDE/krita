/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_ICON_UTILS_H
#define __KIS_ICON_UTILS_H

#include "kritawidgetutils_export.h"

class QAbstractButton;
class QComboBox;
class QAction;
class QObject;

#include <QIcon>
#include <QString>

namespace KisIconUtils
{

    enum StdSizes {
        SizeSmall = 16, SizeSmallMedium = 22, SizeMedium = 32, SizeLarge = 48,
        SizeHuge = 64, SizeEnormous = 128
    };

    enum class Group {
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
     * Krita instead of previous KisIconUtils::loadIcon()
     */
    KRITAWIDGETUTILS_EXPORT QIcon loadIcon(const QString &name);

    /**
     * Should we use a dark or light themeed icon? Useful for images that are loaded
     * dynamically like document templates instead of being in static resource files
     */
    KRITAWIDGETUTILS_EXPORT bool useDarkIcons();

    /**
     * This function updates an icon of \p object depending on its
     * type. See updateIcon() overrides to see the supported types
     */
    KRITAWIDGETUTILS_EXPORT void updateIconCommon(QObject *object);

    /**
     * Update an icon of \p button according to the current theme
     */
    KRITAWIDGETUTILS_EXPORT void updateIcon(QAbstractButton *button);


    KRITAWIDGETUTILS_EXPORT void clearIconCache();

    /**
     * Update an icon of \p comboBox according to the current theme
     */
    KRITAWIDGETUTILS_EXPORT void updateIcon(QComboBox *comboBox);

    /**
     * Update an icon of \p action according to the current theme
     */
    KRITAWIDGETUTILS_EXPORT void updateIcon(QAction *action);
}

#endif /* __KIS_ICON_UTILS_H */
