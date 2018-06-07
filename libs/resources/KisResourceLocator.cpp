/*
 * Copyright (C) 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KisResourceLocator.h"

#include <QList>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <klocalizedstring.h>

#include "KisResourceStorage.h"

const QString KisResourceLocator::resourceLocationKey {"ResourceDirectory"};
const QStringList KisResourceLocator::resourceTypeFolders = QStringList()
        << "tags"
        << "asl"
        << "bundles"
        << "brushes"
        << "gradients"
        << "paintoppresets"
        << "palettes"
        << "patterns"
        << "taskset"
        << "workspaces"
        << "input"
        << "pykrita"
        << "symbols"
        << "color-schemes"
        << "preset_icons"
        << "preset_icons/tool_icons"
        << "preset_icons/emblem_icons";

class KisResourceLocator::Private {
public:
    QString resourceLocation;
    QList<KisResourceStorageSP> storages;
    QStringList errorMessages;
};

KisResourceLocator::KisResourceLocator()
    : d(new Private())
{
}

KisResourceLocator::~KisResourceLocator()
{
}

KisResourceLocator::LocatorError KisResourceLocator::initialize()
{
    KConfigGroup cfg(KSharedConfig::openConfig(), "");
    d->resourceLocation = cfg.readEntry(resourceLocationKey, QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

    QFileInfo fi(d->resourceLocation);

    if (!fi.exists()) {
        if (!QDir().mkpath(d->resourceLocation)) {
            d->errorMessages << i18n("Could not create the resource location at %1.", d->resourceLocation);
            return LocatorError::CannotCreateLocation;
        }
    }

    if (!fi.isWritable()) {
        d->errorMessages << i18n("The resource location at %1 is not writable.", d->resourceLocation);
        return LocatorError::LocationReadOnly;
    }

    if (QDir(d->resourceLocation).isEmpty()) {
        KisResourceLocator::LocatorError res = firstTimeInstallation();
        if (res != LocatorError::Ok) {
            return res;
        }
    }



    return LocatorError::Ok;
}

KisResourceLocator::LocatorError KisResourceLocator::firstTimeInstallation()
{
    Q_FOREACH(const QString &folder, resourceTypeFolders) {
        QFileInfo fi(d->resourceLocation + "/" + folder + "/");
        QDir dir;
        if (!fi.exists()) {
            if (!dir.mkpath(fi.canonicalFilePath())) {
                d->errorMessages << i18n("Could not create the resource location at %1.", fi.canonicalPath());
                return LocatorError::CannotCreateLocation;
            }
        }
        if (!fi.isWritable()) {
            d->errorMessages << i18n("The resource location at %1 is not writable.", fi.canonicalPath());
            return LocatorError::LocationReadOnly;
        }
    }

    return LocatorError::Ok;
}
