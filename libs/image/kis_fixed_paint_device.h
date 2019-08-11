/*
 *  Copyright (c) 2009 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_FIXED_PAINT_DEVICE_H
#define KIS_FIXED_PAINT_DEVICE_H

#include <kritaimage_export.h>
#include <KoColorSpace.h>
#include "kis_shared.h"
#include <kis_shared_ptr.h>

#include <QRect>
#include <QImage>
#include "KisOptimizedByteArray.h"

class KoColor;


/**
 * A fixed paint device is a simple paint device that consists of an array
 * of bytes and a rectangle. It cannot grow, it cannot shrink, all you can
 * do is fill the paint device with the right bytes and use it as an argument
 * to KisPainter or use the bytes as an argument to KoColorSpace functions.
 */
class KRITAIMAGE_EXPORT KisFixedPaintDevice : public KisShared
{

public:

    KisFixedPaintDevice(const KoColorSpace* colorSpace,
                        KisOptimizedByteArray::MemoryAllocatorSP allocator = KisOptimizedByteArray::MemoryAllocatorSP());
    virtual ~KisFixedPaintDevice();

    /**
     * Deep copy the fixed paint device, including the data.
     */
    KisFixedPaintDevice(const KisFixedPaintDevice& rhs);

    /**
     * Deep copy the fixed paint device, including the data.
     */
    KisFixedPaintDevice& operator=(const KisFixedPaintDevice& rhs);

    /**
     * setRect sets the rect of the fixed paint device to rect.
     * This will _not_ create the associated data area.
     *
     * @param rc the bounds in pixels. The x,y of the rect represent the origin
     * of the fixed paint device.
     */
    void setRect(const QRect& rc);

    /**
     * @return the rect that the data represents
     */
    QRect bounds() const;

    /**
     * @return the amount of allocated pixels (you can fake the size with setRect/bounds)
     * It is useful to know the accumulated memory size in pixels (not in bytes) for optimizations to avoid re-allocation.
     */
    int allocatedPixels() const;


    /**
     * @return the pixelSize associated with this fixed paint device.
     */
    quint32 pixelSize() const;

    const KoColorSpace* colorSpace() const {
        return m_colorSpace;
    }

    /**
     * initializes the paint device.
     *
     * @param defaultValue the default byte with which all pixels will be filled.
     * @return false if the allocation failed.
     */
    bool initialize(quint8 defaultValue = 0);

    /**
     * Changed the size of the internal buffer to accommodate the exact number of bytes
     * needed to store area bounds(). The allocated data is *not* initialized!
     */
    void reallocateBufferWithoutInitialization();

    /**
     * If the size of the internal buffer is smaller than the one needed to accommodate
     * bounds(), resize the buffer. Otherwise, do nothing. The allocated data is neither
     * copying or initialized!
     */
    void lazyGrowBufferWithoutInitialization();

    /**
     * @return a pointer to the beginning of the data associated with this fixed paint device.
     */
    quint8* data();

    const quint8* constData() const;

    quint8* data() const;

    /**
     * Read the bytes representing the rectangle described by x, y, w, h into
     * data. If data is not big enough, Krita will gladly overwrite the rest
     * of your precious memory.
     *
     * Since this is a copy, you need to make sure you have enough memory.
     *
     * The reading is done only if the rectangular area x,y,w,h is inside the bounds of the device
     * and the device is not empty
     */
    void readBytes(quint8 * dstData, qint32 x, qint32 y, qint32 w, qint32 h) const;

    /**
     *   Converts the paint device to a different colorspace
     */
    void convertTo(const KoColorSpace * dstColorSpace = 0,
                   KoColorConversionTransformation::Intent renderingIntent = KoColorConversionTransformation::internalRenderingIntent(),
                   KoColorConversionTransformation::ConversionFlags conversionFlags = KoColorConversionTransformation::internalConversionFlags());

    /**
     * Set color profile for the device without converting actual pixel data
     */
    void setProfile(const KoColorProfile *profile);

    /**
     * Fill this paint device with the data from image
     *
     * @param image the image
     * @param srcProfileName name of the RGB profile to interpret the image as. 0 is interpreted as sRGB
     */
    virtual void convertFromQImage(const QImage& image, const QString &srcProfileName);

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
    virtual QImage convertToQImage(const KoColorProfile *dstProfile, qint32 x, qint32 y, qint32 w, qint32 h,
                                   KoColorConversionTransformation::Intent renderingIntent = KoColorConversionTransformation::internalRenderingIntent(),
                                   KoColorConversionTransformation::ConversionFlags conversionFlags = KoColorConversionTransformation::internalConversionFlags()) const;

    /**
     * Create an RGBA QImage from a rectangle in the paint device. The
     * rectangle is defined by the parent image's bounds.
     *
     * @param dstProfile RGB profile to use in conversion. May be 0, in which
     * case it's up to the color strategy to choose a profile (most
     * like sRGB).
     * @param renderingIntent The rendering intent of conversion.
     * @param conversionFlags The conversion flags.
     */
    virtual QImage convertToQImage(const KoColorProfile *dstProfile,
                                   KoColorConversionTransformation::Intent renderingIntent = KoColorConversionTransformation::internalRenderingIntent(),
                                   KoColorConversionTransformation::ConversionFlags conversionFlags = KoColorConversionTransformation::internalConversionFlags()) const;

    /**
     * Clear the given rectangle to transparent black.
     *
     * XXX: this will not (yet) expand the paint device to contain the specified rect
     * but if the paintdevice has not been initialized, it will be.
     */
    void clear(const QRect & rc);

    /**
     * Fill the given rectangle with the given pixel. This does not take the
     * selection into account.
     *
     * XXX: this will not (yet) expand the paint device to contain the specified rect
     * but if the paintdevice has not been initialized, it will be.
     */
    void fill(qint32 x, qint32 y, qint32 w, qint32 h, const quint8 *fillPixel);

    void fill(const QRect &rc, const KoColor &color);


    /**
     * Mirrors the device.
     */
    void mirror(bool horizontal, bool vertical);

private:

    const KoColorSpace* m_colorSpace;
    QRect m_bounds;
    KisOptimizedByteArray m_data;
};

#endif
