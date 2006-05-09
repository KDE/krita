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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KIS_STRATEGY_COLORSPACE_RGB_U16_H_
#define KIS_STRATEGY_COLORSPACE_RGB_U16_H_

#include <klocale.h>

#include <krita_export.h>

#include "kis_global.h"
#include "kis_u16_base_colorspace.h"
#include "kis_integer_maths.h"


class KRITACOLOR_EXPORT KisRgbU16ColorSpace : public KisU16BaseColorSpace {
public:

    struct Pixel {
        quint16 blue;
        quint16 green;
        quint16 red;
        quint16 alpha;
    };
public:
    KisRgbU16ColorSpace(KisColorSpaceFactoryRegistry * parent, KisProfile *p);
    virtual ~KisRgbU16ColorSpace();

    virtual bool willDegrade(ColorSpaceIndependence independence)
        {
            if (independence == TO_RGBA8)
                return true;
            else
                return false;
        };



public:
    void setPixel(quint8 *pixel, quint16 red, quint16 green, quint16 blue, quint16 alpha) const;
    void getPixel(const quint8 *pixel, quint16 *red, quint16 *green, quint16 *blue, quint16 *alpha) const;

    virtual void mixColors(const quint8 **colors, const quint8 *weights, quint32 nColors, quint8 *dst) const;
    virtual void convolveColors(quint8** colors, qint32* kernelValues, KisChannelInfo::enumChannelFlags channelFlags, quint8 *dst, qint32 factor, qint32 offset, qint32 nColors) const;
    virtual void invertColor(quint8 * src, qint32 nPixels);
    virtual quint8 intensity8(const quint8 * src) const;

    virtual Q3ValueVector<KisChannelInfo *> channels() const;
    virtual quint32 nChannels() const;
    virtual quint32 nColorChannels() const;
    virtual quint32 pixelSize() const;

    virtual KisCompositeOpList userVisiblecompositeOps() const;

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
                const KisCompositeOp& op);

    void compositeOver(quint8 *dst, qint32 dstRowStride, const quint8 *src, qint32 srcRowStride, const quint8 *mask, qint32 maskRowStride, qint32 rows, qint32 columns, quint16 opacity);
    void compositeMultiply(quint8 *dst, qint32 dstRowStride, const quint8 *src, qint32 srcRowStride, const quint8 *mask, qint32 maskRowStride, qint32 rows, qint32 columns, quint16 opacity);
    void compositeDivide(quint8 *dst, qint32 dstRowStride, const quint8 *src, qint32 srcRowStride, const quint8 *mask, qint32 maskRowStride, qint32 rows, qint32 columns, quint16 opacity);
    void compositeScreen(quint8 *dst, qint32 dstRowStride, const quint8 *src, qint32 srcRowStride, const quint8 *mask, qint32 maskRowStride, qint32 rows, qint32 columns, quint16 opacity);
    void compositeOverlay(quint8 *dst, qint32 dstRowStride, const quint8 *src, qint32 srcRowStride, const quint8 *mask, qint32 maskRowStride, qint32 rows, qint32 columns, quint16 opacity);
    void compositeDodge(quint8 *dst, qint32 dstRowStride, const quint8 *src, qint32 srcRowStride, const quint8 *mask, qint32 maskRowStride, qint32 rows, qint32 columns, quint16 opacity);
    void compositeBurn(quint8 *dst, qint32 dstRowStride, const quint8 *src, qint32 srcRowStride, const quint8 *mask, qint32 maskRowStride, qint32 rows, qint32 columns, quint16 opacity);
    void compositeDarken(quint8 *dst, qint32 dstRowStride, const quint8 *src, qint32 srcRowStride, const quint8 *mask, qint32 maskRowStride, qint32 rows, qint32 columns, quint16 opacity);
    void compositeLighten(quint8 *dst, qint32 dstRowStride, const quint8 *src, qint32 srcRowStride, const quint8 *mask, qint32 maskRowStride, qint32 rows, qint32 columns, quint16 opacity);
    void compositeHue(quint8 *dst, qint32 dstRowStride, const quint8 *src, qint32 srcRowStride, const quint8 *mask, qint32 maskRowStride, qint32 rows, qint32 columns, quint16 opacity);
    void compositeSaturation(quint8 *dst, qint32 dstRowStride, const quint8 *src, qint32 srcRowStride, const quint8 *mask, qint32 maskRowStride, qint32 rows, qint32 columns, quint16 opacity);
    void compositeValue(quint8 *dst, qint32 dstRowStride, const quint8 *src, qint32 srcRowStride, const quint8 *mask, qint32 maskRowStride, qint32 rows, qint32 columns, quint16 opacity);
    void compositeColor(quint8 *dst, qint32 dstRowStride, const quint8 *src, qint32 srcRowStride, const quint8 *mask, qint32 maskRowStride, qint32 rows, qint32 columns, quint16 opacity);
    void compositeErase(quint8 *dst, qint32 dstRowStride, const quint8 *src, qint32 srcRowStride, const quint8 *mask, qint32 maskRowStride, qint32 rows, qint32 columns, quint16 opacity);

private:
    friend class KisRgbU16ColorSpaceTester;

    static const quint8 PIXEL_BLUE = 0;
    static const quint8 PIXEL_GREEN = 1;
    static const quint8 PIXEL_RED = 2;
    static const quint8 PIXEL_ALPHA = 3;
};

class KisRgbU16ColorSpaceFactory : public KisColorSpaceFactory
{
public:
    /**
     * Krita definition for use in .kra files and internally: unchanging name +
     * i18n'able description.
     */
    virtual KisID id() const { return KisID("RGBA16", i18n("RGB (16-bit integer/channel)")); };

    /**
     * lcms colorspace type definition.
     */
    virtual quint32 colorSpaceType() { return TYPE_BGRA_16; };

    virtual icColorSpaceSignature colorSpaceSignature() { return icSigRgbData; };

    virtual KisColorSpace *createColorSpace(KisColorSpaceFactoryRegistry * parent, KisProfile *p) { return new KisRgbU16ColorSpace(parent, p); };

    virtual QString defaultProfile() { return "sRGB built-in - (lcms internal)"; };
};

#endif // KIS_STRATEGY_COLORSPACE_RGB_U16_H_
