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

#include "KisResourceStorage.h"

#include <QFileInfo>
#include <QDebug>

#include <quazip.h>

#include "KisStoragePlugin.h"
#include "KisFolderStorage.h"
#include "KisBundleStorage.h"
#include "KisAbrStorage.h"
#include "KisAslStorage.h"

class KisResourceStorage::Private {
public:
    QString name;
    QString location;
    bool valid {false};
    KisResourceStorage::StorageType storageType {KisResourceStorage::StorageType::Unknown};
    QScopedPointer<KisStoragePlugin> storagePlugin;
};


KisResourceStorage::KisResourceStorage(const QString &location)
    : d(new Private())
{
    d->location = location;
    QFileInfo fi(d->location);
    if (fi.isDir()) {
        d->storagePlugin.reset(new KisFolderStorage(location));
        d->name = location;
        d->storageType = StorageType::Folder;
        d->valid = fi.isWritable();
    }
    else {
        if (d->location.endsWith(".bundle")) {
            d->storagePlugin.reset(new KisBundleStorage(location));
            d->storageType = StorageType::Bundle;
            // XXX: should we also check whether there's a valid metadata entry? Or is this enough?
            d->valid = (fi.isReadable() && QuaZip(d->location).open(QuaZip::mdUnzip));
        }
        else if (d->location.endsWith(".abr")) {
            d->storagePlugin.reset(new KisAbrStorage(location));
            d->storageType = StorageType::AdobeBrushLibrary;
            d->valid = fi.isReadable();
        }
        else if (d->location.endsWith(".asl")) {
            d->storagePlugin.reset(new KisAslStorage(location));
            d->storageType = StorageType::AdobeStyleLibrary;
            d->valid = fi.isReadable();
        }
    }
}

KisResourceStorage::~KisResourceStorage()
{
}

QString KisResourceStorage::name() const
{
    return d->name;
}

QString KisResourceStorage::location() const
{
    return d->location;
}

KisResourceStorage::StorageType KisResourceStorage::type() const
{
    return d->storageType;
}

QDateTime KisResourceStorage::timestamp() const
{
    return QFileInfo(d->location).lastModified();
}

KisResourceStorage::ResourceItem KisResourceStorage::resourceItem(const QString &url)
{
    return d->storagePlugin->resourceItem(url);
}

KoResourceSP KisResourceStorage::resource(const QString &url)
{
    return d->storagePlugin->resource(url);
}

QSharedPointer<KisResourceStorage::ResourceIterator> KisResourceStorage::resources(const QString &resourceType) const
{
    return d->storagePlugin->resources(resourceType);
}

QSharedPointer<KisResourceStorage::TagIterator> KisResourceStorage::tags(const QString &resourceType) const
{
    return d->storagePlugin->tags(resourceType);
}


bool KisResourceStorage::valid() const
{
    return d->valid;
}
