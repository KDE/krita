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

#include <koffice_export.h>

#include "kis_global.h"
#include "kis_abstract_colorspace.h"
#include "kis_u8_base_colorspace.h"
#include "kis_pixel.h"

class KRITACORE_EXPORT KisGrayColorSpace : public KisU8BaseColorSpace {
public:
    KisGrayColorSpace(KisColorSpaceFactoryRegistry * parent, KisProfile *p);
    virtual ~KisGrayColorSpace();

public:

    void setPixel(Q_UINT8 *pixel, Q_UINT8 gray, Q_UINT8 alpha) const;
    void getPixel(const Q_UINT8 *pixel, Q_UINT8 *gray, Q_UINT8 *alpha) const;

    virtual void fromQColor(const QColor& c, Q_UINT8 *dst);
    virtual void fromQColor(const QColor& c, Q_UINT8 opacity, Q_UINT8 *dst);

    virtual void getAlpha(const Q_UINT8 *pixel, Q_UINT8 *alpha);
    virtual void setAlpha(Q_UINT8 * pixels, Q_UINT8 alpha, Q_INT32 nPixels);

    virtual void toQColor(const Q_UINT8 *src, QColor *c);
    virtual void toQColor(const Q_UINT8 *src, QColor *c, Q_UINT8 *opacity);

    virtual KisPixelRO toKisPixelRO(const Q_UINT8 *src)
        { return KisPixelRO (src, src + PIXEL_GRAY_ALPHA, this); }

    virtual KisPixel toKisPixel(Q_UINT8 *src)
        { return KisPixel (src, src + PIXEL_GRAY_ALPHA, this); }

    virtual Q_INT8 difference(const Q_UINT8 *src1, const Q_UINT8 *src2);
    virtual void mixColors(const Q_UINT8 **colors, const Q_UINT8 *weights, Q_UINT32 nColors, Q_UINT8 *dst) const;

    virtual QValueVector<KisChannelInfo *> channels() const;
    virtual bool hasAlpha() const;
    virtual Q_INT32 nChannels() const;
    virtual Q_INT32 nColorChannels() const;
    virtual Q_INT32 pixelSize() const;

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
    void compositeOver(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, Q_UINT8 opacity);
    void compositeMultiply(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, Q_UINT8 opacity);
    void compositeDivide(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, Q_UINT8 opacity);
    void compositeScreen(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, Q_UINT8 opacity);
    void compositeOverlay(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, Q_UINT8 opacity);
    void compositeDodge(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, Q_UINT8 opacity);
    void compositeBurn(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, Q_UINT8 opacity);
    void compositeDarken(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, Q_UINT8 opacity);
    void compositeLighten(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, Q_UINT8 opacity);
    void compositeErase(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, Q_UINT8 opacity);

private:
    friend class KisGrayColorSpaceTester;

    static const Q_UINT8 PIXEL_GRAY = 0;
    static const Q_UINT8 PIXEL_GRAY_ALPHA = 1;
};

class KisGrayColorSpaceFactory : public KisColorSpaceFactory
{
public:
    /**
     * Krita definition for use in .kra files and internally: unchanging name +
     * i18n'able description.
     */
    virtual KisID id() const { return KisID("GRAYA", i18n("Grayscale/Alpha")); };

    /**
     * lcms colorspace type definition.
     */
    virtual Q_UINT32 colorSpaceType() { return TYPE_GRAYA_8; };

    virtual icColorSpaceSignature colorSpaceSignature() { return icSigGrayData; };

    virtual KisColorSpace *createColorSpace(KisColorSpaceFactoryRegistry * parent, KisProfile *p) { return new KisGrayColorSpace(parent, p); };

    virtual QString defaultProfile() { return "gray built-in - (lcms internal)"; };
};

#endif // KIS_STRATEGY_COLORSPACE_GRAYSCALE_H_
