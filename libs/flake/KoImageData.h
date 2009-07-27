/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
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
#ifndef KOIMAGEDATA_H
#define KOIMAGEDATA_H

#include "flake_export.h"

#include <QExplicitlySharedDataPointer>
#include <QSize>

#include <KoShapeUserData.h>

class QIODevice;
class QPixmap;
class QImage;
class QSizeF;
class KUrl;
class KoImageCollection;
class KoImageDataPrivate;
class KoStore;

/**
 * This class is meant to represent the image data so it can be shared between image shapes.
 * In KOffice there is a picture shape and a krita shape which both can both show
 * an image.  To allow smooth transition of image data between shapes, as well as allowing
 * lower-resolution data to be shown this class will actually be the backing store of
 * the image data and it can create a pre-rendered QPixmap without deminishing the backing-store
 * data.
 * This class inherits from KoShapeUserData which means you can set it on any KoShape using
 * KoShape::setUserData() and get it using KoShape::userData().  The pictureshape plugin
 * uses this class to show its image data.
 * Such plugins are suggested to not make a copy of the pixmap data, but use the fact that this
 * image data caches one for every request to pixmap()
 */
class FLAKE_EXPORT KoImageData : public KoShapeUserData
{
public:
    /**
     * The image quality that the pixmap() method will render to.
     * The DPI of the original image is read from the source file making this real units quality metrics.
     */
    enum ImageQuality {
        NoPreviewImage,      ///<  shows a dummy.
        LowQuality,     ///< 50ppi
        MediumQuality,  ///< 100ppi
        HighQuality     ///< upto 150ppi
    };

#if 0
    /**
     * The storate location
     */
    enum StorageLocation {
        SaveRelativeUrl,        ///< in the odf use a relative (to document) xlink:href, if possible
        SaveAbsoluteUrl,        ///< in the odf use a fully specified xlink:href
        SaveInStore,            ///< Save the image in the ODF store
        SaveInline              ///< Save the image serialized in the content.xml
    };
#endif

    /// Various error codes representing what has gone wrong
    enum ErrorCode {
        Success,
        OpenFailed,
        StorageFailed, ///< This is set if the image data has to be stored on disk in a temporary file, but we failed to do so
        LoadFailed
    };

    /// default constructor, creates an invalid imageData object
    KoImageData();

    /**
     * copy constructor
     * @param imageData the other one.
     */
    KoImageData(const KoImageData &imageData);
    /// destructor
    virtual ~KoImageData();

#if 0
    /**
     * Alter the image quality rendered by this data object, will also remove the cached data.
     */
    void setImageQuality(ImageQuality quality);
    // TODO remove ImageQuality
    /**
     * Return the current image quality
     */
    ImageQuality imageQuality() const;
#endif

    /**
     * Renders a pixmap the first time you request it is called and returns it.
     * @returns the cached pixmap
     */
    QPixmap pixmap(const QSize &targetSize = QSize());

    /// returns true only if pixmap() would return immediately with a cached pixmap
    bool hasCachedPixmap();

    void setImage(const QImage &image, KoImageCollection *collection = 0);
    void setImage(const KUrl &image, KoImageCollection *collection = 0);
    void setImage(const QString &location, KoStore *store, KoImageCollection *collection = 0);
    //void setImage(const QString &image, KoImageCollection *collection = 0); // TODO can I get rid of this one due to it auto-converting to KUrl ?

    /**
     * Return the internal store of the image.
     * @see QImage::isNull(), isValid()
     */
    QImage image() const; // TODO why do we want to expose the image? saveToFile() should be enough, no?

    /**
     * Save the image data to the param device.
     * The full file is saved.
     * @param device the device that is used to get the data from.
     * @return returns true if load was successful.
     */
    bool saveToFile(QIODevice &device);

    /**
     * The size of the image in points
     */
    const QSizeF imageSize();

    KoImageData &operator=(const KoImageData &other);

    bool operator!=(const KoImageData &other) const { return !operator==(other); }
    bool operator==(const KoImageData &other) const;

    /**
     * Get a unique key of the image data
     */
    QByteArray key() const; // TODO make this a qint64

    QString suffix() const;

    ErrorCode errorCode() const;

    /// returns if this is a valid imageData with actual image data present on it.
    bool isValid() const;

    /// \internal
    KoImageDataPrivate *priv();

signals:
    void imageLoaded();

private:
    /**
     * Only the image collection is able to create the KoImageData.
     * This is done to make sure the same images get the same KoImageData::Private pointer 
     * and reuse it.
     */
    friend class KoImageCollection;
#if 0
    // TODO get rid of these and just have setters.
    /**
     * constructor
     * This is private to only allow the KoImageCollection to create new KoImageData objects
     *
     * @param collection the image collection which will do the loading of the image data for us.
     */
    KoImageData(KoImageCollection *collection, const QImage &image);
    /**
     * constructor
     * This is private to only allow the KoImageCollection to create new KoImageData objects
     */
    KoImageData(KoImageCollection *collection, const KUrl &url);
    /**
     * constructor
     * This is private to only allow the KoImageCollection to create new KoImageData objects
     */
    KoImageData(KoImageCollection *collection, const QString &href, KoStore *store);
#endif

    KoImageData(KoImageDataPrivate *priv);

    // TODO move these to the private
    /**
     * Load the image data from the \a device.
     * Note that if the file is bigger than 250Kb instead of loading the full file into memory it will
     * copy the data to a temp-file and postpone loading it until the first time pixmap() is called.
     * @param device the device that is used to get the data from.
     * @return returns true if load was successful.
     */
    bool loadFromFile(QIODevice &device);

    // TODO move these to the private
    void setSuffix(const QString &name);

    QExplicitlySharedDataPointer<KoImageDataPrivate> d;
};

#endif
