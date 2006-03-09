/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.o>
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
#ifndef KIS_COLORSPACE_WET_STICKY_H_
#define KIS_COLORSPACE_WET_STICKY_H_

#include <qcolor.h>

#include "kis_global.h"
#include "kis_abstract_colorspace.h"

namespace WetAndSticky {

    /**
         * A color is specified as a vector in HLS space.  Hue is a value
     * in the range 0..360 degrees with 0 degrees being red.  Saturation
         * and Lightness are both in the range [0,1].  A lightness of 0 means
     * black, with 1 being white.  A totally saturated color has saturation
     * of 1.
     */

    enum enumDirection {
        UP,
        DOWN,
        LEFT,
        RIGHT
    };

    /**
     * Defines the contents and attributes of a cell on the canvas.
     */
    typedef struct cell {
        Q_UINT8 blue;
        Q_UINT8 green;
        Q_UINT8 red;
        Q_UINT8 alpha;

        float   hue;
        float   saturation;
        float   lightness;

        Q_UINT8 liquid_content;
        Q_UINT8 drying_rate;
        Q_UINT8 miscibility;

        enumDirection direction;
        Q_UINT8 strength;

        Q_UINT8  absorbancy;  /* How much paint can this cell hold? */
        Q_UINT8  volume;      /* The volume of paint. */

    } CELL, *CELL_PTR;


}



class KisWetStickyColorSpace : public KisAbstractColorSpace {
public:
    KisWetStickyColorSpace();
    virtual ~KisWetStickyColorSpace();

public:



    virtual void fromQColor(const QColor& c, Q_UINT8 *dst, KisProfile *  profile = 0);
    virtual void fromQColor(const QColor& c, Q_UINT8 opacity, Q_UINT8 *dst, KisProfile *  profile = 0);

    virtual void toQColor(const Q_UINT8 *src, QColor *c, KisProfile *  profile = 0);
    virtual void toQColor(const Q_UINT8 *src, QColor *c, Q_UINT8 *opacity, KisProfile *  profile = 0);

    virtual Q_UINT8 getAlpha(const Q_UINT8 *pixel) const;
    virtual void setAlpha(Q_UINT8 * pixels, Q_UINT8 alpha, Q_INT32 nPixels) const;

    virtual void applyAlphaU8Mask(Q_UINT8 * pixels, Q_UINT8 * alpha, Q_INT32 nPixels);
    virtual void applyInverseAlphaU8Mask(Q_UINT8 * pixels, Q_UINT8 * alpha, Q_INT32 nPixels);

    virtual Q_UINT8 scaleToU8(const Q_UINT8 * srcPixel, Q_INT32 channelPos);
    virtual Q_UINT16 scaleToU16(const Q_UINT8 * srcPixel, Q_INT32 channelPos);

    virtual QValueVector<KisChannelInfo *> channels() const;
    virtual bool hasAlpha() const;
    virtual Q_INT32 nChannels() const;
    virtual Q_INT32 nColorChannels() const;
    virtual Q_INT32 nSubstanceChannels() const;
    virtual Q_INT32 pixelSize() const;

    virtual QString channelValueText(const Q_UINT8 *pixel, Q_UINT32 channelIndex) const;
    virtual QString normalisedChannelValueText(const Q_UINT8 *pixel, Q_UINT32 channelIndex) const;

    virtual QImage convertToQImage(const Q_UINT8 *data, Q_INT32 width, Q_INT32 height,
                       KisProfile *  srcProfile, KisProfile *  dstProfile,
                       Q_INT32 renderingIntent = INTENT_PERCEPTUAL,
                       float exposure = 0.0f);


    virtual void mixColors(const Q_UINT8 **colors, const Q_UINT8 *weights, Q_UINT32 nColors, Q_UINT8 *dst) const;
    virtual void convolveColors(Q_UINT8** colors, Q_INT32* kernelValues, KisChannelInfo::enumChannelFlags channelFlags, Q_UINT8 *dst, Q_INT32 factor, Q_INT32 offset, Q_INT32 nColors) const;
    virtual void invertColor(Q_UINT8 * src, Q_INT32 nPixels);
    virtual void darken(const Q_UINT8 * src, Q_UINT8 * dst, Q_INT32 shade, bool compensate, double compensation, Q_INT32 nPixels) const;

    virtual KisCompositeOpList userVisiblecompositeOps() const;

protected:

    virtual void bitBlt(Q_UINT8 *dst,
                Q_INT32 dstRowSize,
                const Q_UINT8 *src,
                Q_INT32 srcRowStride,
                const Q_UINT8 *srcAlphaMask,
                Q_INT32 maskRowStride,
                Q_UINT8 opacity,
                Q_INT32 rows,
                Q_INT32 cols,
                const KisCompositeOp& op);


    virtual bool convertPixelsTo(const Q_UINT8 * src, KisProfile *  srcProfile,
                     Q_UINT8 * dst, KisAbstractColorSpace * dstColorSpace, KisProfile *  dstProfile,
                     Q_UINT32 numPixels,
                     Q_INT32 renderingIntent = INTENT_PERCEPTUAL);


private:

    void compositeOver(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, Q_UINT8 opacity);
    void compositeClear(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, Q_UINT8 opacity);
    void compositeCopy(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, Q_UINT8 opacity);

};

#endif // KIS_COLORSPACE_WET_STICKY_H_
