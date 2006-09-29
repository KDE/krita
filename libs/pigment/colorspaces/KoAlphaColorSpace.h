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
#ifndef KOALPHACOLORSPACE_H
#define KOALPHACOLORSPACE_H

#include <QColor>

#include <koffice_export.h>

#include "KoLcmsColorSpace.h"


struct AlphaU8Traits {
    typedef quint8 channels_type;
    static const quint32 channels_nb = 1;
    static const qint32 alpha_pos = 0;
};

/**
 * The alpha mask is a special color strategy that treats all pixels as
 * alpha value with a color common to the mask. The default color is white.
 */
class PIGMENT_EXPORT KoAlphaColorSpace : public KoLcmsColorSpace<AlphaU8Traits> {
public:
    KoAlphaColorSpace(KoColorSpaceRegistry * parent,
                       KoColorProfile *p);
    virtual ~KoAlphaColorSpace();

public:
    virtual bool willDegrade(ColorSpaceIndependence independence) const
        {
            Q_UNUSED(independence);
            return false;
        };

    virtual void fromQColor(const QColor& c, quint8 *dst, KoColorProfile * profile = 0);
    virtual void fromQColor(const QColor& c, quint8 opacity, quint8 *dst, KoColorProfile * profile = 0);

    virtual void getAlpha(const quint8 *pixel, quint8 *alpha) const;

    virtual void toQColor(const quint8 *src, QColor *c, KoColorProfile * profile = 0);
    virtual void toQColor(const quint8 *src, QColor *c, quint8 *opacity, KoColorProfile * profile = 0);

    virtual quint8 difference(const quint8 *src1, const quint8 *src2);
    virtual void mixColors(const quint8 **colors, const quint8 *weights, quint32 nColors, quint8 *dst) const;

    virtual Q3ValueVector<KoChannelInfo *> channels() const;
    virtual quint32 nChannels() const { return 1; };
    virtual quint32 nColorChannels() const { return 0; };
    virtual quint32 pixelSize() const { return 1; };

    virtual QString channelValueText(const quint8 *pixel, quint32 channelIndex) const;
    virtual QString normalisedChannelValueText(const quint8 *pixel, quint32 channelIndex) const;

    virtual void convolveColors(quint8** colors, qint32* kernelValues, KoChannelInfo::enumChannelFlags channelFlags, quint8 *dst, qint32 factor, qint32 offset, qint32 nColors) const;

protected:

    /**
     * Convert a byte array of srcLen pixels *src to the specified color space
     * and put the converted bytes into the prepared byte array *dst.
     *
     * Returns false if the conversion failed, true if it succeeded
     */
    virtual bool convertPixelsTo(const quint8 *src,
                     quint8 *dst, KoColorSpace * dstColorSpace,
                     quint32 numPixels,
                     qint32 renderingIntent = INTENT_PERCEPTUAL);

};

#endif // KIS_COLORSPACE_ALPHA_H_
