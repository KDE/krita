/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007, 2009 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2008 Thorsten Zachmann <zachmann@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "KoImageCollection.h"
#include "KoImageData.h"
#include "KoImageData_p.h"
#include "KoShapeSavingContext.h"

#include <KoStoreDevice.h>
#include <QCryptographicHash>
#include <KoXmlWriter.h>

#include <QMap>
#include <FlakeDebug.h>
#include <KisMimeDatabase.h>


class Q_DECL_HIDDEN KoImageCollection::Private
{
public:
    ~Private()
    {
        Q_FOREACH (KoImageDataPrivate *id, images)
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
    QMap<qint64, QString> imagesToSave(context->imagesToSave());
    QMap<qint64, QString>::iterator imagesToSaveIter(imagesToSave.begin());

    QMap<qint64, KoImageDataPrivate *>::iterator knownImagesIter(d->images.begin());

    while (imagesToSaveIter != imagesToSave.end()) {
        if (knownImagesIter == d->images.end()) {
            // this should not happen
            warnFlake << "image not found";
            Q_ASSERT(0);
            break;
        }
        else if (knownImagesIter.key() == imagesToSaveIter.key()) {
            KoImageDataPrivate *imageData = knownImagesIter.value();
            if (store->open(imagesToSaveIter.value())) {
                KoStoreDevice device(store);
                bool ok = imageData->saveData(device);
                store->close();
                // TODO error handling
                if (ok) {
                    const QString mimetype = KisMimeDatabase::mimeTypeForFile(imagesToSaveIter.value(), false);
                    manifestWriter->addManifestEntry(imagesToSaveIter.value(), mimetype);
                } else {
                    warnFlake << "saving image" << imagesToSaveIter.value() << "failed";
                }
            } else {
                warnFlake << "saving image failed: open store failed";
            }
            ++knownImagesIter;
            ++imagesToSaveIter;
        } else if (knownImagesIter.key() < imagesToSaveIter.key()) {
            ++knownImagesIter;
        } else {
            // this should not happen
            warnFlake << "image not found";
            abort();
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

KoImageData *KoImageCollection::createImageData(const QString &href, KoStore *store)
{
    // the tricky thing with a 'store' is that we need to read the data now
    // as the store will no longer be readable after the loading completed.
    //
    // The solution we use is to read the data, store it in a QTemporaryFile
    // and read and parse it on demand when the image data is actually needed.
    // This leads to having two keys, one for the store and one for the
    // actual image data. We need the latter so if someone else gets the same
    // image data they can find this data and share (insert warm fuzzy feeling here).
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

void KoImageCollection::update(qint64 oldKey, qint64 newKey)
{
    if (oldKey == newKey) {
        return;
    }
    if (d->images.contains(oldKey)) {
        KoImageDataPrivate *imageData = d->images[oldKey];
        d->images.remove(oldKey);
        d->images.insert(newKey, imageData);
    }
}

void KoImageCollection::removeOnKey(qint64 imageDataKey)
{
    d->images.remove(imageDataKey);
}
