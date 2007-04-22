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
#ifndef KOIMAGEDATA_H
#define KOIMAGEDATA_H

#include "kofficeui_export.h"

#include <KoShapeUserData.h>
#include <KoStoreDevice.h>

#include <QPixmap>
#include <KUrl>

class KoImageCollection;
class KoStoreDevice;

class KOFFICEUI_EXPORT KoImageData : public KoShapeUserData {
public:
    enum ImageQuality {
        LowQuality,
        MediumQuality,
        FullQuality
    };

    enum StorageLocation {
        SaveRelativeUrl,        ///< in the odf use a relative (to document) xlink:href, if possible
        SaveAbsoluteUrl,        ///< in the odf use a fully specified xlink:href
        SaveInStore,            ///< Save the image in the ODF store
        SaveInline              ///< Save the image serialized in the content.xml
    };

    KoImageData(KoImageCollection *collection);
    KoImageData(const KoImageData &imageData);
    ~KoImageData();

    void setImageQuality(ImageQuality quality);
    ImageQuality imageQuality() const;

    QPixmap pixmap();

    void setUrl(const KUrl &location);
    KUrl location() const;

    /// If using SaveInStore, the collection will set a url-like location using this method.
    void setStoreHref(const QString &href);
    /// returns the SaveInStore type url-like location
    QString storeHref() const;

    void setKoStoreDevice(KoStoreDevice *device);

    bool operator==(const KoImageData &other) {
        return other.d == d;
    }

private:
    class Private;
    Private * const d;
};

#endif

