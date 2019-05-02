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
#include "kis_shared.h"
#include "kis_default_bounds_base.h"

#include <kritaimage_export.h>

class KUndo2Command;
class QRect;
class QImage;
class QPoint;
class QString;
class QColor;
class QIODevice;

class KoColor;
class KoColorSpace;
class KoColorProfile;

class KisDataManager;
class KisPaintDeviceWriter;
class KisKeyframe;
class KisRasterKeyframeChannel;

class KisPaintDeviceFramesInterface;

typedef KisSharedPtr<KisDataManager> KisDataManagerSP;

namespace KritaUtils {
enum DeviceCopyMode {
    CopySnapshot = 0,
    CopyAllFrames
};
}


/**
 * A paint device contains the actual pixel data and offers methods
 * to read and write pixels. A paint device has an integer x, y position
 * (it is not positioned on the image with sub-pixel accuracy).
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
    explicit KisPaintDevice(const KoColorSpace * colorSpace, const QString& name = QString());

    /**
     * Create a new paint device with the specified colorspace. The
     * parent node will be notified of changes to this paint device.
     *
     * @param parent the node that contains this paint device
     * @param colorSpace the colorspace of this paint device
     * @param defaultBounds boundaries of the device in case it is empty
     * @param name for debugging purposes
     */
    KisPaintDevice(KisNodeWSP parent, const KoColorSpace * colorSpace, KisDefaultBoundsBaseSP defaultBounds = KisDefaultBoundsBaseSP(), const QString& name = QString());

    /**
     * Creates a copy of this device.
     *
     * If \p copyMode is CopySnapshot, the newly created device clones the
     * current frame of \p rhs only (default and efficient
     * behavior). If \p copyFrames is CopyAllFrames, the new device is a deep
     * copy of the source with all the frames included.
     */
    KisPaintDevice(const KisPaintDevice& rhs, KritaUtils::DeviceCopyMode copyMode = KritaUtils::CopySnapshot, KisNode *newParentNode = 0);
    ~KisPaintDevice() override;

    void makeFullCopyFrom(const KisPaintDevice& rhs, KritaUtils::DeviceCopyMode copyMode = KritaUtils::CopySnapshot, KisNode *newParentNode = 0);

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
    bool write(KisPaintDeviceWriter &store);

    /**
     * Fill this paint device with the pixels from the specified file store.
     */
    bool read(QIODevice *stream);

public:

    /**
     * set the parent node of the paint device
     */
    void setParentNode(KisNodeWSP parent);

    /**
     * set the default bounds for the paint device when
     * the default pixel is not completely transparent
     */
    void setDefaultBounds(KisDefaultBoundsBaseSP bounds);

    /**
     * the default bounds rect of the paint device
     */
    KisDefaultBoundsBaseSP defaultBounds() const;

    /**
     * Moves the device to these new coordinates (no incremental move)
     */
    void moveTo(qint32 x, qint32 y);

    /**
     * Convenience method for the above.
     */
    virtual void moveTo(const QPoint& pt);

    /**
     * Return an X,Y offset of the device in a convenient form
     */
    QPoint offset() const;

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
     *
     * If default pixel is not transparent, then the actual extent
     * rect is united with the defaultBounds()->bounds() value
     * (the size of the image, usually).
     */
    QRect extent() const;

    /// Convenience method for the above
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
     * <li>if default pixel is not transparent, then the union
     *     (defaultBounds()->bounds() | nonDefaultPixelArea()) is
     *     returned
     * </ul>
     * \see calculateExactBounds()
     */
    QRect exactBounds() const;

    /**
     * Relaxed version of the exactBounds() that can be used in tight
     * loops.  If the exact bounds value is present in the paint
     * device cache, returns this value.  If the cache is invalidated,
     * returns extent() and tries to recalculate the exact bounds not
     * faster than once in 1000 ms.
     */
    QRect exactBoundsAmortized() const;

    /**
     * Returns exact rectangle of the paint device that contains
     * non-default pixels. For paint devices with fully transparent
     * default pixel is equivalent to exactBounds().
     *
     * nonDefaultPixelArea() follows these rules:
     *
     * <ul>
     * <li>if default pixel is transparent, then exact bounds
     *     of actual pixel data are returned. The same as exactBounds()
     * <li>if default pixel is not transparent, then calculates the
     *     rectangle of non-default pixels. May be smaller or greater
     *     than image bounds
     * </ul>
     * \see calculateExactBounds()
     */
    QRect nonDefaultPixelArea() const;


    /**
     * Returns a rough approximation of region covered by device.
     * For tiled data manager, it region will consist of a number
     * of rects each corresponding to a tile.
     */
    QRegion region() const;

    /**
     * The slow version of region() that searches for exact bounds of
     * each rectangle in the region
     */
    QRegion regionExact() const;

    /**
     * Cut the paint device down to the specified rect. If the crop
     * area is bigger than the paint device, nothing will happen.
     */
    void crop(qint32 x, qint32 y, qint32 w, qint32 h);

    /// Convenience method for the above
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
     * Frees the memory occupied by the pixels containing default
     * values. The extents() and exactBounds() of the image will
     * probably also shrink
     */
    void purgeDefaultPixels();

    /**
     * Sets the default pixel. New data will be initialised with this pixel. The pixel is copied: the
     * caller still owns the pointer and needs to delete it to avoid memory leaks.
     * If frame ID is given, set default pixel for that frame. Otherwise use active frame.
     */
    void setDefaultPixel(const KoColor &defPixel);

    /**
     * Get a pointer to the default pixel.
     * If the frame parameter is given, get the default pixel of
     * specified frame. Otherwise use currently active frame.
     */
    KoColor defaultPixel() const;

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
     * Prepares the device for fastBitBlt operation. It clears
     * the device, switches x,y shifts and colorspace if needed.
     * After this call fastBitBltPossible will return true.
     * May be used for initialization of temporary devices.
     */
    void prepareClone(KisPaintDeviceSP src);

    /**
     * Make this device to become a clone of \a src. It will have the same
     * x,y shifts, colorspace and will share pixels inside \a rect.
     * After calling this function:
     * (this->extent() >= this->exactBounds() == rect).
     *
     * Rule of thumb:
     *
     * "Use makeCloneFrom() or makeCloneFromRough() if and only if you
     * are the only owner of the destination paint device and you are
     * 100% sure no other thread has access to it"
     */
    void makeCloneFrom(KisPaintDeviceSP src, const QRect &rect);

    /**
     * Make this device to become a clone of \a src. It will have the same
     * x,y shifts, colorspace and will share pixels inside \a rect.
     * Be careful, this function will copy *at least* \a rect
     * of pixels. Actual copy area will be a bigger - it will
     * be aligned by tiles borders. So after calling this function:
     * (this->extent() == this->exactBounds() >= rect).
     *
     * Rule of thumb:
     *
     * "Use makeCloneFrom() or makeCloneFromRough() if and only if you
     * are the only owner of the destination paint device and you are
     * 100% sure no other thread has access to it"
     */
    void makeCloneFromRough(KisPaintDeviceSP src, const QRect &minimalRect);


protected:
    friend class KisPaintDeviceTest;
    friend class DataReaderThread;

    /**
     * Checks whether a src paint device can be used as source
     * of fast bitBlt operation. The result of the check may
     * depend on whether color spaces coincide, whether there is
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
    void readBytes(quint8 * data, const QRect &rect) const;

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
    QVector<quint8*> readPlanarBytes(qint32 x, qint32 y, qint32 w, qint32 h) const;

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
     */
    void convertTo(const KoColorSpace * dstColorSpace,
                   KoColorConversionTransformation::Intent renderingIntent = KoColorConversionTransformation::internalRenderingIntent(),
                   KoColorConversionTransformation::ConversionFlags conversionFlags = KoColorConversionTransformation::internalConversionFlags(),
                   KUndo2Command *parentCommand = 0);

    /**
     * Changes the profile of the colorspace of this paint device to the given
     * profile. If the given profile is 0, nothing happens.
     */
    bool setProfile(const KoColorProfile * profile);

    /**
     * Fill this paint device with the data from image; starting at (offsetX, offsetY)
     * @param image the image
     * @param profile name of the RGB profile to interpret the image as. 0 is interpreted as sRGB
     * @param offsetX x offset
     * @param offsetY y offset
     */
    void convertFromQImage(const QImage& image, const KoColorProfile *profile, qint32 offsetX = 0, qint32 offsetY = 0);

    /**
     * Create an RGBA QImage from a rectangle in the paint device.
     *
     * @param dstProfile RGB profile to use in conversion. May be 0, in which
     * case it's up to the color strategy to choose a profile (most
     * like sRGB).
     * @param x Left coordinate of the rectangle
     * @param y Top coordinate of the rectangle
     * @param w Width of the rectangle in pixels
     * @param h Height of the rectangle in pixels
     * @param renderingIntent Rendering intent
     * @param conversionFlags Conversion flags
     */
    QImage convertToQImage(const KoColorProfile *dstProfile, qint32 x, qint32 y, qint32 w, qint32 h,
                           KoColorConversionTransformation::Intent renderingIntent = KoColorConversionTransformation::internalRenderingIntent(),
                           KoColorConversionTransformation::ConversionFlags conversionFlags = KoColorConversionTransformation::internalConversionFlags()) const;

    /**
     * Overridden method for convenience
     */
    QImage convertToQImage(const KoColorProfile *dstProfile,
                           const QRect &rc,
                           KoColorConversionTransformation::Intent renderingIntent = KoColorConversionTransformation::internalRenderingIntent(),
                           KoColorConversionTransformation::ConversionFlags conversionFlags = KoColorConversionTransformation::internalConversionFlags()) const;

    /**
     * Create an RGBA QImage from a rectangle in the paint device. The
     * rectangle is defined by the parent image's bounds.
     *
     * @param dstProfile RGB profile to use in conversion. May be 0, in which
     * case it's up to the color strategy to choose a profile (most
     * like sRGB).
     * @param renderingIntent Rendering intent
     * @param conversionFlags Conversion flags
     */
    QImage convertToQImage(const KoColorProfile *  dstProfile,
                           KoColorConversionTransformation::Intent renderingIntent = KoColorConversionTransformation::internalRenderingIntent(),
                           KoColorConversionTransformation::ConversionFlags conversionFlags = KoColorConversionTransformation::internalConversionFlags()) const;

    /**
     * Creates a paint device thumbnail of the paint device, retaining
     * the aspect ratio. The width and height of the returned device
     * won't exceed \p maxw and \p maxw, but they may be smaller.
     *
     * @param w maximum width
     * @param h maximum height
     * @param rect only this rect will be used for the thumbnail
     * @param outputRect output rectangle
     *
     */
    KisPaintDeviceSP createThumbnailDevice(qint32 w, qint32 h, QRect rect = QRect(), QRect outputRect = QRect()) const;
    KisPaintDeviceSP createThumbnailDeviceOversampled(qint32 w, qint32 h, qreal oversample, QRect rect = QRect(),  QRect outputRect = QRect()) const;

    /**
     * Creates a thumbnail of the paint device, retaining the aspect ratio.
     * The width and height of the returned QImage won't exceed \p maxw and \p maxw, but they may be smaller.
     * The colors are not corrected for display!
     *
     * @param maxw: maximum width
     * @param maxh: maximum height
     * @param rect: only this rect will be used for the thumbnail
     * @param oversample: ratio used for antialiasing
     * @param renderingIntent Rendering intent
     * @param conversionFlags Conversion flags
     */
    QImage createThumbnail(qint32 maxw, qint32 maxh, QRect rect, qreal oversample = 1,
                           KoColorConversionTransformation::Intent renderingIntent = KoColorConversionTransformation::internalRenderingIntent(),
                           KoColorConversionTransformation::ConversionFlags conversionFlags = KoColorConversionTransformation::internalConversionFlags());

    /**
     * Cached version of createThumbnail(qint32 maxw, qint32 maxh, const KisSelection *selection, QRect rect)
     */
    QImage createThumbnail(qint32 maxw, qint32 maxh, qreal oversample = 1,
                           KoColorConversionTransformation::Intent renderingIntent = KoColorConversionTransformation::internalRenderingIntent(),
                           KoColorConversionTransformation::ConversionFlags conversionFlags = KoColorConversionTransformation::internalConversionFlags());

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

    /// Convenience method for the above
    bool setPixel(qint32 x, qint32 y, const KoColor& kc);

    /**
     * @return the colorspace of the pixels in this paint device
     */
    const KoColorSpace* colorSpace() const;

    /**
     * There is quite a common technique in Krita. It is used in
     * cases, when we want to paint something over a paint device
     * using the composition, opacity or selection. E.g. painting a
     * dab in a paint op, filling the selection in the Fill Tool.
     * Such work is usually done in the following way:
     *
     * 1) Create a paint device
     *
     * 2) Fill it with the desired color or data
     *
     * 3) Create a KisPainter and set all the properties of the
     *    transaction: selection, compositeOp, opacity and etc.
     *
     * 4) Paint a newly created paint device over the destination
     *    device.
     *
     * The following two methods (createCompositionSourceDevice() or
     * createCompositionSourceDeviceFixed())should be used for the
     * accomplishing the step 1). The point is that the desired color
     * space of the temporary device may not coincide with the color
     * space of the destination. That is the case, for example, for
     * the alpha8() colorspace used in the selections. So for such
     * devices the temporary target would have a different (grayscale)
     * color space.
     *
     * So there are two rules of thumb:
     *
     * 1) If you need a temporary device which you are going to fill
     *    with some data and then paint over the paint device, create
     *    it with either createCompositionSourceDevice() or
     *    createCompositionSourceDeviceFixed().
     *
     * 2) Do *not* expect that the color spaces of the destination and
     *    the temporary device would coincide. If you need to copy a
     *    single pixel from one device to another, you can use
     *    KisCrossDeviceColorPicker class, that will handle all the
     *    necessary conversions for you.
     *
     * \see createCompositionSourceDeviceFixed()
     * \see compositionSourceColorSpace()
     * \see KisCrossDeviceColorPicker
     * \see KisCrossDeviceColorPickerInt
     */
    KisPaintDeviceSP createCompositionSourceDevice() const;

    /**
     * The same as createCompositionSourceDevice(), but initializes the
     * newly created device with the content of \p cloneSource
     *
     * \see createCompositionSourceDevice()
     */
    KisPaintDeviceSP createCompositionSourceDevice(KisPaintDeviceSP cloneSource) const;

    /**
     * The same as createCompositionSourceDevice(), but initializes
     * the newly created device with the *rough* \p roughRect of
     * \p cloneSource.
     *
     * "Rough rect" means that it may copy a bit more than
     * requested. It is expected that the caller will not use the area
     * outside \p roughRect.
     *
     * \see createCompositionSourceDevice()
     */
    KisPaintDeviceSP createCompositionSourceDevice(KisPaintDeviceSP cloneSource, const QRect roughRect) const;

    /**
     * This is a convenience method for createCompositionSourceDevice()
     *
     * \see createCompositionSourceDevice()
     */
    KisFixedPaintDeviceSP createCompositionSourceDeviceFixed() const;

    /**
     * This is a lowlevel method for the principle used in
     * createCompositionSourceDevice(). In most of the cases the paint
     * device creation methods should be used instead of this function.
     *
     * \see createCompositionSourceDevice()
     * \see createCompositionSourceDeviceFixed()
     */
    virtual const KoColorSpace* compositionSourceColorSpace() const;

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
    quint32 pixelSize() const;

    /**
     * Return the number of channels a pixel takes
     */
    quint32 channelCount() const;

    /**
     * Create a keyframe channel for the content on this device.
     * @param id identifier for the channel
     * @return keyframe channel or 0 if there is not one
     */
    KisRasterKeyframeChannel *createKeyframeChannel(const KoID &id);

    KisRasterKeyframeChannel* keyframeChannel() const;

    /**
     * An interface to modify/load/save frames stored inside this device
     */
    KisPaintDeviceFramesInterface* framesInterface();

    /**
     * @brief debugPaintDevice save the current paint device to a numbered PNG image
     * @param basename the basename for the file.
     */
    void debugPaintDevice(const QString &basename) const;

public:

    /**
     * Add the specified rect to the parent layer's set of dirty rects
     * (if there is a parent layer)
     */
    void setDirty(const QRect & rc);

    /**
     *  Add the specified region to the parent layer's dirty region
     *  (if there is a parent layer)
     */
    void setDirty(const QRegion & region);

    /**
     *  Set the parent layer completely dirty, if this paint device has
     *  as parent layer.
     */
    void setDirty();

    void setDirty(const QVector<QRect> rects);

    /**
     * Called by KisTransactionData when it thinks current time should
     * be changed. And the requests is forwarded to the image if
     * needed.
     */
    void requestTimeSwitch(int time);

    /**
     * \return a sequence number corresponding to the current paint
     *         device state. Every time the paint device is changed,
     *         the sequence number is increased
     */
    int sequenceNumber() const;


    void estimateMemoryStats(qint64 &imageData, qint64 &temporaryData, qint64 &lodData) const;

public:

    KisHLineIteratorSP createHLineIteratorNG(qint32 x, qint32 y, qint32 w);
    KisHLineConstIteratorSP createHLineConstIteratorNG(qint32 x, qint32 y, qint32 w) const;

    KisVLineIteratorSP createVLineIteratorNG(qint32 x, qint32 y, qint32 h);
    KisVLineConstIteratorSP createVLineConstIteratorNG(qint32 x, qint32 y, qint32 h) const;

    KisRandomAccessorSP createRandomAccessorNG(qint32 x, qint32 y);
    KisRandomConstAccessorSP createRandomConstAccessorNG(qint32 x, qint32 y) const;

    /**
     * Create an iterator that will "artificially" extend the paint device with the
     * value of the border when trying to access values outside the range of data.
     *
     * @param x x of top left corner
     * @param y y of top left corner
     * @param w width of the border
     * @param _dataWidth indicates the rectangle that truly contains data
     */
    KisRepeatHLineConstIteratorSP createRepeatHLineConstIterator(qint32 x, qint32 y, qint32 w, const QRect& _dataWidth) const;
    /**
     * Create an iterator that will "artificially" extend the paint device with the
     * value of the border when trying to access values outside the range of data.
     *
     * @param x x of top left corner
     * @param y y of top left corner
     * @param h height of the border
     * @param _dataWidth indicates the rectangle that truly contains data
     */
    KisRepeatVLineConstIteratorSP createRepeatVLineConstIterator(qint32 x, qint32 y, qint32 h, const QRect& _dataWidth) const;

    /**
     * This function create a random accessor which can easily access to sub pixel values.
     */
    KisRandomSubAccessorSP createRandomSubAccessor() const;


    /** Clear the selected pixels from the paint device */
    void clearSelection(KisSelectionSP selection);

Q_SIGNALS:

    void profileChanged(const KoColorProfile *  profile);
    void colorSpaceChanged(const KoColorSpace *colorspace);

public:
    friend class PaintDeviceCache;

    /**
     * Caclculates exact bounds of the device. Used internally
     * by a transparent caching system. The solution is very slow
     * because it does a linear scanline search. So the complexity
     * is n*n at worst.
     *
     * \see exactBounds(), nonDefaultPixelArea()
     */
    QRect calculateExactBounds(bool nonDefaultOnly) const;

public:
    struct MemoryReleaseObject : public QObject {
        ~MemoryReleaseObject() override;
    };

    static MemoryReleaseObject* createMemoryReleaseObject();

public:
    struct LodDataStruct {
        virtual ~LodDataStruct();
    };

    QRegion regionForLodSyncing() const;
    LodDataStruct* createLodDataStruct(int lod);
    void updateLodDataStruct(LodDataStruct *dst, const QRect &srcRect);
    void uploadLodDataStruct(LodDataStruct *dst);

    void generateLodCloneDevice(KisPaintDeviceSP dst, const QRect &originalRect, int lod);

    void setProjectionDevice(bool value);
    void tesingFetchLodDevice(KisPaintDeviceSP targetDevice);

private:
    KisPaintDevice& operator=(const KisPaintDevice&);
    void init(const KoColorSpace *colorSpace,
              KisDefaultBoundsBaseSP defaultBounds,
              KisNodeWSP parent, const QString& name);

    // Only KisPainter is allowed to have access to these low-level methods
    friend class KisPainter;

    /**
     * Return a vector with in order the size in bytes of the channels
     * in the colorspace of this paint device.
     */
    QVector<qint32> channelSizes() const;

    void emitColorSpaceChanged();
    void emitProfileChanged();

private:
    friend class KisPaintDeviceFramesInterface;

protected:
    friend class KisSelectionTest;
    KisNodeWSP parentNode() const;

private:
    struct Private;
    Private * const m_d;
};

#endif // KIS_PAINT_DEVICE_IMPL_H_
