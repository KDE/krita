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

#include <QDebug>
#include <QList>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QMessageBox>
#include <QVersionNumber>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <klocalizedstring.h>

#include <KritaVersionWrapper.h>
#include "KoResourcePaths.h"
#include "KisResourceStorage.h"
#include "KisResourceCacheDb.h"

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
        << "symbols";

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

KisResourceLocator::LocatorError KisResourceLocator::initialize(const QString &installationResourcesLocation)
{
    InitalizationStatus initalizationStatus = InitalizationStatus::Unknown;

    KConfigGroup cfg(KSharedConfig::openConfig(), "");
    d->resourceLocation = cfg.readEntry(resourceLocationKey, QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

    QFileInfo fi(d->resourceLocation);

    if (!fi.exists()) {
        if (!QDir().mkpath(d->resourceLocation)) {
            d->errorMessages << i18n("1. Could not create the resource location at %1.", d->resourceLocation);
            return LocatorError::CannotCreateLocation;
        }
        initalizationStatus = InitalizationStatus::FirstRun;
    }

    if (!fi.isWritable()) {
        d->errorMessages << i18n("2. The resource location at %1 is not writable.", d->resourceLocation);
        return LocatorError::LocationReadOnly;
    }

    // Check whether we're updating from an older version
    if (initalizationStatus != InitalizationStatus::FirstRun) {
        QFile fi(d->resourceLocation + '/' + "KRITA_RESOURCE_VERSION");
        if (!fi.exists()) {
            initalizationStatus = InitalizationStatus::FirstUpdate;
        }
        else {
            fi.open(QFile::ReadOnly);
            QVersionNumber resource_version = QVersionNumber::fromString(QString::fromUtf8(fi.readAll()));
            QVersionNumber krita_version = QVersionNumber::fromString(KritaVersionWrapper::versionString());
            if (krita_version > resource_version) {
                initalizationStatus = InitalizationStatus::Updating;
            }
            else {
                initalizationStatus = InitalizationStatus::Initialized;
            }
        }
    }

    if (initalizationStatus != InitalizationStatus::Initialized) {
        KisResourceLocator::LocatorError res = firstTimeInstallation(initalizationStatus, installationResourcesLocation);
        if (res != LocatorError::Ok) {
            return res;
        }
        initalizationStatus = InitalizationStatus::Initialized;
    }

    // Add the folder
    d->storages.append(QSharedPointer<KisResourceStorage>::create(d->resourceLocation));

    // And add bundles and adobe libraries
    QStringList filters = QStringList() << "*.bundle" << "*.abr" << "*.asl";
    QDirIterator iter(d->resourceLocation, filters, QDir::Files, QDirIterator::Subdirectories);
    while (iter.hasNext()) {
        iter.next();
        d->storages.append(QSharedPointer<KisResourceStorage>::create(iter.filePath()));
    }

    return LocatorError::Ok;
}

QStringList KisResourceLocator::errorMessages() const
{
    return d->errorMessages;
}

KisResourceLocator::LocatorError KisResourceLocator::firstTimeInstallation(InitalizationStatus initalizationStatus, const QString &installationResourcesLocation)
{
    Q_UNUSED(initalizationStatus);

    Q_FOREACH(const QString &folder, resourceTypeFolders) {
        QDir dir(d->resourceLocation + '/' + folder + '/');
        if (!dir.exists()) {
            if (!QDir().mkpath(d->resourceLocation + '/' + folder + '/')) {
                d->errorMessages << i18n("3. Could not create the resource location at %1.", dir.path());
                return LocatorError::CannotCreateLocation;
            }
        }
    }

    Q_FOREACH(const QString &folder, resourceTypeFolders) {
        QDir dir(installationResourcesLocation + '/' + folder + '/');
        if (dir.exists()) {
            Q_FOREACH(const QString &entry, dir.entryList(QDir::Files | QDir::Readable)) {
                QFile f(dir.canonicalPath() + '/'+ entry);
                bool r = f.copy(d->resourceLocation + '/' + folder + '/' + entry);
                if (!r) {
                    d->errorMessages << "Could not copy resource" << f.fileName() << "to the resource folder";
                }
            }
        }
    }

    QFile f(d->resourceLocation + '/' + "KRITA_RESOURCE_VERSION");
    f.open(QFile::WriteOnly);
    f.write(KritaVersionWrapper::versionString().toUtf8());
    f.close();

    return LocatorError::Ok;
}

bool KisResourceLocator::synchronizeDb()
{
    Q_FOREACH(const KisResourceStorageSP storage, d->storages) {
        qDebug() << "Storage" << storage->location() << storage->valid();
    }
    return true;
}
