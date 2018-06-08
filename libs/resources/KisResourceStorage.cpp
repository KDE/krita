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

#include <kzip.h>

#include <QFileInfo>

class KisResourceStorage::Private {
public:
    QString location;
    bool valid {false};
    KisResourceStorage::StorageType storageType {KisResourceStorage::StorageType::Unknown};
};


KisResourceStorage::KisResourceStorage(const QString &location)
    : d(new Private())
{
    d->location = location;
    QFileInfo fi(d->location);
    if (fi.isDir()) {
        d->storageType = StorageType::Folder;
        d->valid = fi.isWritable();
    }
    else {
        if (d->location.endsWith(".bundle")) {
            d->storageType = StorageType::Bundle;
            // XXX: should we also check whether there's a valid metadata entry? Or is this enough?
            d->valid = (fi.isReadable() && KZip(d->location).open(QIODevice::ReadOnly));
        }
        else if (d->location.endsWith(".abr")) {
            d->storageType = StorageType::AdobeBrushLibrary;
            d->valid = fi.isReadable();
        }
        else if (d->location.endsWith(".asl")) {
            d->storageType = StorageType::AdobeStyleLibrary;
            d->valid = fi.isReadable();
        }
    }
}

KisResourceStorage::~KisResourceStorage()
{

}

KisResourceStorage::StorageType KisResourceStorage::type() const
{
    return d->storageType;
}


bool KisResourceStorage::valid() const
{
    return d->valid;
}
