/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_COLORSPACE_ALPHA_H_
#define KIS_COLORSPACE_ALPHA_H_

#include <qcolor.h>

#include "kis_global.h"
#include "kis_u8_base_colorspace.h"
#include "kis_pixel.h"

/**
 * The alpha mask is a special color strategy that treats all pixels as
 * alpha value with a colour common to the mask. The default color is white.
 */
class KisAlphaColorSpace : public KisU8BaseColorSpace {
public:
    KisAlphaColorSpace(KisColorSpaceFactoryRegistry * parent,
                       KisProfile *p);
    virtual ~KisAlphaColorSpace();

public:
    virtual void fromQColor(const QColor& c, Q_UINT8 *dst);
    virtual void fromQColor(const QColor& c, Q_UINT8 opacity, Q_UINT8 *dst);

    virtual void getAlpha(const Q_UINT8 *pixel, Q_UINT8 *alpha);

    virtual void toQColor(const Q_UINT8 *src, QColor *c);
    virtual void toQColor(const Q_UINT8 *src, QColor *c, Q_UINT8 *opacity);

    virtual KisPixelRO toKisPixelRO(const Q_UINT8 *src) { return KisPixelRO (src, src, this); }
    virtual KisPixel toKisPixel(Q_UINT8 *src) { return KisPixel (src, src, this); }

    virtual Q_INT8 difference(const Q_UINT8 *src1, const Q_UINT8 *src2);
    virtual void mixColors(const Q_UINT8 **colors, const Q_UINT8 *weights, Q_UINT32 nColors, Q_UINT8 *dst) const;

    virtual QValueVector<KisChannelInfo *> channels() const;
    virtual bool hasAlpha() const;
    virtual Q_INT32 nChannels() const { return 1; };
    virtual Q_INT32 nColorChannels() const { return 0; };
    virtual Q_INT32 pixelSize() const { return 1; };

    virtual QString channelValueText(const Q_UINT8 *pixel, Q_UINT32 channelIndex) const;
    virtual QString normalisedChannelValueText(const Q_UINT8 *pixel, Q_UINT32 channelIndex) const;

    virtual void convolveColors(Q_UINT8** colors, Q_INT32* kernelValues, KisChannelInfo::enumChannelFlags channelFlags, Q_UINT8 *dst, Q_INT32 factor, Q_INT32 offset, Q_INT32 nColors) const;

protected:

    /**
     * Convert a byte array of srcLen pixels *src to the specified color space
     * and put the converted bytes into the prepared byte array *dst.
     *
     * Returns false if the conversion failed, true if it succeeded
     */
    virtual bool convertPixelsTo(const Q_UINT8 *src,
                     Q_UINT8 *dst, KisAbstractColorSpace * dstColorSpace,
                     Q_UINT32 numPixels,
                     Q_INT32 renderingIntent = INTENT_PERCEPTUAL);



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

};

#endif // KIS_COLORSPACE_ALPHA_H_
