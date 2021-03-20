/*  This file is part of the KDE libraries
 *  SPDX-FileCopyrightText: 2012 David Faure <faure@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
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

