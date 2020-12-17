/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007, 2009 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2007 Jan Hambrecht <jaham@gmx.net>
 * SPDX-FileCopyrightText: 2008 C. Boemann <cbo@boemann.dk>
 * SPDX-FileCopyrightText: 2008 Thorsten Zachmann <zachmann@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOIMAGEDATA_P_H
#define KOIMAGEDATA_P_H

#include <QUrl>
#include <QByteArray>
#include <QImage>
#include <QPixmap>
#include <QTimer>
#include <QDir>

#include "KoImageData.h"

class KoImageCollection;
class QTemporaryFile;

class KoImageDataPrivate
{
public:
    explicit KoImageDataPrivate(KoImageData *q);
    virtual ~KoImageDataPrivate();

    /**
     * Save the image data to the param device.
     * The full file is saved.
     * @param device the device that is used to get the data from.
     * @return returns true if load was successful.
     */
    bool saveData(QIODevice &device);

    /// store the suffix based on the full filename.
    void setSuffix(const QString &fileName);

    /// take the data from \a device and store it in the temporaryFile
    void copyToTemporary(QIODevice &device);

    /// clean the image cache.
    void cleanupImageCache();

    void clear();

    static qint64 generateKey(const QByteArray &bytes);

    enum DataStoreState {
        StateEmpty,     ///< No image data, either as url or as QImage
        StateNotLoaded, ///< Image data is set as Url
        StateImageLoaded,///< Image data is loaded from Url, so both are present.
        StateImageOnly  ///< Image data is stored in a QImage. There is no external storage.
    };

    KoImageCollection *collection;
    KoImageData::ErrorCode errorCode;
    QSizeF imageSize;
    qint64 key;
    QString suffix; // the suffix of the picture e.g. png  TODO use a QByteArray ?
    QTimer cleanCacheTimer;

    QAtomicInt refCount;

    // Image data store.
    DataStoreState dataStoreState;
    QUrl imageLocation;
    QImage image;
    /// screen optimized cached version.
    QPixmap pixmap;

    QTemporaryFile *temporaryFile;
};

#endif /* KOIMAGEDATA_P_H */
