/*
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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
#ifndef KIS_STRATEGY_COLORSPACE_GRAYSCALE_H_
#define KIS_STRATEGY_COLORSPACE_GRAYSCALE_H_
#include <qcolor.h>

#include <klocale.h>

#include <krita_export.h>

#include "kis_global.h"
#include "kis_abstract_colorspace.h"
#include "kis_u8_base_colorspace.h"

class KRITACOLOR_EXPORT KisGrayColorSpace : public KisU8BaseColorSpace {
public:
    KisGrayColorSpace(KisColorSpaceFactoryRegistry * parent, KisProfile *p);
    virtual ~KisGrayColorSpace();

    virtual bool willDegrade(ColorSpaceIndependence /*independence*/)
        {
            return false;
        };

public:

    void setPixel(quint8 *pixel, quint8 gray, quint8 alpha) const;
    void getPixel(const quint8 *pixel, quint8 *gray, quint8 *alpha) const;

    virtual void getAlpha(const quint8 *pixel, quint8 *alpha) const;
    virtual void setAlpha(quint8 * pixels, quint8 alpha, qint32 nPixels) const;

    virtual void mixColors(const quint8 **colors, const quint8 *weights, quint32 nColors, quint8 *dst) const;
    virtual void convolveColors(quint8** colors, qint32* kernelValues, KisChannelInfo::enumChannelFlags channelFlags, quint8 *dst, qint32 factor, qint32 offset, qint32 nColors) const;
    virtual void invertColor(quint8 * src, qint32 nPixels);
    virtual void darken(const quint8 * src, quint8 * dst, qint32 shade, bool compensate, double compensation, qint32 nPixels) const;
    virtual quint8 intensity8(const quint8 * src) const;

    virtual Q3ValueVector<KisChannelInfo *> channels() const;
    virtual quint32 nChannels() const;
    virtual quint32 nColorChannels() const;
    virtual quint32 pixelSize() const;

    virtual void bitBlt(quint8 *dst,
                qint32 dststride,
                const quint8 *src,
                qint32 srcRowStride,
                const quint8 *srcAlphaMask,
                qint32 maskRowStride,
                quint8 opacity,
                qint32 rows,
                qint32 cols,
                const KisCompositeOp& op);

    KisCompositeOpList userVisiblecompositeOps() const;

protected:
    void compositeOver(quint8 *dst, qint32 dstRowStride, const quint8 *src, qint32 srcRowStride, const quint8 *mask, qint32 maskRowStride, qint32 rows, qint32 columns, quint8 opacity);
    void compositeMultiply(quint8 *dst, qint32 dstRowStride, const quint8 *src, qint32 srcRowStride, const quint8 *mask, qint32 maskRowStride, qint32 rows, qint32 columns, quint8 opacity);
    void compositeDivide(quint8 *dst, qint32 dstRowStride, const quint8 *src, qint32 srcRowStride, const quint8 *mask, qint32 maskRowStride, qint32 rows, qint32 columns, quint8 opacity);
    void compositeScreen(quint8 *dst, qint32 dstRowStride, const quint8 *src, qint32 srcRowStride, const quint8 *mask, qint32 maskRowStride, qint32 rows, qint32 columns, quint8 opacity);
    void compositeOverlay(quint8 *dst, qint32 dstRowStride, const quint8 *src, qint32 srcRowStride, const quint8 *mask, qint32 maskRowStride, qint32 rows, qint32 columns, quint8 opacity);
    void compositeDodge(quint8 *dst, qint32 dstRowStride, const quint8 *src, qint32 srcRowStride, const quint8 *mask, qint32 maskRowStride, qint32 rows, qint32 columns, quint8 opacity);
    void compositeBurn(quint8 *dst, qint32 dstRowStride, const quint8 *src, qint32 srcRowStride, const quint8 *mask, qint32 maskRowStride, qint32 rows, qint32 columns, quint8 opacity);
    void compositeDarken(quint8 *dst, qint32 dstRowStride, const quint8 *src, qint32 srcRowStride, const quint8 *mask, qint32 maskRowStride, qint32 rows, qint32 columns, quint8 opacity);
    void compositeLighten(quint8 *dst, qint32 dstRowStride, const quint8 *src, qint32 srcRowStride, const quint8 *mask, qint32 maskRowStride, qint32 rows, qint32 columns, quint8 opacity);
    void compositeErase(quint8 *dst, qint32 dstRowStride, const quint8 *src, qint32 srcRowStride, const quint8 *mask, qint32 maskRowStride, qint32 rows, qint32 columns, quint8 opacity);

private:
    friend class KisGrayColorSpaceTester;

    static const quint8 PIXEL_GRAY = 0;
    static const quint8 PIXEL_GRAY_ALPHA = 1;
};

class KisGrayColorSpaceFactory : public KisColorSpaceFactory
{
public:
    /**
     * Krita definition for use in .kra files and internally: unchanging name +
     * i18n'able description.
     */
    virtual KisID id() const { return KisID("GRAYA", i18n("Grayscale (8-bit integer/channel)")); };

    /**
     * lcms colorspace type definition.
     */
    virtual quint32 colorSpaceType() { return TYPE_GRAYA_8; };

    virtual icColorSpaceSignature colorSpaceSignature() { return icSigGrayData; };

    virtual KisColorSpace *createColorSpace(KisColorSpaceFactoryRegistry * parent, KisProfile *p) { return new KisGrayColorSpace(parent, p); };

    virtual QString defaultProfile() { return "gray built-in - (lcms internal)"; };
};

#endif // KIS_STRATEGY_COLORSPACE_GRAYSCALE_H_
