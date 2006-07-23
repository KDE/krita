/*
 *  Copyright (c) 2002 Patrick Julien  <freak@codepimps.org>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef KIS_STRATEGY_COLORSPACE_RGB_F32_H_
#define KIS_STRATEGY_COLORSPACE_RGB_F32_H_

#include <QColor>

#include <klocale.h>

#include <krita_export.h>

#include "KoF32HalfColorSpaceTrait.h"
#include "KoLcmsColorSpaceTrait.h"

class KoColorSpaceRegistry;

class KRITACOLOR_EXPORT KisRgbF32ColorSpace : public KoF32ColorSpaceTrait, public KoLcmsColorSpaceTrait {
public:
    KisRgbF32ColorSpace(KoColorSpaceRegistry * parent, KoColorProfile *p);
    virtual ~KisRgbF32ColorSpace();

    virtual bool willDegrade(ColorSpaceIndependence independence)
        {
            if (independence == TO_RGBA8 || independence == TO_LAB16)
                return true;
            else
                return false;
        };



public:
    void setPixel(quint8 *pixel, float red, float green, float blue, float alpha) const;
    void getPixel(const quint8 *pixel, float *red, float *green, float *blue, float *alpha) const;

    virtual bool hasHighDynamicRange() const { return true; }

    virtual void fromQColor(const QColor& c, quint8 *dst, KoColorProfile * profile = 0);
    virtual void fromQColor(const QColor& c, quint8 opacity, quint8 *dst, KoColorProfile * profile = 0);

    virtual void toQColor(const quint8 *src, QColor *c, KoColorProfile * profile = 0);
    virtual void toQColor(const quint8 *src, QColor *c, quint8 *opacity, KoColorProfile * profile = 0);

    virtual quint8 difference(const quint8 *src1, const quint8 *src2);
    virtual void mixColors(const quint8 **colors, const quint8 *weights, quint32 nColors, quint8 *dst) const;
    virtual void invertColor(quint8 * src, qint32 nPixels);
    virtual void convolveColors(quint8** colors, qint32 * kernelValues, KoChannelInfo::enumChannelFlags channelFlags, quint8 *dst, qint32 factor, qint32 offset, qint32 nColors) const;
    virtual quint8 intensity8(const quint8 * src) const;

    virtual Q3ValueVector<KoChannelInfo *> channels() const;
    virtual quint32 nChannels() const;
    virtual quint32 nColorChannels() const;
    virtual quint32 pixelSize() const;


    virtual QImage convertToQImage(const quint8 *data, qint32 width, qint32 height,
                       KoColorProfile *  dstProfile,
                       qint32 renderingIntent,
                       float exposure = 0.0f);

    virtual KoCompositeOpList userVisiblecompositeOps() const;


protected:

    virtual void bitBlt(quint8 *dst,
                qint32 dstRowStride,
                const quint8 *src,
                qint32 srcRowStride,
                const quint8 *srcAlphaMask,
                qint32 maskRowStride,
                quint8 opacity,
                qint32 rows,
                qint32 cols,
                const KoCompositeOp& op);

    void compositeOver(quint8 *dst, qint32 dstRowStride, const quint8 *src, qint32 srcRowStride, const quint8 *mask, qint32 maskRowStride, qint32 rows, qint32 columns, float opacity);
    void compositeMultiply(quint8 *dst, qint32 dstRowStride, const quint8 *src, qint32 srcRowStride, const quint8 *mask, qint32 maskRowStride, qint32 rows, qint32 columns, float opacity);
    void compositeDivide(quint8 *dst, qint32 dstRowStride, const quint8 *src, qint32 srcRowStride, const quint8 *mask, qint32 maskRowStride, qint32 rows, qint32 columns, float opacity);
    void compositeScreen(quint8 *dst, qint32 dstRowStride, const quint8 *src, qint32 srcRowStride, const quint8 *mask, qint32 maskRowStride, qint32 rows, qint32 columns, float opacity);
    void compositeOverlay(quint8 *dst, qint32 dstRowStride, const quint8 *src, qint32 srcRowStride, const quint8 *mask, qint32 maskRowStride, qint32 rows, qint32 columns, float opacity);
    void compositeDodge(quint8 *dst, qint32 dstRowStride, const quint8 *src, qint32 srcRowStride, const quint8 *mask, qint32 maskRowStride, qint32 rows, qint32 columns, float opacity);
    void compositeBurn(quint8 *dst, qint32 dstRowStride, const quint8 *src, qint32 srcRowStride, const quint8 *mask, qint32 maskRowStride, qint32 rows, qint32 columns, float opacity);
    void compositeDarken(quint8 *dst, qint32 dstRowStride, const quint8 *src, qint32 srcRowStride, const quint8 *mask, qint32 maskRowStride, qint32 rows, qint32 columns, float opacity);
    void compositeLighten(quint8 *dst, qint32 dstRowStride, const quint8 *src, qint32 srcRowStride, const quint8 *mask, qint32 maskRowStride, qint32 rows, qint32 columns, float opacity);
    void compositeHue(quint8 *dst, qint32 dstRowStride, const quint8 *src, qint32 srcRowStride, const quint8 *mask, qint32 maskRowStride, qint32 rows, qint32 columns, float opacity);
    void compositeSaturation(quint8 *dst, qint32 dstRowStride, const quint8 *src, qint32 srcRowStride, const quint8 *mask, qint32 maskRowStride, qint32 rows, qint32 columns, float opacity);
    void compositeValue(quint8 *dst, qint32 dstRowStride, const quint8 *src, qint32 srcRowStride, const quint8 *mask, qint32 maskRowStride, qint32 rows, qint32 columns, float opacity);
    void compositeColor(quint8 *dst, qint32 dstRowStride, const quint8 *src, qint32 srcRowStride, const quint8 *mask, qint32 maskRowStride, qint32 rows, qint32 columns, float opacity);
    void compositeErase(quint8 *dst, qint32 dstRowStride, const quint8 *src, qint32 srcRowStride, const quint8 *mask, qint32 maskRowStride, qint32 rows, qint32 columns, float opacity);

private:
    friend class KisRgbF32ColorSpaceTester;

    static const quint8 PIXEL_BLUE = 0;
    static const quint8 PIXEL_GREEN = 1;
    static const quint8 PIXEL_RED = 2;
    static const quint8 PIXEL_ALPHA = 3;

    struct Pixel {
        float blue;
        float green;
        float red;
        float alpha;
    };
};

// FIXME: lcms doesn't support 32-bit float
#define F32_LCMS_TYPE TYPE_BGRA_16

class KisRgbF32ColorSpaceFactory : public KoColorSpaceFactory
{
public:
    /**
     * Krita definition for use in .kra files and internally: unchanging name +
     * i18n'able description.
     */
    virtual KoID id() const { return KoID("RGBAF32", i18n("RGB (32-bit float/channel)")); };

    /**
     * lcms colorspace type definition.
     */
    virtual quint32 colorSpaceType() { return F32_LCMS_TYPE; };

    virtual icColorSpaceSignature colorSpaceSignature() { return icSigRgbData; };

    virtual KoColorSpace *createColorSpace(KoColorSpaceRegistry * parent, KoColorProfile *p) { return new KisRgbF32ColorSpace(parent, p); };

    virtual QString defaultProfile() { return "sRGB built-in - (lcms internal)"; };
};

#endif // KIS_STRATEGY_COLORSPACE_RGB_F32_H_

