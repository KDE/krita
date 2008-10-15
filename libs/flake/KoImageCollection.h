/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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
    ~KoImageCollection();

    /// reimplmented
    bool completeLoading(KoStore *store);

    /**
     * Save all images to the store which are in the context
     * @return returns true if save was successful (no images failed).
     */
    bool completeSaving(KoStore *store, KoXmlWriter * manifestWriter, KoShapeSavingContext * context);

    KoImageData * getImage(const QImage & image);
    KoImageData * getImage(const KUrl & url);
    KoImageData * getImage(const QString & href, KoStore * store);

    /**
     * Get the number of images inside the collection
     */
    int size() const;

protected:
    void lookup(KoImageData * image);
    friend class KoImageDataPrivate;
    void removeImage(KoImageDataPrivate * image);

private:
    class Private;
    Private * const d;
};

#endif // KOIMAGECOLLECTIONNEW_H
