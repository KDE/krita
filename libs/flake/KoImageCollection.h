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
#ifndef KOIMAGECOLLECTIONNEW_H
#define KOIMAGECOLLECTIONNEW_H

#include "flake_export.h"

#include <KoDataCenter.h>

class QImage;
class KUrl;
class QUrl;
class KoStore;
class KoImageData;
class KoImageDataPrivate;


/**
 * An collection of KoImageData objects to allow loading and saving them all together to the KoStore.
 * It also makes sure that if the same image is added to the collection that they share the internal data structure.
 */
class FLAKE_EXPORT KoImageCollection : public KoDataCenter
{
public:
    /// constructor
    KoImageCollection();
    virtual ~KoImageCollection();

    /// reimplemented
    bool completeLoading(KoStore *store);

    /**
     * Save all images to the store which are in the context
     * @return returns true if save was successful (no images failed).
     */
    bool completeSaving(KoStore *store, KoXmlWriter * manifestWriter, KoShapeSavingContext * context);

    // TODO rename 'getImage' to something more sane
    // return or create a data object for the image data
    KoImageData getImage(const QImage &image);
    KoImageData getExternalImage(const QUrl &url);
    KoImageData getImage(const QString &href, KoStore *store);

    void add(const KoImageData &data);
    void remove(const KoImageData &data);
    void removeOnKey(const QByteArray &imageDataKey);

    /**
     * Get the number of images inside the collection
     */
    int size() const;
    int count() const;

private:
    class Private;
    Private * const d;
};

#endif // KOIMAGECOLLECTIONNEW_H
