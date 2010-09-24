/* This file is part of the KDE project
 * Copyright (C) 2007, 2009 Thomas Zander <zander@kde.org>
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
#ifndef KOIMAGECOLLECTION_H
#define KOIMAGECOLLECTION_H

#include "flake_export.h"

#include <QObject>
#include <KoDataCenterBase.h>

class QImage;
class QUrl;
class KoStore;
class KoImageData;

/**
 * A collection of KoImageData objects to allow loading and saving them all together to the KoStore.
 * It also makes sure that if the same image is added to the collection that they share the internal data structure.
 */
class FLAKE_EXPORT KoImageCollection : public QObject, public KoDataCenterBase
{
    Q_OBJECT
public:
    /// constructor
    KoImageCollection(QObject *parent = 0);
    virtual ~KoImageCollection();

    /// reimplemented
    bool completeLoading(KoStore *store);

    /**
     * Save all images to the store which are in the context
     * @return returns true if save was successful (no images failed).
     */
    bool completeSaving(KoStore *store, KoXmlWriter *manifestWriter, KoShapeSavingContext *context);

    /**
     * Create a data object for the image data.
     * The collection will create an image data in a way that if there is an
     * existing data object with the same image the returned KoImageData will
     * share its data.
     * @param image a valid image which will be represented by the imageData.
     * @see KoImageData::isValid()
     */
    KoImageData *createImageData(const QImage &image);

    /**
     * Create a data object for the image data.
     * The collection will create an image data in a way that if there is an
     * existing data object with the same image the returned KoImageData will
     * share its data.
     * @param url a valid, local url to point to an image on the filesystem.
     * @see KoImageData::isValid()
     */
    KoImageData *createExternalImageData(const QUrl &url);

    /**
     * Create a data object for the image data.
     * The collection will create an image data in a way that if there is an
     * existing data object with the same image the returned KoImageData will
     * share its data.
     * @param href the name of the image inside the store.
     * @param store the KoStore object.
     * @see KoImageData::isValid()
     */
    KoImageData *createImageData(const QString &href, KoStore *store);

    /**
     * Create a data object for the image data.
     * The collection will create an image data in a way that if there is an
     * existing data object with the same image the returned KoImageData will
     * share its data.
     * @param imageData the bytes that represent the image in a format like png.
     * @see KoImageData::isValid()
     */
    KoImageData *createImageData(const QByteArray &imageData);

    void add(const KoImageData &data);
    void remove(const KoImageData &data);
    void removeOnKey(qint64 imageDataKey);

    bool fillFromKey(KoImageData &idata, qint64 imageDataKey);

    /**
     * Get the number of images inside the collection
     */
    int size() const;
    /**
     * Get the number of images inside the collection
     */
    int count() const;

private:
    KoImageData *cacheImage(KoImageData *data);

    class Private;
    Private * const d;
};

#endif // KOIMAGECOLLECTION_H
