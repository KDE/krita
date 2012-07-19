/*
 *  Copyright (c) 2002 patrick julien <freak@codepimps.org>
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KIS_PAINT_DEVICE_IMPL_H_
#define KIS_PAINT_DEVICE_IMPL_H_

#include <QObject>
#include <QRect>
#include <QVector>

#include "kis_debug.h"

#include <KoColorConversionTransformation.h>

#include "kis_types.h"
#include "kis_global.h"
#include "kis_shared.h"
#include "kis_default_bounds_base.h"

#include <krita_export.h>

class KUndo2Command;
class QRect;
class QImage;
class QPoint;
class QString;
class QColor;

class KoStore;
class KoColor;
class KoColorSpace;
class KoColorProfile;

class KisHLineIteratorNG;
class KisRandomSubAccessorPixel;
class KisDataManager;
class KisSelectionComponent;


typedef KisSharedPtr<KisDataManager> KisDataManagerSP;

/**
 * A paint device contains the actual pixel data and offers methods
 * to read and write pixels. A paint device has an integer x,y position
 * (i.e., are not positioned on the image with sub-pixel accuracy).
 * A KisPaintDevice doesn't have any fixed size, the size changes dynamically
 * when pixels are accessed by an iterator.
 */
class KRITAIMAGE_EXPORT KisPaintDevice
        : public QObject
        , public KisShared
{

    Q_OBJECT

public:

    /**
     * Create a new paint device with the specified colorspace.
     *
     * @param colorSpace the colorspace of this paint device
     * @param name for debugging purposes
     */
    KisPaintDevice(const KoColorSpace * colorSpace, const QString& name = QString());

    /**
     * Create a new paint device with the specified colorspace. The
     * parent node will be notified of changes to this paint device.
     *
     * @param parent the node that contains this paint device
     * @param colorSpace the colorspace of this paint device
     * @param defaultBounds boundaries of the device in case it is empty
     * @param name for debugging purposes
     */
    KisPaintDevice(KisNodeWSP parent, const KoColorSpace * colorSpace, KisDefaultBoundsBaseSP defaultBounds = 0, const QString& name = QString());

    KisPaintDevice(const KisPaintDevice& rhs);
    virtual ~KisPaintDevice();

protected:
    /**
     * A special constructor for usage in KisPixelSelection. It allows
     * two paint devices to share a data manager.
     *
     * @param explicitDataManager data manager to use inside paint device
     * @param src source paint device to copy parameters from
     * @param name for debugging purposes
     */
    KisPaintDevice(KisDataManagerSP explicitDataManager,
                   KisPaintDeviceSP src, const QString& name = QString());

public:

    /**
     * Write the pixels of this paint device into the specified file store.
     */
    virtual bool write(KoStore *store);

    /**
     * Fill this paint device with the pixels from the specified file store.
     */
    virtual bool read(KoStore *store);

public:

    /**
     * set the parent node of the paint device
     */
    void setParentNode(KisNodeWSP parent);

    /**
     * set the default bounds for the paint device when
     * the default pixel in not completely transarent
     */
    virtual void setDefaultBounds(KisDefaultBoundsBaseSP bounds);

     /**
     * the default bounds rect of the paint device
     */
    KisDefaultBoundsBaseSP defaultBounds() const;

    /**
     * Moves the device to these new coordinates (so no incremental move or so)
     */
    virtual void move(qint32 x, qint32 y);

    /**
     * Convenience method for the above
     */
    virtual void move(const QPoint& pt);

    /**
     * The X offset of the paint device
     */
    qint32 x() const;

    /**
     * The Y offset of the paint device
     */
    qint32 y() const;

    /**
     * set the X offset of the paint device
     */
    virtual void setX(qint32 x);

    /**
     * set the Y offset of the paint device
     */
    virtual void setY(qint32 y);

    /**
     * Retrieve the bounds of the paint device. The size is not exact,
     * but may be larger if the underlying datamanager works that way.
     * For instance, the tiled datamanager keeps the extent to the nearest
     * multiple of 64.
     *
     * If default pixel is not transparent, then the actual extent
     * rect is united with the defaultBounds()->bounds() value
     * (the size of the image, usually).
     */
    virtual QRect extent() const;

    /// Convience method for the above
    void extent(qint32 &x, qint32 &y, qint32 &w, qint32 &h) const;

    /**
     * Get the exact bounds of this paint device. The real solution is
     * very slow because it does a linear scanline search, but it
     * uses caching, so calling to this function without changing
     * the device is quite cheap.
     *
     * Exactbounds follows these rules:
     *
     * <ul>
     * <li>if default pixel is transparent, then exact bounds
     *     of actual pixel data are returned
     * <li>if default pixel is not transparent, then extent() of
     *     the device is returned.
     * </ul>
     * \see calculateExactBounds()
     */
    virtual QRect exactBounds() const;

    /**
     * Returns a rough approximation of region covered by device.
     * For tiled data manager, it region will consist of a number
     * of rects each corresponding to a tile.
     */
    virtual QRegion region() const;

    /**
     * Cut the paint device down to the specified rect. If the crop
     * area is bigger than the paint device, nothing will happen.
     */
    void crop(qint32 x, qint32 y, qint32 w, qint32 h);

    /// Convience method for the above
    void crop(const QRect & r);

    /**
     * Complete erase the current paint device. Its size will become 0. This
     * does not take the selection into account.
     */
    virtual void clear();

    /**
     * Clear the given rectangle to transparent black. The paint device will expand to
     * contain the given rect.
     */
    void clear(const QRect & rc);


    /**
     * Sets the default pixel. New data will be initialised with this pixel. The pixel is copied: the
     * caller still owns the pointer and needs to delete it to avoid memory leaks.
     */
    void setDefaultPixel(const quint8 *defPixel);

    /**
     * Get a pointer to the default pixel.
     */
    const quint8 *defaultPixel() const;

    /**
     * Fill the given rectangle with the given pixel. The paint device will expand to
     * contain the given rect.
     */
    void fill(const QRect & rc, const KoColor &color);

    /**
     * Overloaded function. For legacy purposes only.
     * Please use fill(const QRect & rc, const KoColor &color) instead
     */
    void fill(qint32 x, qint32 y, qint32 w, qint32 h, const quint8 *fillPixel);

public:

    /**
     * Prepares the device for fastBitBlt opreration. It clears
     * the device, switches x,y shifts and colorspace if needed.
     * After this call fastBitBlt will return true.
     * May be used for initialization of temporary devices.
     */
    void prepareClone(KisPaintDeviceSP src);

    /**
     * Make this device to become a clone of \a src. It will have the same
     * x,y shifts, colorspace and will share pixels inside \a rect.
     * After calling this function:
     * (this->extent() >= this->exactBounds() == rect).
     */
    void makeCloneFrom(KisPaintDeviceSP src, const QRect &rect);

    /**
     * Make this device to become a clone of \a src. It will have the same
     * x,y shifts, colorspace and will share pixels inside \a rect.
     * Be careful, this function will copy *at least* \a rect
     * of pixels. Actual copy area will be a bigger - it will
     * be aligned by tiles borders. So after calling this function:
     * (this->extent() == this->exactBounds() >= rect).
     */
    void makeCloneFromRough(KisPaintDeviceSP src, const QRect &minimalRect);


protected:
    friend class KisPaintDeviceTest;
    friend class DataReaderThread;

    /**
     * Checks whether a src paint device can be used as source
     * of fast bitBlt operation. The result of the check may
     * depend on whether color spaces coinside, whether there is
     * any shift of tiles between the devices and etc.
     *
     * WARNING: This check must be done <i>before</i> performing any
     * fast bitBlt operation!
     *
     * \see fastBitBlt
     * \see fastBitBltRough
     */
    bool fastBitBltPossible(KisPaintDeviceSP src);

    /**
     * Clones rect from another paint device. The cloned area will be
     * shared between both paint devices as much as possible using
     * copy-on-write. Parts of the rect that cannot be shared
     * (cross tiles) are deep-copied,
     *
     * \see fastBitBltPossible
     * \see fastBitBltRough
     */
    void fastBitBlt(KisPaintDeviceSP src, const QRect &rect);

    /**
     * The same as \ref fastBitBlt() but reads old data
     */
    void fastBitBltOldData(KisPaintDeviceSP src, const QRect &rect);

    /**
     * Clones rect from another paint device in a rough and fast way.
     * All the tiles touched by rect will be shared, between both
     * devices, that means it will copy a bigger area than was
     * requested. This method is supposed to be used for bitBlt'ing
     * into temporary paint devices.
     *
     * \see fastBitBltPossible
     * \see fastBitBlt
     */
    void fastBitBltRough(KisPaintDeviceSP src, const QRect &rect);

    /**
     * The same as \ref fastBitBltRough() but reads old data
     */
    void fastBitBltRoughOldData(KisPaintDeviceSP src, const QRect &rect);

public:
    /**
     * Read the bytes representing the rectangle described by x, y, w, h into
     * data. If data is not big enough, Krita will gladly overwrite the rest
     * of your precious memory.
     *
     * Since this is a copy, you need to make sure you have enough memory.
     *
     * Reading from areas not previously initialized will read the default
     * pixel value into data but not initialize that region.
     */
    void readBytes(quint8 * data, qint32 x, qint32 y, qint32 w, qint32 h) const;

    /**
     * Read the bytes representing the rectangle rect into
     * data. If data is not big enough, Krita will gladly overwrite the rest
     * of your precious memory.
     *
     * Since this is a copy, you need to make sure you have enough memory.
     *
     * Reading from areas not previously initialized will read the default
     * pixel value into data but not initialize that region.
     * @param data The address of the memory to receive the bytes read
     * @param rect The rectangle in the paint device to read from
     */
    void readBytes(quint8 * data, const QRect &rect);

    /**
     * Copy the bytes in data into the rect specified by x, y, w, h. If the
     * data is too small or uninitialized, Krita will happily read parts of
     * memory you never wanted to be read.
     *
     * If the data is written to areas of the paint device not previously initialized,
     * the paint device will grow.
     */
    void writeBytes(const quint8 * data, qint32 x, qint32 y, qint32 w, qint32 h);

    /**
     * Copy the bytes in data into the rectangle rect. If the
     * data is too small or uninitialized, Krita will happily read parts of
     * memory you never wanted to be read.
     *
     * If the data is written to areas of the paint device not previously initialized,
     * the paint device will grow.
     * @param data The address of the memory to write bytes from
     * @param rect The rectangle in the paint device to write to
     */
    void writeBytes(const quint8 * data, const QRect &rect);

    /**
     * Copy the bytes in the paint device into a vector of arrays of bytes,
     * where the number of arrays is the number of channels in the
     * paint device. If the specified area is larger than the paint
     * device's extent, the default pixel will be read.
     */
    QVector<quint8*> readPlanarBytes(qint32 x, qint32 y, qint32 w, qint32 h);

    /**
     * Write the data in the separate arrays to the channes. If there
     * are less vectors than channels, the remaining channels will not
     * be copied. If any of the arrays points to 0, the channel in
     * that location will not be touched. If the specified area is
     * larger than the paint device, the paint device will be
     * extended. There are no guards: if the area covers more pixels
     * than there are bytes in the arrays, krita will happily fill
     * your paint device with areas of memory you never wanted to be
     * read. Krita may also crash.
     *
     * XXX: what about undo?
     */
    void writePlanarBytes(QVector<quint8*> planes, qint32 x, qint32 y, qint32 w, qint32 h);

    /**
     * Converts the paint device to a different colorspace
     *
     * @return a command that can be used to undo the conversion.
     */
    KUndo2Command* convertTo(const KoColorSpace * dstColorSpace,
                             KoColorConversionTransformation::Intent renderingIntent = KoColorConversionTransformation::IntentPerceptual,
                             KoColorConversionTransformation::ConversionFlags conversionFlags = KoColorConversionTransformation::Empty);

    /**
     * Changes the profile of the colorspace of this paint device to the given
     * profile. If the given profile is 0, nothing happens.
     */
    void setProfile(const KoColorProfile * profile);

    /**
     * Fill this paint device with the data from image; starting at (offsetX, offsetY)
     * @param srcProfileName name of the RGB profile to interpret the image as. 0 is interpreted as sRGB
     */
    virtual void convertFromQImage(const QImage& image, const KoColorProfile *profile, qint32 offsetX = 0, qint32 offsetY = 0);

    /**
     * Create an RGBA QImage from a rectangle in the paint device.
     *
     * @param x Left coordinate of the rectangle
     * @param y Top coordinate of the rectangle
     * @param w Width of the rectangle in pixels
     * @param h Height of the rectangle in pixels
     * @param dstProfile RGB profile to use in conversion. May be 0, in which
     * case it's up to the color strategy to choose a profile (most
     * like sRGB).
     */
    virtual QImage convertToQImage(const KoColorProfile *dstProfile, qint32 x, qint32 y, qint32 w, qint32 h,
                                   KoColorConversionTransformation::Intent renderingIntent = KoColorConversionTransformation::IntentPerceptual,
                                   KoColorConversionTransformation::ConversionFlags conversionFlags = KoColorConversionTransformation::Empty) const;

    /**
     * Create an RGBA QImage from a rectangle in the paint device. The
     * rectangle is defined by the parent image's bounds.
     *
     * @param dstProfile RGB profile to use in conversion. May be 0, in which
     * case it's up to the color strategy to choose a profile (most
     * like sRGB).
     */
    virtual QImage convertToQImage(const KoColorProfile *  dstProfile,
                                   KoColorConversionTransformation::Intent renderingIntent = KoColorConversionTransformation::IntentPerceptual,
                                   KoColorConversionTransformation::ConversionFlags conversionFlags = KoColorConversionTransformation::Empty) const;

    /**
     * Creates a paint device thumbnail of the paint device, retaining
     * the aspect ratio. The width and height of the returned device
     * won't exceed \p maxw and \p maxw, but they may be smaller.
     *
     * @param maxw: maximum width
     * @param maxh: maximum height
     * @param selection: if present, only the selected pixels will be added to the thumbnail. May be 0
     * @param rect: only this rect will be used for the thumbnail
     *
     */
    virtual KisPaintDeviceSP createThumbnailDevice(qint32 w, qint32 h, QRect rect = QRect()) const;

    /**
     * Creates a thumbnail of the paint device, retaining the aspect ratio.
     * The width and height of the returned QImage won't exceed \p maxw and \p maxw, but they may be smaller.
     * The colors are not corrected for display!
     *
     * @param maxw: maximum width
     * @param maxh: maximum height
     * @param rect: only this rect will be used for the thumbnail
     */
    virtual QImage createThumbnail(qint32 maxw, qint32 maxh, QRect rect,
                                   KoColorConversionTransformation::Intent renderingIntent = KoColorConversionTransformation::IntentPerceptual,
                                   KoColorConversionTransformation::ConversionFlags conversionFlags = KoColorConversionTransformation::Empty);

    /**
     * Cached version of createThumbnail(qint32 maxw, qint32 maxh, const KisSelection *selection, QRect rect)
     */
    virtual QImage createThumbnail(qint32 maxw, qint32 maxh,
                                   KoColorConversionTransformation::Intent renderingIntent = KoColorConversionTransformation::IntentPerceptual,
                                   KoColorConversionTransformation::ConversionFlags conversionFlags = KoColorConversionTransformation::Empty);

    /**
     * Fill c and opacity with the values found at x and y.
     *
     * The color values will be transformed from the profile of
     * this paint device to the display profile.
     *
     * @return true if the operation was successful.
     */
    bool pixel(qint32 x, qint32 y, QColor *c) const;

    /**
     * Fill kc with the values found at x and y. This method differs
     * from the above in using KoColor, which can be of any colorspace
     *
     * The color values will be transformed from the profile of
     * this paint device to the display profile.
     *
     * @return true if the operation was successful.
     */
    bool pixel(qint32 x, qint32 y, KoColor * kc) const;

    /**
     * Set the specified pixel to the specified color. Note that this
     * bypasses KisPainter. the PaintDevice is here used as an equivalent
     * to QImage, not QPixmap. This means that this is not undoable; also,
     * there is no compositing with an existing value at this location.
     *
     * The color values will be transformed from the display profile to
     * the paint device profile.
     *
     * Note that this will use 8-bit values and may cause a significant
     * degradation when used on 16-bit or hdr quality images.
     *
     * @return true if the operation was successful
     */
    bool setPixel(qint32 x, qint32 y, const QColor& c);

    /// Convience method for the above
    bool setPixel(qint32 x, qint32 y, const KoColor& kc);

    /**
     * @return the colorspace of the pixels in this paint device
     */
    KoColorSpace * colorSpace();

    /**
     * @return the colorspace of the pixels in this paint device
     */
    const KoColorSpace * colorSpace() const;

    /**
     * @return the internal datamanager that keeps the pixels.
     */
    KisDataManagerSP dataManager() const;

    /**
     * Replace the pixel data, color strategy, and profile.
     */
    void setDataManager(KisDataManagerSP data, const KoColorSpace * colorSpace = 0);

    /**
     * Return the number of bytes a pixel takes.
     */
    virtual quint32 pixelSize() const;

    /**
     * Return the number of channels a pixel takes
     */
    virtual quint32 channelCount() const;

public:

    /**
     * Add the specified rect to the parent layer's set of dirty rects
     * (if there is a parent layer)
     */
    virtual void setDirty(const QRect & rc);

    /**
     *  Add the specified region to the parent layer's dirty region
     *  (if there is a parent layer)
     */
    virtual void setDirty(const QRegion & region);

    /**
     *  Set the parent layer completely dirty, if this paint device has
     *  as parent layer.
     */
    virtual void setDirty();

    virtual void setDirty(const QVector<QRect> rects);

public:

    KisHLineIteratorSP createHLineIteratorNG(qint32 x, qint32 y, qint32 w);

    KisHLineConstIteratorSP createHLineConstIteratorNG(qint32 x, qint32 y, qint32 w) const;

    KisVLineIteratorSP createVLineIteratorNG(qint32 x, qint32 y, qint32 h);

    KisVLineConstIteratorSP createVLineConstIteratorNG(qint32 x, qint32 y, qint32 h) const;

    KisRectIteratorSP createRectIteratorNG(qint32 x, qint32 y, qint32 w, qint32 h);
    KisRectIteratorSP createRectIteratorNG(const QRect& r);

    KisRectConstIteratorSP createRectConstIteratorNG(qint32 x, qint32 y, qint32 w, qint32 h) const;
    KisRectConstIteratorSP createRectConstIteratorNG(const QRect& r) const;

    KisRandomAccessorSP createRandomAccessorNG(qint32 x, qint32 y);

    KisRandomConstAccessorSP createRandomConstAccessorNG(qint32 x, qint32 y) const;

    /**
     * Create an iterator that will "artificially" extend the paint device with the
     * value of the border when trying to access values outside the range of data.
     *
     * @param rc indicates the rectangle that truly contains data
     */
    KisRepeatHLineConstIteratorSP createRepeatHLineConstIterator(qint32 x, qint32 y, qint32 w, const QRect& _dataWidth) const;
    /**
     * Create an iterator that will "artificially" extend the paint device with the
     * value of the border when trying to access values outside the range of data.
     *
     * @param rc indicates the rectangle that trully contains data
     */
    KisRepeatVLineConstIteratorSP createRepeatVLineConstIterator(qint32 x, qint32 y, qint32 h, const QRect& _dataWidth) const;

    /**
     * This function create a random accessor which can easily access to sub pixel values.
     * @param selection an up-to-date selection that has the same origin as the paint device
     */
    KisRandomSubAccessorSP createRandomSubAccessor() const;


    /** Clear the selected pixels from the paint device */
    void clearSelection(KisSelectionSP selection);

signals:

    void profileChanged(const KoColorProfile *  profile);
    void colorSpaceChanged(const KoColorSpace *colorspace);

private:
    friend class PaintDeviceCache;

    /**
     * Caclculates exact bounds of the device. Used internally
     * by a transparent caching system. The solution is very slow
     * because it does a linear scanline search. So the complexity
     * is n*n at worst.
     *
     * \see exactBounds()
     */
    QRect calculateExactBounds() const;

private:
    KisPaintDevice& operator=(const KisPaintDevice&);
    void init(KisDataManagerSP explicitDataManager,
              const KoColorSpace *colorSpace,
              KisDefaultBoundsBaseSP defaultBounds,
              KisNodeWSP parent, const QString& name);

    // Only KisPainter is allowed to have access to these low-level methods
    friend class KisPainter;

    /**
     * Get the number of contiguous columns starting at x, valid for all values
     * of y between minY and maxY.
     */
    qint32 numContiguousColumns(qint32 x, qint32 minY, qint32 maxY) const;

    /**
     * Get the number of contiguous rows starting at y, valid for all values
     * of x between minX and maxX.
     */
    qint32 numContiguousRows(qint32 y, qint32 minX, qint32 maxX) const;

    /**
     * Get the row stride at pixel (x, y). This is the number of bytes to add to a
     * pointer to pixel (x, y) to access (x, y + 1).
     */
    qint32 rowStride(qint32 x, qint32 y) const;

    /**
     * Return a vector with in order the size in bytes of the channels
     * in the colorspace of this paint device.
     */
    QVector<qint32> channelSizes();

private:
    friend class KisSelectionTest;
    KisNodeWSP parentNode() const;

private:
    KisDataManagerSP m_datamanager;

    struct Private;
    Private * const m_d;

};

#endif // KIS_PAINT_DEVICE_IMPL_H_
