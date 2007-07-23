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

#include "koguiutils_export.h"

#include <KoShapeUserData.h>

#include <QPixmap>
#include <KUrl>

class KoImageCollection;
class QIODevice;

/**
 * Class meant to hold the full image data so it can be shared between image shapes.
 * In KOffice there is a picture shape and a krita shape which both can both show
 * an image.  To allow smooth transition of image data between shapes, as well as allowing
 * lower-resolution data to be shown this class will actually be the backing store of
 * the image data and it can create a pre-rendered QPixmap without deminishing the backing-store
 * data.
 * This class inherits from KoShapeUserData which means you can set it on any KoShape using
 * KoShape::setUserData() and get it using KoShape::userData().  The pictureshape plugin should
 * use this class' API to show its image data.
 * Such plugins are suggested to not make a copy of the pixmap data, but use the fact that this
 * image data caches one for every request to pixmap()
 */
class KOGUIUTILS_EXPORT KoImageData : public KoShapeUserData {
public:
    /**
     * The image quality that the pixmap() method will render to.
     * The DPI of the original image are read from the source file making this real units quality metrics.
     */
    enum ImageQuality {
        LowQuality,     // 50ppi
        MediumQuality,  // 100ppi
        HighQuality     // upto 150ppi
    };

    /**
     * The storate location
     */
    enum StorageLocation {
        SaveRelativeUrl,        ///< in the odf use a relative (to document) xlink:href, if possible
        SaveAbsoluteUrl,        ///< in the odf use a fully specified xlink:href
        SaveInStore,            ///< Save the image in the ODF store
        SaveInline              ///< Save the image serialized in the content.xml
    };

    /**
     * constructor
     * @param collection the image collection which will do the loading of the image data for us.
     */
    KoImageData(KoImageCollection *collection);

    /**
     * copy constructor using ref-counting.
     * @param imageData the other one.
     */
    KoImageData(const KoImageData &imageData);
    /// destructor
    ~KoImageData();

    /**
     * Alter the image quality rendered by this data object, will also remove the cached data.
     */
    void setImageQuality(ImageQuality quality);
    /**
     * Return the current image quality
     */
    ImageQuality imageQuality() const;

    /**
     * Renders a pixmap the first time you request it is called and returns it.
     * The rendering will use the ImageQuality set on this KoImageData object to determine how
     * much memory to spent on the pixmap.
     * @returns the cached pixmap
     */
    QPixmap pixmap();

    /**
     * Return the location of the external file.  Returns an empty URL if there is no external file.
     * @see image()
     */
    KUrl imageLocation() const;

    /**
     * Return the internal store of the image.
     * Will return a null image if the image is stored externally.
     * @see imageLocation()
     * @see QImage::isNull()
     */
    const QImage image() const;

    /// If using SaveInStore, the collection will set a url-like location using this method.
    void setStoreHref(const QString &href);
    /// returns the SaveInStore type url-like location
    QString storeHref() const;

    /**
     * Load the image data from the param device.
     * Note that if the file is bigger than 250Kb instead of loading the full file into memory it will
     * copy the data to a temp-file and postpone loading it until the first time pixmap() is called.
     * @para device the device that is used to get the data from.
     * @return returns true if load was successful.
     */
    bool loadFromStore(QIODevice *device);

    bool operator==(const KoImageData &other) {
        return other.d == d;
    }

private:
    class Private;
    Private * const d;
};

#endif

