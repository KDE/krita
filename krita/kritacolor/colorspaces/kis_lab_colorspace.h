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
#ifndef KIS_STRATEGY_COLORSPACE_LAB_H_
#define KIS_STRATEGY_COLORSPACE_LAB_H_

#include <QColor>

#include <klocale.h>

#include "kis_global.h"
#include "kis_integer_maths.h"
#include "kis_u16_base_colorspace.h"

class KisLabColorSpace : public KisU16BaseColorSpace {
public:
    KisLabColorSpace(KisColorSpaceFactoryRegistry * parent, KisProfile *p);
    virtual ~KisLabColorSpace();

public:

    /**
     * Return a COPY of the provided data. This method is provided to provide consistency,
     * but you really don't want to be calling it.
     */
    virtual quint8 * toLabA16(const quint8 * data, const quint32 nPixels) const;

    /**
     * Return a COPY of the provided data. This method is provided for consistency,
     * but you really don't want to call it.
     */
    virtual quint8 * fromLabA16(const quint8 * labData, const quint32 nPixels) const;



    virtual bool willDegrade(ColorSpaceIndependence independence)
        {
            if (independence == TO_RGBA8) 
                return true;
            else
                return false;
        };

    virtual Q3ValueVector<KisChannelInfo *> channels() const;
    virtual quint32 nChannels() const;
    virtual quint32 nColorChannels() const;
    virtual quint32 pixelSize() const;

    virtual QString channelValueText(const quint8 *pixel, quint32 channelIndex) const;
    virtual QString normalisedChannelValueText(const quint8 *pixel, quint32 channelIndex) const;
    virtual void getSingleChannelPixel(quint8 *dstPixel, const quint8 *srcPixel, quint32 channelIndex);

    virtual quint8 difference(const quint8 *src1, const quint8 *src2);
    virtual void mixColors(const quint8 **colors, const quint8 *weights, quint32 nColors, quint8 *dst) const;
    virtual void invertColor(quint8 * src, qint32 nPixels);
    virtual void convolveColors(quint8** colors, qint32 * kernelValues, KisChannelInfo::enumChannelFlags channelFlags, quint8 *dst, qint32 factor, qint32 offset, qint32 nColors) const;
    
    virtual void darken(const quint8 * src, quint8 * dst, qint32 shade, bool compensate, double compensation, qint32 nPixels) const;

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
/*
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
*/
    void compositeErase(quint8 *dst, qint32 dstRowStride, const quint8 *src, qint32 srcRowStride, const quint8 *mask, qint32 maskRowStride, qint32 rows, qint32 columns, quint16 opacity);

private:
    struct Pixel {
        quint16 lightness;
        quint16 a;
        quint16 b;
        quint16 alpha;
    };
    static const quint16 U16_OPACITY_OPAQUE = UINT16_MAX;
    static const quint16 U16_OPACITY_TRANSPARENT = UINT16_MIN;

    static const quint32 NUM_CHANNELS = 4;
    static const quint32 NUM_COLOR_CHANNELS = 3;

    static const quint32 CHANNEL_L = 0;
    static const quint32 CHANNEL_A = 1;
    static const quint32 CHANNEL_B = 2;
    static const quint32 CHANNEL_ALPHA = 3;

    static const quint32 MAX_CHANNEL_L = 0xff00;
    static const quint32 MAX_CHANNEL_AB = 0xffff;
    static const quint32 CHANNEL_AB_ZERO_OFFSET = 0x8000;

    friend class KisLabColorSpaceTester;
};

class KisLabColorSpaceFactory : public KisColorSpaceFactory
{
public:
    /**
     * Krita definition for use in .kra files and internally: unchanging name +
     * i18n'able description.
     */
    virtual KisID id() const { return KisID("LABA", i18n("L*a*b* (16-bit integer/channel)")); };

    /**
     * lcms colorspace type definition.
     */
    virtual quint32 colorSpaceType() { return (COLORSPACE_SH(PT_Lab)|CHANNELS_SH(3)|BYTES_SH(2)|EXTRA_SH(1)); };

    virtual icColorSpaceSignature colorSpaceSignature() { return icSigLabData; };
    
    virtual KisColorSpace *createColorSpace(KisColorSpaceFactoryRegistry * parent, KisProfile *p) { return new KisLabColorSpace(parent, p); };

    virtual QString defaultProfile() { return "Lab built-in - (lcms internal)"; };
};

#endif // KIS_STRATEGY_COLORSPACE_LAB_H_
