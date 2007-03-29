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

#include <QColor>
#include <QObject>
#include <QPixmap>
#include <QRect>
#include <QList>
#include <QString>
#include <QPaintDevice>
#include <QPaintEngine>

#include "kis_types.h"
#include "kdebug.h"
#include "kis_global.h"
#include "kis_image.h"
#include "KoColorSpace.h"
#include "KoColor.h"
#include <krita_export.h>
#include <kis_shared.h>

class QImage;
class QSize;
class QPoint;
class QMatrix;
class QTimer;
class QPaintEngine;
class KNamedCommand;
class QUndoCommand;

class KoStore;

class KisExifInfo;
class KisImage;
class KisRandomSubAccessorPixel;
class KisFilter;
class KisDataManager;
typedef KisSharedPtr<KisDataManager> KisDataManagerSP;

class KisMemento;
typedef KisSharedPtr<KisMemento> KisMementoSP;


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
    , public QPaintDevice
{

        Q_OBJECT

public:

    /**
     * Create a new paint device with the specified colorspace.
     *
     * @param colorSpace the colorspace of this paint device
     * @param name for debugging purposes
     */
    KisPaintDevice(KoColorSpace * colorSpace, const QString& name = QString());

    /**
     * Create a new paint device with the specified colorspace. The
     * parentLayer will be notified of changes to this paint device.
     *
     * @param parentLayer the layer that contains this paint device.
     * @param colorSpace the colorspace of this paint device
     * @param name for debugging purposes
     */
    KisPaintDevice(KisLayer *parentLayer, KoColorSpace * colorSpace, const QString& name = QString());

    KisPaintDevice(const KisPaintDevice& rhs);
    virtual ~KisPaintDevice();

public:

    /**
     * Returns an instance of KisPaintEngine for QPainters to work with.
     */
    QPaintEngine * paintEngine () const;

protected:

    /**
     * Returns the metric information for the given paint device
     * metric.
     */
    int metric( PaintDeviceMetric metric ) const;

public:
    /**
     * Start the long-running background filters. This is typically done from
     * paint layers
     */
    void startBackgroundFilters();

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
     * Moves the device to these new coordinates (so no incremental move or so)
     */
    virtual void move(qint32 x, qint32 y);

    /**
     * Convenience method for the above
     */
    virtual void move(const QPoint& pt);

    /**
     * Returns true of x,y is within the extent of this paint device
     */
    bool contains(qint32 x, qint32 y) const;

    /**
     * Convenience method for the above
     */
    bool contains(const QPoint& pt) const;

    /**
     * Retrieve the bounds of the paint device. The size is not exact,
     * but may be larger if the underlying datamanager works that way.
     * For instance, the tiled datamanager keeps the extent to the nearest
     * multiple of 64.
     */
    void extent(qint32 &x, qint32 &y, qint32 &w, qint32 &h) const;
    virtual QRect extent() const;

    /**
     * Retrieve the region, that is the collection of non-overlapping
     * rects, that contain actual pixels in the paint device. For
     * instance, given a transparent paint device with two 10x10 black
     * spots, at 50,50 and 2000, 1800, the result would be a region
     * with two rects, 50,50 10x10 and 2000,1800 10x10 -- in the best case.
     * The implementation may optimize by putting a margin aound it.
     */
    virtual QRegion region() const;

    /**
     * XXX: This should be a temporay hack, awaiting a proper fix.
     *
     * Indicates whether the extent really represents the extent. For example,
     * the KisBackground checkerboard pattern is generated by filling the
     * default tile but it will return an empty extent.
     */
    bool extentIsValid() const;

    /// Convience method for the above
    void setExtentIsValid(bool isValid);

    /**
     * Get the exact bounds of this paint device. This may be very slow,
     * especially on larger paint devices because it does a linear scanline search.
     */
    void exactBounds(qint32 &x, qint32 &y, qint32 &w, qint32 &h) const;
    virtual QRect exactBounds() const;

    /**
     * Cut the paint device down to the specified rect
     */
    void crop(qint32 x, qint32 y, qint32 w, qint32 h);

    /// Convience method for the above
    void crop(QRect r);

    /**
     * Complete erase the current paint device. Its size will become 0.
     */
    virtual void clear();

    /**
     * Fill the given rectangle with the given pixel.
     */
    void fill(qint32 x, qint32 y, qint32 w, qint32 h, const quint8 *fillPixel);

    /**
     * Clear the given rectangle to transparent black
     */
    void clear( const QRect & rc );

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
    virtual void readBytes(quint8 * data, qint32 x, qint32 y, qint32 w, qint32 h);

    /**
     * Copy the bytes in data into the rect specified by x, y, w, h. If the
     * data is too small or uninitialized, Krita will happily read parts of
     * memory you never wanted to be read.
     *
     * If the data is written to areas of the paint device not previously initialized,
     * the paint device will grow.
     */
    virtual void writeBytes(const quint8 * data, qint32 x, qint32 y, qint32 w, qint32 h);

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
     *   Converts the paint device to a different colorspace
     */
    virtual void convertTo(KoColorSpace * dstColorSpace, qint32 renderingIntent = INTENT_PERCEPTUAL);

    /**
     * Changes the profile of the colorspace of this paint device to the given
     * profile. If the given profile is 0, nothing happens.
     */
    virtual void setProfile(KoColorProfile * profile);

    /**
     * Fill this paint device with the data from img; starting at (offsetX, offsetY)
     * @param srcProfileName name of the RGB profile to interpret the img as. "" is interpreted as sRGB
     */
    virtual void convertFromQImage(const QImage& img, const QString &srcProfileName, qint32 offsetX = 0, qint32 offsetY = 0);

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
     * @param exposure The exposure setting used to render a preview of a high dynamic range image.
     */
    virtual QImage convertToQImage(KoColorProfile *  dstProfile, qint32 x, qint32 y, qint32 w, qint32 h, float exposure = 0.0f);

    /**
     * Create an RGBA QImage from a rectangle in the paint device. The rectangle is defined by the parent image's bounds.
     *
     * @param dstProfile RGB profile to use in conversion. May be 0, in which
     * case it's up to the color strategy to choose a profile (most
     * like sRGB).
     * @param exposure The exposure setting used to render a preview of a high dynamic range image.
     */
    virtual QImage convertToQImage(KoColorProfile *  dstProfile, float exposure = 0.0f);

    /**
     * Creates a paint device thumbnail of the paint device, retaining the aspect ratio.
     * The width and height of the returned device won't exceed \p maxw and \p maxw, but they may be smaller.
     */

    KisPaintDeviceSP createThumbnailDevice(qint32 w, qint32 h) const;

    /**
     * Creates a thumbnail of the paint device, retaining the aspect ratio.
     * The width and height of the returned QImage won't exceed \p maxw and \p maxw, but they may be smaller.
     * The colors are not corrected for display!
     */
    virtual QImage createThumbnail(qint32 maxw, qint32 maxh);


    /**
     * Fill c and opacity with the values found at x and y.
     *
     * The color values will be transformed from the profile of
     * this paint device to the display profile.
     *
     * @return true if the operation was successful.
     */
    bool pixel(qint32 x, qint32 y, QColor *c, quint8 *opacity);


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
     * Return the KoColor of the pixel at x,y.
     */
    KoColor colorAt(qint32 x, qint32 y);

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
     *
     */
    bool setPixel(qint32 x, qint32 y, const QColor& c, quint8 opacity);

    bool setPixel(qint32 x, qint32 y, const KoColor& kc);

    KoColorSpace * colorSpace() const;

    KisDataManagerSP dataManager() const;

    /**
     * Replace the pixel data, color strategy, and profile.
     */
    void setData(KisDataManagerSP data, KoColorSpace * colorSpace);

    /**
     * The X offset of the paint device
     */
    qint32 getX() const;

    /**
     * The Y offset of the paint device
     */
    qint32 getY() const;

    /**
     * Return the X offset of the paint device
     */
    void setX(qint32 x);

    /**
     * Return the Y offset of the paint device
     */
    void setY(qint32 y);


    /**
     * Return the number of bytes a pixel takes.
     */
    virtual qint32 pixelSize() const;

    /**
     * Return the number of channels a pixel takes
     */
    virtual qint32 channelCount() const;

    /**
     * Return the image that contains this paint device, or 0 if it is not
     * part of an image. This is the same as calling parentLayer()->image().
     */
    KisImageSP image() const;

    /**
     * Returns the KisLayer that contains this paint device, or 0 if this is not
     * part of a layer.
     */
    KisLayer *parentLayer() const;

    /**
     * Set the KisLayer that contains this paint device, or 0 if this is not
     * part of a layer.
     */
    void setParentLayer(KisLayer *parentLayer);


    /**
       Add the specified rect to the parent layer's set of dirty rects
       (if there is a parent layer)

       XXX: Refactor this so whenever we need to set a layer dirty, we
       do it on the layer instead of in this roundabout way. (BSAR)
     */
    virtual void setDirty(const QRect & rc);

    /**
       Add the specified region to the parent layer's dirty region
       (if there is a parent layer)

       XXX: Refactor this so whenever we need to set a layer dirty, we
       do it on the layer instead of in this roundabout way. (BSAR)
    */
    virtual void setDirty( const QRegion & region );

    /**
       Set the parent layer completely dirty, if this paint device has
       as parent layer.

       XXX: Refactor this so whenever we need to set a layer dirty, we
       do it on the layer instead of in this roundabout way. (BSAR)
     */
    virtual void setDirty();


    /**
     * Mirror the device along the X axis
     */
    void mirrorX();
    /**
     * Mirror the device along the Y axis
     */
    void mirrorY();

    KisMementoSP getMemento();
    void rollback(KisMementoSP memento);
    void rollforward(KisMementoSP memento);

    /**
     * Create an iterator over a rectangle section of a paint device, the path followed by
     * the iterator is not guaranteed, it is optimized for speed, which means that you shouldn't
     * use this type of iterator if you are combining two differents layers.
     * @param w width
     * @param h height
     * @return an iterator which points to the first pixel of an rectangle
     */
    KisRectIteratorPixel createRectIterator(qint32 left, qint32 top, qint32 w, qint32 h);
    /**
     * Create an iterator over a rectangle section of a paint device, the path followed by
     * the iterator is not guaranteed, it is optimized for speed, which means that you shouldn't
     * use this type of iterator if you are combining two differents layers.
     * @param w width
     * @param h height
     * @return an iterator which points to the first pixel of an rectangle, this iterator
     * does not allow to change the pixel values
     */
    KisRectConstIteratorPixel createRectConstIterator(qint32 left, qint32 top, qint32 w, qint32 h) const;

    /**
     * @return an iterator which points to the first pixel of a horizontal line, this iterator
     * does not allow to change the pixel values
     */
    KisHLineConstIteratorPixel createHLineConstIterator(qint32 x, qint32 y, qint32 w) const;
    /**
     * @return an iterator which points to the first pixel of a horizontal line
     */
    KisHLineIteratorPixel createHLineIterator(qint32 x, qint32 y, qint32 w);

    /**
     * This function return an iterator which points to the first pixel of a vertical line
     */
    KisVLineIteratorPixel createVLineIterator(qint32 x, qint32 y, qint32 h);

    /**
     * This function return an iterator which points to the first pixel of a vertical line
     */
    KisVLineConstIteratorPixel createVLineConstIterator(qint32 x, qint32 y, qint32 h) const;

    /**
     * This function creates a random accessor which allow to randomly access any pixels on
     * the paint device.
     * <b>Note:</b> random access is way slower than iterators, always use iterators whenever
     * you can
     */
    KisRandomAccessorPixel createRandomAccessor(Q_INT32 x, Q_INT32 y);

    /**
     * This function creates a random accessor which allow to randomly access any pixels on
     * the paint device.
     * <b>Note:</b> random access is way slower than iterators, always use iterators whenever
     * you can
     */
    KisRandomConstAccessorPixel createRandomConstAccessor(Q_INT32 x, Q_INT32 y) const;

    /**
     * This function create a random accessor which can easily access to sub pixel values.
     */
    KisRandomSubAccessorPixel createRandomSubAccessor() const;


    /**
     * @return the current selection or create one if this paintdevice hasn't got a selection yet. */
    KisSelectionSP selection();
    /** @return the current selection or create one if this paintdevice hasn't got a selection yet. */
    const KisSelectionSP selection() const;

    /** Adds the specified selection to the currently active selection for this paintdevice */
    void addSelection(KisSelectionSP selection);

    /** Subtracts the specified selection from the currently active selection for this paindevice */
    void subtractSelection(KisSelectionSP selection);

    /** Whether there is a valid selection for this paintdevice. */
    bool hasSelection() const;

   /** Whether the previous selection was deselected. */
    bool selectionDeselected();

    /** Deselect the selection for this paintdevice. */
    void deselect();

    /** Reinstates the old selection */
    void reselect();

    /** Clear the selected pixels from the paint device */
    void clearSelection();

    /**
     * Apply a mask to the image data, i.e. multiply each pixel's opacity by its
     * selectedness in the mask.
     */
    void applySelectionMask(KisSelectionSP mask);

    /**
     * Sets the selection of this paint device to the new selection,
     * returns the old selection, if there was an old selection,
     * otherwise 0
     */
    KisSelectionSP setSelection(KisSelectionSP selection);

    /**
     * Notify the owning image that the current selection has changed.
     */
    void emitSelectionChanged();


    KisUndoAdapter *undoAdapter() const;

    /**
     * Notify the owning image that the current selection has changed.
     *
     * @param r the area for which the selection has changed
     */
    void emitSelectionChanged(const QRect& r);

    /**
     * Return the exifInfo associated with this layer. If no exif infos are
     * available, the function will create it.
     */
    KisExifInfo* exifInfo();
    /**
     * This function return true if the layer has exif info associated with it.
     */
    bool hasExifInfo();
signals:
    void positionChanged(KisPaintDeviceSP device);
    void ioProgress(qint8 percentage);
    void profileChanged(KoColorProfile *  profile);
    void colorSpaceChanged(KoColorSpace *colorspace);

private slots:

    void runBackgroundFilters();

private:
    KisPaintDevice& operator=(const KisPaintDevice&);

protected:
    KisDataManagerSP m_datamanager;

private:

    class Private;
    Private * const m_d;

};

#endif // KIS_PAINT_DEVICE_IMPL_H_

