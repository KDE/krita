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
#include "kis_abstract_colorspace.h"
#include "kis_pixel.h"
#include "kis_integer_maths.h"

class KisXyzColorSpace : public KisAbstractColorSpace {
public:

    static const Q_UINT16 U16_OPACITY_OPAQUE = UINT16_MAX;
    static const Q_UINT16 U16_OPACITY_TRANSPARENT = UINT16_MIN;

    struct Pixel {
        Q_UINT16 X;
        Q_UINT16 Y;
        Q_UINT16 Z;
        Q_UINT16 alpha;
    };

public:
    KisXyzColorSpace();
    virtual ~KisXyzColorSpace();

public:

    virtual void nativeColor(const QColor& c, Q_UINT8 *dst, KisProfileSP profile = 0);
    virtual void nativeColor(const QColor& c, QUANTUM opacity, Q_UINT8 *dst, KisProfileSP profile = 0);

    virtual void getAlpha(const Q_UINT8 *pixel, Q_UINT8 *alpha);
    virtual void setAlpha(Q_UINT8 * pixels, Q_UINT8 alpha, Q_INT32 nPixels);

    virtual void toQColor(const Q_UINT8 *src, QColor *c, KisProfileSP profile = 0);
    virtual void toQColor(const Q_UINT8 *src, QColor *c, QUANTUM *opacity, KisProfileSP profile = 0);

    //XXX: KisPixel(RO) does not work with this colourspace as it only handles 8-bit channels.
    virtual KisPixelRO toKisPixelRO(const Q_UINT8 *src, KisProfileSP profile = 0)
        { return KisPixelRO (src, src + PIXEL_ALPHA * sizeof(Q_UINT16), this, profile); }
    virtual KisPixel toKisPixel(Q_UINT8 *src, KisProfileSP profile = 0)
        { return KisPixel (src, src + PIXEL_ALPHA * sizeof(Q_UINT16), this, profile); }

    virtual Q_INT8 difference(const Q_UINT8 *src1, const Q_UINT8 *src2);
    virtual void mixColors(const Q_UINT8 **colors, const Q_UINT8 *weights, Q_UINT32 nColors, Q_UINT8 *dst) const;

    virtual vKisChannelInfoSP channels() const;
    virtual bool hasAlpha() const;
    virtual Q_INT32 nChannels() const;
    virtual Q_INT32 nColorChannels() const;
    virtual Q_INT32 pixelSize() const;

    virtual QString channelValueText(const Q_UINT8 *pixel, Q_UINT32 channelIndex) const;
    virtual QString normalisedChannelValueText(const Q_UINT8 *pixel, Q_UINT32 channelIndex) const;


    virtual QImage convertToQImage(const Q_UINT8 *data, Q_INT32 width, Q_INT32 height,
                       KisProfileSP srcProfile, KisProfileSP dstProfile,
                       Q_INT32 renderingIntent = INTENT_PERCEPTUAL,
                       float exposure = 0.0f);

    virtual void adjustBrightnessContrast(const Q_UINT8 *src, Q_UINT8 *dst, Q_INT8 brightness, Q_INT8 contrast, Q_INT32 nPixels) const;

    virtual void bitBlt(Q_UINT8 *dst,
                Q_INT32 dststride,
                const Q_UINT8 *src,
                Q_INT32 srcRowStride,
                const Q_UINT8 *srcAlphaMask,
                Q_INT32 maskRowStride,
                QUANTUM opacity,
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
    void compositeCopy(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, Q_UINT16 opacity);

private:

    vKisChannelInfoSP m_channels;

    cmsHTRANSFORM m_defaultToRGB;
    cmsHTRANSFORM m_defaultFromRGB;

    static const Q_UINT8 PIXEL_X = 0;
    static const Q_UINT8 PIXEL_Y = 2;
    static const Q_UINT8 PIXEL_Z = 3;
    static const Q_UINT8 PIXEL_ALPHA = 4;

    Q_UINT8 * m_qcolordata; // A small buffer for conversion from and to qcolor.

};

#endif // KIS__COLORSPACE_XYZ_H_
