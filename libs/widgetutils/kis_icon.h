/*  This file is part of the Calligra project, made within the KDE community.

    SPDX-FileCopyrightText: 2012 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOICON_H
#define KOICON_H

#include <kis_icon_utils.h>
#include <QString>
/**
 * Macros to support collecting the icons in use.
 *
 * After any change to this list of macros the file /CheckIcons.sh needs to be
 * updated accordingly, to ensure that the icon names of the affected macros are
 * still considered in the extraction.
 *
 * The naming pattern of the macros is like this:
 * * koIcon*, kisIcon return a QIcon object
 * * koIconName* returns a QLatin1String (aligned with usual API where "iconName" property is of type QString)
 * * koIconNameCStr* returns a const char*
 */

/// Use these macros for icons without any issues
#define koIcon(name) (KisIconUtils::loadIcon(QLatin1String(name)))
#define kisIcon(name) (KisIconUtils::loadIcon(name))
#define koIconName(name) (QLatin1String(name))
#define koIconNameCStr(name) (name)

/// Use these macros if there is a proper icon missing
#define koIconNeeded(comment, neededName) (KisIconUtils::loadIcon(QLatin1String(neededName)))
#define koIconNeededWithSubs(comment, neededName, substituteName) (KisIconUtils::loadIcon(QLatin1String(substituteName)))
#define koIconNameNeeded(comment, neededName) (QLatin1String(neededName))
#define koIconNameNeededWithSubs(comment, neededName, substituteName) (QLatin1String(substituteName))
#define koIconNameCStrNeeded(comment, neededName) (neededName)
#define koIconNameCStrNeededWithSubs(comment, neededName, substituteName) (substituteName)

#endif
