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

#include "kis_debug.h"

#include <KoColorConversionTransformation.h>

#include "kis_types.h"
#include "kis_global.h"
#include "kis_shared.h"
#include "kis_iterators_pixel.h"

#include <krita_export.h>

class QUndoCommand;
class QRect;
class QImage;
class QPoint;
class QString;
class QColor;

class KoStore;
class KoColor;
class KoColorSpace;
class KoColorProfile;

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
     * @param parent the node that contains this paint device.
     * @param colorSpace the colorspace of this paint device
     * @param name for debugging purposes
     */
    KisPaintDevice(KisNodeWSP parent, const KoColorSpace * colorSpace, const QString& name = QString());

    KisPaintDevice(const KisPaintDevice& rhs);
    virtual ~KisPaintDevice();

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
    void setX(qint32 x);

    /**
     * set the Y offset of the paint device
     */
    void setY(qint32 y);

    /**
     * Retrieve the bounds of the paint device. The size is not exact,
     * but may be larger if the underlying datamanager works that way.
     * For instance, the tiled datamanager keeps the extent to the nearest
     * multiple of 64.
     */
    void extent(qint32 &x, qint32 &y, qint32 &w, qint32 &h) const;

    /// Convience method for the above
    virtual QRect extent() const;

    /**
     * Get the exact bounds of this paint device. This may be very slow,
     * especially on larger paint devices because it does a linear scanline search.
     */
    void exactBounds(qint32 &x, qint32 &y, qint32 &w, qint32 &h) const;

    /// Convience method for the above
    virtual QRect exactBounds() const;

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
     * Sets the default pixel. New data will be initialised with this pixel. The pixel is copied: the
     * caller still owns the pointer and needs to delete it to avoid memory leaks.
     */
    void setDefaultPixel(const quint8 *defPixel);

    /**
     * Get a pointer to the default pixel.
     */
    const quint8 *defaultPixel() const;

    /**
     * Clear the given rectangle to transparent black. The paint device will expand to
     * contain the given rect.
     */
    void clear(const QRect & rc);

    /**
     * Fill the given rectangle with the given pixel. The paint device will expand to
     * contain the given rect.
     */
    void fill(qint32 x, qint32 y, qint32 w, qint32 h, const quint8 *fillPixel);

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
    QUndoCommand* convertTo(const KoColorSpace * dstColorSpace, KoColorConversionTransformation::Intent renderingIntent = KoColorConversionTransformation::IntentPerceptual);

    /**
     * Changes the profile of the colorspace of this paint device to the given
     * profile. If the given profile is 0, nothing happens.
     */
    void setProfile(const KoColorProfile * profile);

    /**
     * Fill this paint device with the data from image; starting at (offsetX, offsetY)
     * @param srcProfileName name of the RGB profile to interpret the image as. "" is interpreted as sRGB
     */
    virtual void convertFromQImage(const QImage& image, const QString &srcProfileName, qint32 offsetX = 0, qint32 offsetY = 0);

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
    virtual QImage convertToQImage(const KoColorProfile *  dstProfile, qint32 x, qint32 y, qint32 w, qint32 h) const;

    /**
     * Create an RGBA QImage from a rectangle in the paint device. The
     * rectangle is defined by the parent image's bounds.
     *
     * @param dstProfile RGB profile to use in conversion. May be 0, in which
     * case it's up to the color strategy to choose a profile (most
     * like sRGB).
     */
    virtual QImage convertToQImage(const KoColorProfile *  dstProfile) const;

    /**
     * Creates a paint device thumbnail of the paint device, retaining
     * the aspect ratio. The width and height of the returned device
     * won't exceed \p maxw and \p maxw, but they may be smaller.
     */

    KisPaintDeviceSP createThumbnailDevice(qint32 w, qint32 h, const KisSelection *selection = 0) const;

    /**
     * Creates a thumbnail of the paint device, retaining the aspect ratio.
     * The width and height of the returned QImage won't exceed \p maxw and \p maxw, but they may be smaller.
     * The colors are not corrected for display!
     */
    virtual QImage createThumbnail(qint32 maxw, qint32 maxh, const KisSelection *selection = 0);

    /**
     * Fill c and opacity with the values found at x and y.
     *
     * The color values will be transformed from the profile of
     * this paint device to the display profile.
     *
     * @return true if the operation was successful.
     */
    bool pixel(qint32 x, qint32 y, QColor *c);

    /**
     * Fill kc with the values found at x and y. This method differs
     * from the above in using KoColor, which can be of any colorspace
     *
     * The color values will be transformed from the profile of
     * this paint device to the display profile.
     *
     * @return true if the operation was successful.
     */
    bool pixel(qint32 x, qint32 y, KoColor * kc);

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
    void setDataManager(KisDataManagerSP data, const KoColorSpace * colorSpace);

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

public:

    /**
     * Create an iterator over a rectangle section of a paint device, the path followed by
     * the iterator is not guaranteed, it is optimized for speed, which means that you shouldn't
     * use this type of iterator if you are combining two differents layers.
     * @param w width
     * @param h height
     * @param selection an up-to-date selection that has the same origin as the paint device
     * @return an iterator which points to the first pixel of an rectangle
     */
    KisRectIteratorPixel createRectIterator(qint32 left, qint32 top, qint32 w, qint32 h, const KisSelection * selection = 0);

    /**
     * Create an iterator over a rectangle section of a paint device, the path followed by
     * the iterator is not guaranteed, it is optimized for speed, which means that you shouldn't
     * use this type of iterator if you are combining two differents layers.
     * @param w width
     * @param h height
     * @param selection an up-to-date selection that has the same origin as the paint device* @return an iterator which points to the first pixel of an rectangle, this iterator
     * does not allow to change the pixel values
     */
    KisRectConstIteratorPixel createRectConstIterator(qint32 left, qint32 top, qint32 w, qint32 h, const KisSelection * selection = 0) const;

    /**
     * @param selection an up-to-date selection that has the same origin as the paint device
     * @return an iterator which points to the first pixel of a horizontal line, this iterator
     * does not allow to change the pixel values
     */
    KisHLineConstIteratorPixel createHLineConstIterator(qint32 x, qint32 y, qint32 w, const KisSelection * selection = 0) const;

    /**
     * Create an iterator that will "artificially" extend the paint device with the
     * value of the border when trying to access values outside the range of data.
     *
     * @param rc indicates the rectangle that truly contains data
     */
    KisRepeatHLineConstIteratorPixel createRepeatHLineConstIterator(qint32 x, qint32 y, qint32 w, const QRect& _dataWidth, const KisSelection * selection = 0) const;
    /**
     * Create an iterator that will "artificially" extend the paint device with the
     * value of the border when trying to access values outside the range of data.
     *
     * @param rc indicates the rectangle that trully contains data
     */
    KisRepeatVLineConstIteratorPixel createRepeatVLineConstIterator(qint32 x, qint32 y, qint32 h, const QRect& _dataWidth, const KisSelection * selection = 0) const;

    /**
    * @param selection an up-to-date selection that has the same origin as the paint device
     * @return an iterator which points to the first pixel of a horizontal line
     */
    KisHLineIteratorPixel createHLineIterator(qint32 x, qint32 y, qint32 w, const KisSelection * selection = 0);

    /**
     * @param selection an up-to-date selection that has the same origin as the paint device
     * This function return an iterator which points to the first pixel of a vertical line
     */
    KisVLineIteratorPixel createVLineIterator(qint32 x, qint32 y, qint32 h, const KisSelection * selection = 0);

    /**
     * @param selection an up-to-date selection that has the same origin as the paint device
     * This function return an iterator which points to the first pixel of a vertical line
     */
    KisVLineConstIteratorPixel createVLineConstIterator(qint32 x, qint32 y, qint32 h, const KisSelection * selection = 0) const;

    /**
     * This function creates a random accessor which allow to randomly access any pixels on
     * the paint device.
     * <b>Note:</b> random access is way slower than iterators, always use iterators whenever
     * you can.
     * @param selection an up-to-date selection that has the same origin as the paint device
     */
    KisRandomAccessorPixel createRandomAccessor(qint32 x, qint32 y, const KisSelection * selection = 0);

    /**
     * This function creates a random accessor which allow to randomly access any pixels on
     * the paint device.
     * <b>Note:</b> random access is way slower than iterators, always use iterators whenever
     * you can.
     * @param selection an up-to-date selection that has the same origin as the paint device
     */
    KisRandomConstAccessorPixel createRandomConstAccessor(qint32 x, qint32 y, const KisSelection * selection = 0) const;

    /**
     * This function create a random accessor which can easily access to sub pixel values.
     * @param selection an up-to-date selection that has the same origin as the paint device
     */
    KisRandomSubAccessorPixel createRandomSubAccessor(const KisSelection * selection = 0) const;


    /** Clear the selected pixels from the paint device */
    void clearSelection(KisSelectionSP selection);

    /**
     * Apply a mask to the image data, i.e. multiply each pixel's opacity by its
     * selectedness in the mask.
     */
    void applySelectionMask(KisSelectionSP mask);

signals:

    void ioProgress(qint8 percentage);
    void profileChanged(const KoColorProfile *  profile);
    void colorSpaceChanged(const KoColorSpace *colorspace);

private:

    KisPaintDevice& operator=(const KisPaintDevice&);

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

protected:

    KisDataManagerSP m_datamanager;

private:

    class Private;
    Private * const m_d;

};

#endif // KIS_PAINT_DEVICE_IMPL_H_
