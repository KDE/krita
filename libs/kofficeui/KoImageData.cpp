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

#include "KoImageData.h"
#include "KoImageCollection.h"

#include <KoStoreDevice.h>

#include <QSizeF>
#include <KDebug>

class KoImageData::Private {
public:
    Private(KoImageCollection *c) : refCount(0), collection(c) { }
    KUrl url;
    long  modifiedData; // for reloading the image from disk

    int refCount;
    QSizeF imageSize;
    ImageQuality quality;
    QPixmap pixmap;
    QImage image; // this member holds the data in case the image is embedded.
    QString storeHref;
    KoImageCollection *collection;
};

KoImageData::KoImageData(KoImageCollection *collection)
    : d(new Private(collection))
{
    Q_ASSERT(collection);
    collection->addImage(this);
    Q_ASSERT(d->refCount == 1);
}

KoImageData::KoImageData(const KoImageData &imageData)
    : KoShapeUserData(),
    d(imageData.d)
{
    d->refCount++;
}

KoImageData::~KoImageData() {
    if(--d->refCount == 0) {
        d->collection->removeImage(this);
        delete d;
    }
}

void KoImageData::setImageQuality(KoImageData::ImageQuality quality) {
    if(d->quality == quality) return;
    d->pixmap = QPixmap(); // remove data
    d->quality  = quality;
}

KoImageData::ImageQuality KoImageData::imageQuality() const {
    return d->quality;
}

QPixmap KoImageData::pixmap() {
    if(d->pixmap.isNull()) {
        // TODO scaling
        d->pixmap = QPixmap::fromImage(d->image);
    }
    return d->pixmap;
}

void KoImageData::setUrl(const KUrl &location) {
    d->url = location;
}

KUrl KoImageData::location() const {
    return d->url;
}

void KoImageData::setStoreHref(const QString &href) {
    d->storeHref = href;
}

QString KoImageData::storeHref() const {
    return d->storeHref;
}

void KoImageData::setKoStoreDevice(KoStoreDevice *device) {
    d->image.load(device, 0);
    delete device;
}

