/* This file is part of the KDE libraries
    Copyright (C) 2008 Alexander Dymo <adymo@kdevelop.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#include "kshortcutschemeshelper_p.h"

#include <QAction>
#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QDomDocument>
#include <QStandardPaths>

#include <QDir>
#include <kconfiggroup.h>
#include <ksharedconfig.h>

#include "kactioncollection.h"
#include "kxmlguiclient.h"

#include "KoResourcePaths.h"
#include "kis_action_registry.h"


QString KShortcutSchemesHelper::shortcutSchemeFileName(const QString &schemeName)
{
    // Create a directory if one cannot be found.
    return KoResourcePaths::locateLocal("kis_shortcuts", schemeName, true);
}


QHash<QString, QString> KShortcutSchemesHelper::schemeFileLocations()
{
    QStringList schemes;
    schemes << QString("Default");  // Forbid "Default.shortcuts"
    QHash<QString, QString> schemeFileLocations;
    const QStringList shortcutFiles = KoResourcePaths::findAllResources("kis_shortcuts", "*.shortcuts");
    Q_FOREACH (const QString &file, shortcutFiles) {
        QFileInfo fileInfo(file);
        QString schemeName = fileInfo.completeBaseName();
        if (!schemes.contains(schemeName)) {
            schemes << schemeName;
            schemeFileLocations.insert(schemeName, fileInfo.canonicalFilePath());
        }
    }
    return schemeFileLocations;
}
