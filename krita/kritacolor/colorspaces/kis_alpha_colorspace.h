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

#include <QColor>

#include "kis_global.h"
#include "kis_u8_base_colorspace.h"

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
    virtual bool willDegrade(ColorSpaceIndependence)
        {
            return false;
        };

    virtual void fromQColor(const QColor& c, quint8 *dst, KisProfile * profile = 0);
    virtual void fromQColor(const QColor& c, quint8 opacity, quint8 *dst, KisProfile * profile = 0);

    virtual void getAlpha(const quint8 *pixel, quint8 *alpha) const;

    virtual void toQColor(const quint8 *src, QColor *c, KisProfile * profile = 0);
    virtual void toQColor(const quint8 *src, QColor *c, quint8 *opacity, KisProfile * profile = 0);

    virtual quint8 difference(const quint8 *src1, const quint8 *src2);
    virtual void mixColors(const quint8 **colors, const quint8 *weights, quint32 nColors, quint8 *dst) const;

    virtual Q3ValueVector<KisChannelInfo *> channels() const;
    virtual quint32 nChannels() const { return 1; };
    virtual quint32 nColorChannels() const { return 0; };
    virtual quint32 pixelSize() const { return 1; };

    virtual QString channelValueText(const quint8 *pixel, quint32 channelIndex) const;
    virtual QString normalisedChannelValueText(const quint8 *pixel, quint32 channelIndex) const;

    virtual void convolveColors(quint8** colors, qint32* kernelValues, KisChannelInfo::enumChannelFlags channelFlags, quint8 *dst, qint32 factor, qint32 offset, qint32 nColors) const;

protected:

    /**
     * Convert a byte array of srcLen pixels *src to the specified color space
     * and put the converted bytes into the prepared byte array *dst.
     *
     * Returns false if the conversion failed, true if it succeeded
     */
    virtual bool convertPixelsTo(const quint8 *src,
                     quint8 *dst, KisAbstractColorSpace * dstColorSpace,
                     quint32 numPixels,
                     qint32 renderingIntent = INTENT_PERCEPTUAL);



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

};

#endif // KIS_COLORSPACE_ALPHA_H_
