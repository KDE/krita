/*
 *  Copyright (c) 2005 Boudewijn Rempt (boud@valdyas.org)
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
#ifndef KIS__COLORSPACE_XYZ_H_
#define KIS__COLORSPACE_XYZ_H_

#include <qcolor.h>

#include "kis_global.h"
#include "kis_integer_maths.h"
#include "kis_u16_base_colorspace.h"

namespace xyz {
    const Q_INT32 MAX_CHANNEL_XYZ = 3;
    const Q_INT32 MAX_CHANNEL_XYZA = 4;
}



class KisXyzColorSpace : public KisU16BaseColorSpace {

public:

    struct Pixel {
        Q_UINT16 X;
        Q_UINT16 Y;
        Q_UINT16 Z;
        Q_UINT16 alpha;
    };

public:
    KisXyzColorSpace(KisColorSpaceFactoryRegistry * parent,
                     KisProfile *p);
    virtual ~KisXyzColorSpace();

    virtual bool willDegrade(ColorSpaceIndependence independence)
        {
            if (independence == TO_RGBA8)
                return true;
            else
                return false;
        };
public:
    // Pixel manipulation
    virtual KisColorAdjustment *createBrightnessContrastAdjustment(Q_UINT16 *transferValues);
    virtual void applyAdjustment(const Q_UINT8 *src, Q_UINT8 *dst, KisColorAdjustment *, Q_INT32 nPixels);
    virtual void invertColor(Q_UINT8 * src, Q_INT32 nPixels);
    virtual void mixColors(const Q_UINT8 **colors, const Q_UINT8 *weights, Q_UINT32 nColors, Q_UINT8 *dst) const;
    virtual void convolveColors(Q_UINT8** colors, Q_INT32* kernelValues, KisChannelInfo::enumChannelFlags channelFlags, Q_UINT8 *dst, Q_INT32 factor, Q_INT32 offset, Q_INT32 nPixels) const;
    virtual void darken(const Q_UINT8 * src, Q_UINT8 * dst, Q_INT32 shade, bool compensate, double compensation, Q_INT32 nPixels) const;
    virtual Q_UINT8 intensity8(const Q_UINT8 * src) const;

    // Information about the colorstrategy
    virtual QValueVector<KisChannelInfo *> channels() const;
    virtual Q_UINT32 nChannels() const;
    virtual Q_UINT32 nColorChannels() const;
    virtual Q_UINT32 pixelSize() const;


    // Composition

    virtual void bitBlt(Q_UINT8 *dst,
                Q_INT32 dststride,
                const Q_UINT8 *src,
                Q_INT32 srcRowStride,
                const Q_UINT8 *srcAlphaMask,
                Q_INT32 maskRowStride,
                Q_UINT8 opacity,
                Q_INT32 rows,
                Q_INT32 cols,
                const KisCompositeOp& op);

    KisCompositeOpList userVisiblecompositeOps() const;

protected:
    void compositeOver(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, Q_UINT16 opacity);
    void compositeMultiply(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, Q_UINT16 opacity);
    void compositeDivide(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, Q_UINT16 opacity);
    void compositeScreen(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, Q_UINT16 opacity);
    void compositeOverlay(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, Q_UINT16 opacity);
    void compositeDodge(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, Q_UINT16 opacity);
    void compositeBurn(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, Q_UINT16 opacity);
    void compositeDarken(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, Q_UINT16 opacity);
    void compositeLighten(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, Q_UINT16 opacity);
    void compositeErase(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, Q_UINT16 opacity);

private:

    static const Q_UINT8 PIXEL_X = 0;
    static const Q_UINT8 PIXEL_Y = 1;
    static const Q_UINT8 PIXEL_Z = 2;
    static const Q_UINT8 PIXEL_ALPHA = 3;

    Q_UINT8 * m_qcolordata; // A small buffer for conversion from and to qcolor.

};

#endif // KIS__COLORSPACE_XYZ_H_
