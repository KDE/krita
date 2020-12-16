/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007, 2009 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KOIMAGECOLLECTION_H
#define KOIMAGECOLLECTION_H

#include "kritaflake_export.h"

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
class KRITAFLAKE_EXPORT KoImageCollection : public QObject, public KoDataCenterBase
{
    Q_OBJECT
public:
    /// constructor
    explicit KoImageCollection(QObject *parent = 0);
    ~KoImageCollection() override;

    /// reimplemented
    bool completeLoading(KoStore *store) override;

    /**
     * Save all images to the store which are in the context
     * @return returns true if save was successful (no images failed).
     */
    bool completeSaving(KoStore *store, KoXmlWriter *manifestWriter, KoShapeSavingContext *context) override;

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

    /**
     * Update the imagecollection: the imagedata object with the old
     * key will now be associated with the new key.
     *
     * @param the key by which the object is known in the collection
     * @param they new key by which the object should be known
     */
    void update(qint64 oldKey, qint64 newKey);

private:
    KoImageData *cacheImage(KoImageData *data);

    class Private;
    Private * const d;
};

#endif // KOIMAGECOLLECTION_H
