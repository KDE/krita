/*
 *  Copyright (c) 2005 Adrian Page <adrian@pagenet.plus.com>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#ifndef KIS_RGB_F16HALF_COLORSPACE_H_
#define KIS_RGB_F16HALF_COLORSPACE_H_

#include <qcolor.h>

#include <koffice_export.h>

#include "kis_global.h"
#include "kis_f16half_base_colorspace.h"
#include "kis_pixel.h"


class KRITATOOL_EXPORT KisRgbF16HalfColorSpace : public KisF16HalfBaseColorSpace {
public:
    KisRgbF16HalfColorSpace();
    virtual ~KisRgbF16HalfColorSpace();

public:
    void setPixel(Q_UINT8 *pixel, half red, half green, half blue, half alpha) const;
    void getPixel(const Q_UINT8 *pixel, half *red, half *green, half *blue, half *alpha) const;

    virtual void fromQColor(const QColor& c, Q_UINT8 *dst, KisProfile *  profile = 0);
    virtual void fromQColor(const QColor& c, Q_UINT8 opacity, Q_UINT8 *dst, KisProfile *  profile = 0);

    virtual void toQColor(const Q_UINT8 *src, QColor *c, KisProfile *  profile = 0);
    virtual void toQColor(const Q_UINT8 *src, QColor *c, Q_UINT8 *opacity, KisProfile *  profile = 0);

    //XXX: KisPixel(RO) does not work with this colourspace as it only handles 8-bit channels.
    virtual KisPixelRO toKisPixelRO(const Q_UINT8 *src, KisProfile *  profile = 0)
        { return KisPixelRO (src, src + PIXEL_ALPHA * sizeof(half), this, profile); }
    virtual KisPixel toKisPixel(Q_UINT8 *src, KisProfile *  profile = 0)
        { return KisPixel (src, src + PIXEL_ALPHA * sizeof(half), this, profile); }

    virtual Q_INT8 difference(const Q_UINT8 *src1, const Q_UINT8 *src2);
    virtual void mixColors(const Q_UINT8 **colors, const Q_UINT8 *weights, Q_UINT32 nColors, Q_UINT8 *dst) const;

    virtual QValueVector<KisChannelInfo *> channels() const;
    virtual bool hasAlpha() const;
    virtual Q_INT32 nChannels() const;
    virtual Q_INT32 nColorChannels() const;
    virtual Q_INT32 pixelSize() const;

    virtual QImage convertToQImage(const Q_UINT8 *data, Q_INT32 width, Q_INT32 height,
                       KisProfile *  srcProfile, KisProfile *  dstProfile,
                       Q_INT32 renderingIntent,
                       float exposure = 0.0f);

    virtual KisCompositeOpList userVisiblecompositeOps() const;


protected:

    virtual void bitBlt(Q_UINT8 *dst,
                Q_INT32 dstRowStride,
                const Q_UINT8 *src,
                Q_INT32 srcRowStride,
                const Q_UINT8 *srcAlphaMask,
                Q_INT32 maskRowStride,
                Q_UINT8 opacity,
                Q_INT32 rows,
                Q_INT32 cols,
                const KisCompositeOp& op);

    void compositeOver(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, half opacity);
    void compositeMultiply(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, half opacity);
    void compositeDivide(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, half opacity);
    void compositeScreen(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, half opacity);
    void compositeOverlay(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, half opacity);
    void compositeDodge(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, half opacity);
    void compositeBurn(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, half opacity);
    void compositeDarken(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, half opacity);
    void compositeLighten(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, half opacity);
    void compositeHue(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, half opacity);
    void compositeSaturation(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, half opacity);
    void compositeValue(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, half opacity);
    void compositeColor(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, half opacity);
    void compositeErase(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, half opacity);
    void compositeCopy(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, half opacity);

private:
    friend class KisRgbF16HalfColorSpaceTester;

    static const Q_UINT8 PIXEL_BLUE = 0;
    static const Q_UINT8 PIXEL_GREEN = 1;
    static const Q_UINT8 PIXEL_RED = 2;
    static const Q_UINT8 PIXEL_ALPHA = 3;

    struct Pixel {
        half blue;
        half green;
        half red;
        half alpha;
    };
};

#endif // KIS_RGB_F16HALF_COLORSPACE_H_

