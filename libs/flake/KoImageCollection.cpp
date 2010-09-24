/* This file is part of the KDE project
 * Copyright (C) 2007, 2009 Thomas Zander <zander@kde.org>
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
#include "KoImageCollection.h"
#include "KoImageData.h"
#include "KoImageData_p.h"
#include "KoShapeSavingContext.h"

#include <KoStoreDevice.h>
#include <QCryptographicHash>
#include <KoXmlWriter.h>

#include <QMap>
#include <kdebug.h>
#include <kmimetype.h>

class KoImageCollection::Private
{
public:
    ~Private()
    {
        foreach(KoImageDataPrivate *id, images)
            id->collection = 0;
    }

    QMap<qint64, KoImageDataPrivate*> images;
    // an extra map to find all dataObjects based on the key of a store.
    QMap<QByteArray, KoImageDataPrivate*> storeImages;
};

KoImageCollection::KoImageCollection(QObject *parent)
    : QObject(parent),
    d(new Private())
{
}

KoImageCollection::~KoImageCollection()
{
    delete d;
}

bool KoImageCollection::completeLoading(KoStore *store)
{
    Q_UNUSED(store);
    d->storeImages.clear();
    return true;
}

bool KoImageCollection::completeSaving(KoStore *store, KoXmlWriter *manifestWriter, KoShapeSavingContext *context)
{
    QMap<qint64, QString> images(context->imagesToSave());
    QMap<qint64, QString>::iterator it(images.begin());

    QMap<qint64, KoImageDataPrivate *>::iterator dataIt(d->images.begin());

    while (it != images.end()) {
        if (dataIt == d->images.end()) {
            // this should not happen
            kWarning(30006) << "image not found";
            Q_ASSERT(0);
            break;
        }
        else if (dataIt.key() == it.key()) {
            KoImageDataPrivate *imageData = dataIt.value();
            if (imageData->imageLocation.isValid()) {
                // TODO store url
                Q_ASSERT(0); // not impleented yet
            }
            else if (store->open(it.value())) {
                KoStoreDevice device(store);
                bool ok = imageData->saveData(device);
                store->close();
                // TODO error handling
                if (ok) {
                    const QString mimetype(KMimeType::findByPath(it.value(), 0 , true)->name());
                    manifestWriter->addManifestEntry(it.value(), mimetype);
                } else {
                    kWarning(30006) << "saving image failed";
                }
            } else {
                kWarning(30006) << "saving image failed: open store failed";
            }
            ++dataIt;
            ++it;
        } else if (dataIt.key() < it.key()) {
            ++dataIt;
        } else {
            // this should not happen
            kWarning(30006) << "image not found";
            Q_ASSERT(0);
        }
    }
    return true;
}

KoImageData *KoImageCollection::createImageData(const QImage &image)
{
    Q_ASSERT(!image.isNull());
    KoImageData *data = new KoImageData();
    data->setImage(image);

    data = cacheImage(data);
    return data;
}

KoImageData *KoImageCollection::createExternalImageData(const QUrl &url)
{
    Q_ASSERT(!url.isEmpty() && url.isValid());

    QCryptographicHash md5(QCryptographicHash::Md5);
    md5.addData(url.toEncoded());
    qint64 key = KoImageDataPrivate::generateKey(md5.result());
    if (d->images.contains(key))
        return new KoImageData(d->images.value(key));
    KoImageData *data = new KoImageData();
    data->setExternalImage(url);
    data->priv()->collection = this;
    Q_ASSERT(data->key() == key);
    d->images.insert(key, data->priv());
    return data;
}

KoImageData *KoImageCollection::createImageData(const QString &href, KoStore *store)
{
    // the tricky thing with a 'store' is that we need to read the data now
    // as the store will no longer be readable after the loading completed.
    //
    // The solution we use is to read the data, store it in a KTemporaryFile
    // and read and parse it on demand when the image data is actually needed.
    // This leads to having two keys, one for the store and one for the
    // actual image data. We need the latter so if someone else gets the same
    // image data he can find this data and share (insert warm fuzzy feeling here).
    //
    QByteArray storeKey = (QString::number((qint64) store) + href).toLatin1();
    if (d->storeImages.contains(storeKey))
        return new KoImageData(d->storeImages.value(storeKey));

    KoImageData *data = new KoImageData();
    data->setImage(href, store);

    data = cacheImage(data);
    d->storeImages.insert(storeKey, data->priv());
    return data;
}

KoImageData *KoImageCollection::createImageData(const QByteArray &imageData)
{
    QCryptographicHash md5(QCryptographicHash::Md5);
    md5.addData(imageData);
    qint64 key = KoImageDataPrivate::generateKey(md5.result());
    if (d->images.contains(key))
        return new KoImageData(d->images.value(key));
    KoImageData *data = new KoImageData();
    data->setImage(imageData);
    data->priv()->collection = this;
    Q_ASSERT(data->key() == key);
    d->images.insert(key, data->priv());
    return data;
}

KoImageData *KoImageCollection::cacheImage(KoImageData *data)
{
    QMap<qint64, KoImageDataPrivate*>::const_iterator it(d->images.constFind(data->key()));
    if (it == d->images.constEnd()) {
        d->images.insert(data->key(), data->priv());
        data->priv()->collection = this;
    }
    else {
        delete data;
        data = new KoImageData(it.value());
    }
    return data;
}

bool KoImageCollection::fillFromKey(KoImageData &idata, qint64 key)
{
    if (d->images.contains(key)) {
        idata = KoImageData(d->images.value(key));
        return true;
    }
    return false;
}

int KoImageCollection::size() const
{
    return d->images.count();
}

int KoImageCollection::count() const
{
    return d->images.count();
}

void KoImageCollection::removeOnKey(qint64 imageDataKey)
{
    d->images.remove(imageDataKey);
}
