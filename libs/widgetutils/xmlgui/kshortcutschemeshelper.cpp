/* This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2008 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
