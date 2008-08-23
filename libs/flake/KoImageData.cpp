/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2008 Casper Boemann <cbr@boemann.dk>
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
#include <KoStore.h>

#include <KTemporaryFile>
#include <KDebug>
#include <QSizeF>
#include <QIODevice>
#include <QPainter>

class KoImageData::Private {
public:
    Private(KoImageCollection *c)
    : refCount(0)
    , quality(MediumQuality)
    , collection(c)
    , tempImageFile(0)
    , taggedForSaving(false)
    { }

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
    bool taggedForSaving;
};

KoImageData::KoImageData(KoImageCollection *collection, QString href)
    : d(new Private(collection))
{
    Q_ASSERT(collection);
    collection->addImage(this);
    d->storeHref = href;
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
        image(); // force loading if only present in a tmp file

        if(! d->image.isNull()) {
            int multiplier = 150; // max 150 ppi
            if(d->quality == NoPreviewImage) {
                d->pixmap = QPixmap(1,1);
                QPainter p(&d->pixmap);
                p.setPen(QPen(Qt::gray));
                p.drawPoint(0, 0);
                p.end();
                return d->pixmap;
            }
            if(d->quality == LowQuality)
                multiplier = 50;
            else if(d->quality == MediumQuality)
                multiplier = 100;
            int width = qMin(d->image.width(), qRound(imageSize().width() * multiplier / 72.));
            int height = qMin(d->image.height(), qRound(imageSize().height() * multiplier / 72.));
            // kDebug(30004)() <<"  image:" << width <<"x" << height;

            QImage scaled = d->image.scaled(width, height);
            if(d->tempImageFile) // free memory
                d->image = QImage();

            d->pixmap = QPixmap::fromImage(scaled);
        }
    }
    return d->pixmap;
}

KUrl KoImageData::imageLocation() const {
    return d->url;
}

QString KoImageData::tagForSaving() {
    static int counter=0;
    d->taggedForSaving=true;
    d->storeHref = QString("Pictures/image%1").arg(counter++);
    if(d->tempImageFile) {
        // we should set a suffix, unfortunately the tmp file don'thave a correct one set
        //d->storeHref += 
    }
    else
        // save as png if we don't have the original file data
        // also see saveToFile where we again hardcode to "PNG"
        d->storeHref += ".png";

    return d->storeHref;
}

QString KoImageData::storeHref() const {
    return d->storeHref;
}

bool KoImageData::saveToFile(QIODevice *device)
{
    if(d->tempImageFile) {
        if(! d->tempImageFile->open())
            return false;
        char * data = new char[32 * 1024];
        while(true) {
            bool failed = false;
            qint64 bytes = d->tempImageFile->read(data, 32*1024);
            if(bytes == 0)
                break;
            else if(bytes == -1) {
                kWarning() << "Failed to read data from the tmpfile\n";
                failed = true;
            }
            while(! failed && bytes > 0) {
                qint64 written = device->write(data, bytes);
                if(written < 0) {// error!
                    kWarning() << "Failed to copy the image from the temp to the store\n";
                    failed = true;
                }
                bytes -= written;
            }
            if(failed) { // read or write failed; so lets cleanly abort.
                 delete[] data;
                return false;
            }
        }
        delete[] data;
        return true;
    }
    else {
        return d->image.save(device,"PNG");
    }
}

bool KoImageData::isTaggedForSaving()
{
    return d->taggedForSaving;
}


bool KoImageData::loadFromFile(QIODevice *device) {
    struct Finally {
        Finally(QIODevice *d) : device (d), bytes(0) {}
        ~Finally() {
            delete device;
            delete[] bytes;
        }
        QIODevice *device;
        char *bytes;
    };
    Finally finally(device);

    // remove prev data
    delete d->tempImageFile;
    d->tempImageFile = 0;
    d->image = QImage();

    if(true) { //  right now we tmp file everything device->size() > 25E4) { // larger than 250Kb, save to tmp file.
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
            if(failed) { // read or write failed; so lets cleanly abort.
                delete d->tempImageFile;
                d->tempImageFile = 0;
                return false;
            }
        }
        d->url = d->tempImageFile->fileName();
        d->tempImageFile->close();
    }
    else { // small image; just load it in memory.
        d->image.load(device, 0);
    }
    return true;
}

const QSizeF KoImageData:: imageSize() {
    if(!d->imageSize.isValid()) {
        // The imagesize have not yet been calculated 

        image(); // make sure the image is loaded

        if(d->image.dotsPerMeterX())
            d->imageSize.setWidth( DM_TO_POINT(d->image.width() / (qreal) d->image.dotsPerMeterX() * 10.0) );
        else
            d->imageSize.setWidth( d->image.width() / 72.0);

        if(d->image.dotsPerMeterY())
            d->imageSize.setHeight( DM_TO_POINT(d->image.height() / (qreal) d->image.dotsPerMeterY() * 10.0) );
        else
            d->imageSize.setHeight( d->image.height() / 72.0);
    }
    return d->imageSize;
}

const QImage KoImageData::image() const {
    if(d->image.isNull() && d->tempImageFile) {
        d->tempImageFile->open();
        d->image.load(d->tempImageFile, 0);
            // kDebug(30004)() <<"  orig:" << d->image.width() <<"x" << d->image.height();
        d->tempImageFile->close();
    }

    return d->image;
}

void KoImageData::setImage( const QImage &image ) {
    // remove prev data
    delete d->tempImageFile;
    d->tempImageFile = 0;

    d->image = image;
}
