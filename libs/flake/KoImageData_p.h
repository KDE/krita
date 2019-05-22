/* This file is part of the KDE project
 * Copyright (C) 2007, 2009 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2008 C. Boemann <cbo@boemann.dk>
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

#ifndef KOIMAGEDATA_P_H
#define KOIMAGEDATA_P_H

#include <QUrl>
#include <QByteArray>
#include <QImage>
#include <QPixmap>
#include <QTimer>
#include <QDir>
#include <QSharedData>

#include <KisNewOnCopy.h>

#include "KoImageData.h"

class KoImageCollection;
class QTemporaryFile;

class KoImageDataPrivate : public QSharedData
{
public:
    KoImageDataPrivate();
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
    mutable KoImageData::ErrorCode errorCode;
    QSizeF imageSize;
    qint64 key;
    QString suffix; // the suffix of the picture e.g. png  TODO use a QByteArray ?
    KisNewOnCopy<QTimer> cleanCacheTimer;

    // Image data store.
    mutable DataStoreState dataStoreState;
    QUrl imageLocation;
    // XXX this should not be needed
    mutable QImage image;
    /// screen optimized cached version.
    QPixmap pixmap;

    QTemporaryFile *temporaryFile;
};

#endif /* KOIMAGEDATA_P_H */
