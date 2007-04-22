/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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
#include "KoImageCollection.h"
#include "KoImageData.h"

#include <QList>
#include <KDebug>

class KoImageCollection::Private {
public:
    QList<KoImageData*> images;
};

KoImageCollection::KoImageCollection()
    : d(new Private())
{
}

KoImageCollection::~KoImageCollection() {
    delete d;
}

void KoImageCollection::addImage(KoImageData *image) {
    d->images.append(new KoImageData(*image));
}

void KoImageCollection::removeImage(KoImageData *image) {
    foreach(KoImageData *data, d->images) {
        if(data->operator==(*image)) {
            d->images.removeAll(data);
            delete data;
        }
    }
}

bool KoImageCollection::loadFromStore(KoStore *store) {
    foreach(KoImageData *image, d->images) {
        if(! store->open(image->storeHref()))
            return false;
        image->setKoStoreDevice(new KoStoreDevice(store));
        store->close();
    }
    return true;
}

