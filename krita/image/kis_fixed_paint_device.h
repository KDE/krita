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

#include <krita_export.h>
#include <KoColorSpace.h>
#include "kis_global.h"
#include "kis_shared.h"
#include <kis_shared_ptr.h>

#include <QRect>
#include <QImage>
#include <QVector>

/**
 * A fixed paint device is a simple paint device that consists of an array
 * of bytes and a rectangle. It cannot grow, it cannot shrink, all you can
 * do is fill the paint device with the right bytes and use it as an argument
 * to KisPainter or use the bytes as an argument to KoColorSpace functions.
 */
class KRITAIMAGE_EXPORT KisFixedPaintDevice : public KisShared
{

public:

    KisFixedPaintDevice(const KoColorSpace* colorSpace);
    virtual ~KisFixedPaintDevice();

    /**
     * Deep copy the the fixed paint device, including the data.
     */
    KisFixedPaintDevice(const KisFixedPaintDevice& rhs);

    /**
     * setRect sets the rect of the fixed paint device to rect.
     * This will _not_ create the associated data area.
     *
     * @rect the bounds in pixels. The x,y of the rect represent the origin
     * of the fixed paint device.
     */
    void setRect(const QRect& rc);

    /**
     * @return the rect that the data represents
     */
    QRect bounds() const;

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
     * @return a pointer to the beginning of the data associated with this fixed paint device.
     */
    quint8* data();

    quint8* data() const;

    /**
     *   Converts the paint device to a different colorspace
     */
    void convertTo(const KoColorSpace * dstColorSpace, KoColorConversionTransformation::Intent renderingIntent = KoColorConversionTransformation::IntentPerceptual);

    /**
     * Fill this paint device with the data from img
     *
     * @param srcProfileName name of the RGB profile to interpret the img as. "" is interpreted as sRGB
     */
    virtual void convertFromQImage(const QImage& img, const QString &srcProfileName);

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
    virtual QImage convertToQImage(const KoColorProfile *  dstProfile, qint32 x, qint32 y, qint32 w, qint32 h);

    /**
     * Create an RGBA QImage from a rectangle in the paint device. The
     * rectangle is defined by the parent image's bounds.
     *
     * @param dstProfile RGB profile to use in conversion. May be 0, in which
     * case it's up to the color strategy to choose a profile (most
     * like sRGB).
     */
    virtual QImage convertToQImage(const KoColorProfile *  dstProfile = 0);

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

private:

    KisFixedPaintDevice& operator=(const KisFixedPaintDevice& rhs);

    const KoColorSpace* m_colorSpace;
    QRect m_bounds;
    QVector<quint8> m_data;

};

#endif
