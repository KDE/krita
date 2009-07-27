/* This file is part of the KDE project
 * Copyright (C) 2007, 2009 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2008 Casper Boemann <cbr@boemann.dk>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
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

#include <KoUnit.h>
#include <KoStore.h>
#include <KoStoreDevice.h>

#include <ktemporaryfile.h>
#include <kdebug.h>
#include <kio/netaccess.h>

#include <QBuffer>
#include <QCryptographicHash>
#include <QTemporaryFile>
#include <QFileInfo>
#include <QPainter>

#include "KoImageCollection.h"
#include "KoImageData_p.h"

KoImageData::KoImageData()
{
}

KoImageData::KoImageData(const KoImageData &imageData)
    : KoShapeUserData(),
    d(imageData.d)
{
}

KoImageData::KoImageData(KoImageDataPrivate *priv)
    : d(priv)
{
}

KoImageData::~KoImageData()
{
}

QPixmap KoImageData::pixmap(const QSize &size)
{
    QSize wantedSize = size;
    if (! wantedSize.isValid()) {
        if (d->pixmap.isNull()) // we have a problem, Houston..
            wantedSize = QSize(100, 100);
        else
            wantedSize = d->pixmap.size();
    }
    if (d->pixmap.isNull() || d->pixmap.size() != wantedSize) {
        switch (d->dataStoreState) {
        case KoImageDataPrivate::StateEmpty: {
            d->pixmap = QPixmap(1, 1);
            QPainter p(&d->pixmap);
            p.setPen(QPen(Qt::gray));
            p.drawPoint(0, 0);
            p.end();
            break;
        }
        case KoImageDataPrivate::StateNotLoaded:
            // load image
            if (d->temporaryFile) {
                d->temporaryFile->open();
                if (d->errorCode == Success && !d->image.load(d->temporaryFile, 0))
                    d->errorCode = OpenFailed;
                d->temporaryFile->close();
            } else {
                if (d->errorCode == Success && !d->image.load(d->imageLocation.toLocalFile()))
                    d->errorCode = OpenFailed;
            }
            if (d->errorCode == Success)
                d->dataStoreState = KoImageDataPrivate::StateImageLoaded;
            // fall through
        case KoImageDataPrivate::StateImageLoaded:
        case KoImageDataPrivate::StateImageOnly:
            if (! d->image.isNull()) {
                // create pixmap from image.
                d->pixmap = QPixmap(wantedSize.width(), wantedSize.height());
                QPainter p(&d->pixmap);
                p.drawImage(0, 0, d->image, 0, 0, wantedSize.width(), wantedSize.height());
                p.end();
            }
        }

        if (d->dataStoreState == KoImageDataPrivate::StateImageLoaded) {
            // TODO schedule an auto-unload of the big QImage in a couple of seconds.
        }
    }
    return d->pixmap;
}

bool KoImageData::hasCachedPixmap() const
{
    return d && !d->pixmap.isNull();
}

bool KoImageData::saveToFile(QIODevice &device) // TODO why is this public *and* on the private?
{
    return d->saveToFile(device);
}

const QSizeF KoImageData::imageSize()
{
    if (!d->imageSize.isValid()) {
        // The imagesize have not yet been calculated
        image(); // make sure the image is loaded

        if (d->image.dotsPerMeterX())
            d->imageSize.setWidth(DM_TO_POINT(d->image.width() / (qreal) d->image.dotsPerMeterX() * 10.0));
        else
            d->imageSize.setWidth(d->image.width() / 72.0);

        if (d->image.dotsPerMeterY())
            d->imageSize.setHeight(DM_TO_POINT(d->image.height() / (qreal) d->image.dotsPerMeterY() * 10.0));
        else
            d->imageSize.setHeight(d->image.height() / 72.0);
    }
    return d->imageSize;
}

QImage KoImageData::image() const
{
    // TODO
    if (d->image.isNull()) {
        d->image.loadFromData( d->rawData );
    }
    return d->image;
}

bool KoImageData::hasCachedImage() const
{
    return d && !d->image.isNull();
}

void KoImageData::setImage(const QImage &image, KoImageCollection *collection)
{
    if (collection) {
        // let the collection first check if it already has one. If it doesn't it'll call this method
        // again and we'll go to the other clause
        KoImageData other = collection->getImage(image);
        delete d.data();
        d = other.d;
    } else {
        if (d.data() == 0)
            d = new KoImageDataPrivate();
        d->image = image;
        d->imageLocation.clear();
        d->key = QString::number(image.cacheKey()).toLatin1();
        d->dataStoreState = KoImageDataPrivate::StateImageOnly;
    }
}

void KoImageData::setImage(const KUrl &url, KoImageCollection *collection)
{
    if (collection) {
        // let the collection first check if it already has one. If it doesn't it'll call this method
        // again and we'll go to the other clause
        KoImageData other = collection->getImage(url);
        delete d.data();
        d = other.d;
    } else {
        if (d.data() == 0)
            d = new KoImageDataPrivate();
        d->image = QImage();
        d->imageLocation = url;
        d->key = url.toEncoded();
        d->dataStoreState = KoImageDataPrivate::StateNotLoaded;
    }
}

void KoImageData::setImage(const QString &url, KoStore *store, KoImageCollection *collection)
{
    if (collection) {
        // let the collection first check if it already has one. If it doesn't it'll call this method
        // again and we'll go to the other clause
        KoImageData other = collection->getImage(url, store);
        delete d.data();
        d = other.d;
    } else {
        if (d.data() == 0) {
            d = new KoImageDataPrivate();
        } else {
            d->imageLocation.clear();
            d->key.clear();
            d->image = QImage();
        }

        if (store->open(url)) {
            KoStoreDevice device(store);
            if (!device.open(QIODevice::ReadOnly)) {
                kWarning(30006) << "open file from store " << url << "failed";
                d->errorCode = OpenFailed;
                return;
            }
            delete d->temporaryFile;
            d->temporaryFile = new QTemporaryFile("KoImageDataXXXXXX");
            if (!d->temporaryFile->open()) {
                kWarning(30006) << "open temporary file for writing failed";
                d->errorCode = StorageFailed;
                return;
            }
            QCryptographicHash md5(QCryptographicHash::Md5);
            char buf[8096];
            while (true) {
                device.waitForReadyRead(-1);
                qint64 bytes = device.readData(buf, sizeof(buf));
                if (bytes <= 0)
                    break; // done!
                md5.addData(buf, bytes);
                do {
                    bytes -= d->temporaryFile->write(buf, bytes);
                } while (bytes > 0);
            }
            d->key = md5.result();
            d->temporaryFile->close();

            QFileInfo fi(*d->temporaryFile);
            d->imageLocation = KUrl::fromPath(fi.absoluteFilePath());
            d->dataStoreState = KoImageDataPrivate::StateNotLoaded;
        } else {
            kWarning(30006) << "Find file in store " << url << "failed";
            d->errorCode = OpenFailed;
            return;
        }
    }
}

bool KoImageData::isValid() const
{
    return d;
}

bool KoImageData::operator==(const KoImageData &other) const
{
    return other.d == d;
}

KoImageData &KoImageData::operator=(const KoImageData &other)
{
    d = other.d;
    return *this;
}

QByteArray KoImageData::key() const
{
    return d->key;
}

QString KoImageData::suffix() const
{
    return d->suffix;
}

KoImageData::ErrorCode KoImageData::errorCode() const
{
    return d->errorCode;
}

void KoImageData::setSuffix(const QString & name)
{
    QRegExp rx("\\.([^/]+$)"); // TODO does this work on windows or do we have to use \ instead of / for a path separator?
    if (rx.indexIn(name) != -1) {
        d->suffix = rx.cap(1);
    }
}
