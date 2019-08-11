/*  This file is part of the KDE libraries
 *  Copyright 2012 David Faure <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License or ( at
 *  your option ) version 3 or, at the discretion of KDE e.V. ( which shall
 *  act as a proxy as in section 14 of the GPLv3 ), any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#ifndef KHELPCLIENT_H
#define KHELPCLIENT_H

#include "kritawidgetutils_export.h"
#include <QString>
#include <QByteArray>

namespace KHelpClient
{
/**
 * Invokes the KHelpCenter HTML help viewer from docbook sources.
 *
 * The HTML file will be found using the X-DocPath entry in the application's desktop file.
 * It can be either a relative path, or a website URL.
 *
 * @param anchor      This has to be a defined anchor in your
 *                    docbook sources or website. If empty the main index
 *                    is loaded.
 * @param appname     This allows you to specify the .desktop file to get the help path from.
 *                    If empty the QCoreApplication::applicationName() is used.
 * @since 5.0
 */
KRITAWIDGETUTILS_EXPORT void invokeHelp(const QString &anchor = QString(), const QString &appname = QString());
}

#endif /* KHELPCLIENT_H */

