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

#include <KoUnit.h>
#include <KoStoreDevice.h>

#include <KTemporaryFile>
#include <KDebug>
#include <QSizeF>

class KoImageData::Private {
public:
    Private(KoImageCollection *c) : refCount(0), quality(LowQuality), collection(c), tempImageFile(0) { }
    ~Private() {
        delete tempImageFile;
    }
    KUrl url;
    long  modifiedData; // for reloading the image from disk

    int refCount;
    QSizeF imageSize;
    ImageQuality quality;
    QPixmap pixmap;
    QImage image; // this member holds the data in case the image is embedded.
    QString storeHref;
    KoImageCollection *collection;
    KTemporaryFile *tempImageFile;
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
        if(d->image.isNull() && d->tempImageFile) {
            d->tempImageFile->open();
            d->image.load(d->tempImageFile, 0);
            // kDebug() << "  orig: " << d->image.width() << "x" << d->image.height() << endl;
            d->tempImageFile->close();
            d->imageSize.setWidth( DM_TO_POINT(d->image.width() / (double) d->image.dotsPerMeterX() * 10.0) );
            d->imageSize.setHeight( DM_TO_POINT(d->image.height() / (double) d->image.dotsPerMeterY() * 10.0) );
        }

        if(! d->image.isNull()) {
            int multiplier = 150; // max 150 ppi
            if(d->quality == LowQuality)
                multiplier = 50;
            else if(d->quality == MediumQuality)
                multiplier = 100;
            int width = qMin(d->image.width(), qRound(d->imageSize.width() * multiplier / 72.));
            int height = qMin(d->image.height(), qRound(d->imageSize.height() * multiplier / 72.));
            // kDebug() << "  image: " << width << "x" << height << endl;

            QImage scaled = d->image.scaled(width, height);
            if(d->tempImageFile) // free memory
                d->image = QImage();

            d->pixmap = QPixmap::fromImage(scaled);
        }
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

bool KoImageData::setKoStoreDevice(KoStoreDevice *device) {
    struct Finally {
        Finally(KoStoreDevice *d) : device (d), bytes(0) {}
        ~Finally() {
            delete device;
            delete[] bytes;
        }
        KoStoreDevice *device;
        char *bytes;
    };
    Finally finally(device);

    if(device->size() > 25E4) { // larger than 250Kb, save to tmp file.
        d->tempImageFile = new KTemporaryFile();
        if(! d->tempImageFile->open())
            return false;
        char * data = new char[32 * 1024];
        finally.bytes = data;
        while(true) {
            bool failed = false;
            qint64 bytes = device->read(data, 32*1024);
            if(bytes == 0)
                break;
            else if(bytes == -1) {
                kWarning() << "Failed to read data from the store\n";
                failed = true;
            }
            while(! failed && bytes > 0) {
                qint64 written = d->tempImageFile->write(data, bytes);
                if(written < 0) {// error!
                    kWarning() << "Failed to copy the image from the store to temp\n";
                    failed = true;
                }
                bytes -= written;
            }
            if(failed) { // read or write failed; so lets clealy abort.
                delete d->tempImageFile;
                d->tempImageFile = 0;
                return false;
            }
        }
        d->tempImageFile->close();
    }
    else // small image; just load it in memory.
        d->image.load(device, 0);
    return true;
}

