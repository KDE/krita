/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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
#include <QIODevice>
#include <QPainter>

#include "KoImageCollection.h"
#include "KoImageData_p.h"

KoImageData::KoImageData(KoImageCollection *collection, const QImage &image)
    : d(new KoImageDataPrivate(collection))
{
    d->image = image;
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG"); // use .png for images we get as QImage
    QCryptographicHash ch(QCryptographicHash::Md5);
    ch.addData(ba);
    d->key = ch.result();
    d->suffix = "png";
}

KoImageData::KoImageData(KoImageCollection *collection, const KUrl &url)
    : d(new KoImageDataPrivate(collection))
{
    QString tmpFile;
    if (KIO::NetAccess::download(url, tmpFile, 0)) {
        QFile file(tmpFile);
        file.open(QIODevice::ReadOnly);
        loadFromFile(file);
        setSuffix(url.prettyUrl());
    } else {
        kWarning(30006) << "open image " << url.prettyUrl() << "failed";
        d->errorCode = OpenFailed;
    }
}

KoImageData::KoImageData(KoImageCollection *collection, const QString &href, KoStore *store)
    : d(new KoImageDataPrivate(collection))
{
    if (store->open(href)) {
        // TODO should we use KoStore::extractFile ?
        KoStoreDevice device(store);
        loadFromFile(device);
        setSuffix(href);
        store->close();
    } else {
        kWarning(30006) << "open image " << href << "failed";
        d->errorCode = OpenFailed;
    }
}

KoImageData::KoImageData(const KoImageData &imageData)
    : KoShapeUserData(),
    d(imageData.d)
{
}

KoImageData::~KoImageData()
{
}

void KoImageData::setImageQuality(KoImageData::ImageQuality quality)
{
    if (d->quality == quality) return;
    d->pixmap = QPixmap(); // remove data
    d->quality  = quality;
}

KoImageData::ImageQuality KoImageData::imageQuality() const
{
    return d->quality;
}

QPixmap KoImageData::pixmap()
{
    if (d->pixmap.isNull()) {
        if (d->quality == NoPreviewImage) {
            d->image = QImage(); // free memory
            d->pixmap = QPixmap(1, 1);
            QPainter p(&d->pixmap);
            p.setPen(QPen(Qt::gray));
            p.drawPoint(0, 0);
            p.end();
            return d->pixmap;
        }
        image(); // force loading if only present if only present as raw data

        if (!d->image.isNull()) {
            int multiplier = 150; // max 150 ppi
            if (d->quality == LowQuality)
                multiplier = 50;
            else if (d->quality == MediumQuality)
                multiplier = 100;
            int width = qMin(d->image.width(), qRound(imageSize().width() * multiplier / 72.));
            int height = qMin(d->image.height(), qRound(imageSize().height() * multiplier / 72.));
            // kDebug(30006)() <<"  image:" << width <<"x" << height;

            QImage scaled = d->image.scaled(width, height);
            if (!d->rawData.isEmpty()) { // free memory
                d->image = QImage();
            }

            d->pixmap = QPixmap::fromImage(scaled);
        }
    }
    return d->pixmap;
}

bool KoImageData::saveToFile(QIODevice &device)
{
    return d->saveToFile(device);
}

bool KoImageData::loadFromFile(QIODevice &device)
{
    d->rawData = device.readAll();
    bool loaded = d->image.loadFromData(d->rawData);
    if (loaded) {
        QCryptographicHash ch(QCryptographicHash::Md5);
        ch.addData(d->rawData);
        d->key = ch.result();
    } else {
        d->errorCode = LoadFailed;
    }
    return loaded;
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

const QImage KoImageData::image() const
{
    if (d->image.isNull()) {
        d->image.loadFromData( d->rawData );
    }
    return d->image;
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
