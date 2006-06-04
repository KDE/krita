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

#include <QColor>

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
        quint8 Qt::blue;
        quint8 Qt::green;
        quint8 Qt::red;
        quint8 alpha;

        float   hue;
        float   saturation;
        float   lightness;

        quint8 liquid_content;
        quint8 drying_rate;
        quint8 miscibility;

        enumDirection direction;
        quint8 strength;

        quint8  absorbancy;  /* How much paint can this cell hold? */
        quint8  volume;      /* The volume of paint. */

    } CELL, *CELL_PTR;


}



class KisWetStickyColorSpace : public KisAbstractColorSpace {
public:
    KisWetStickyColorSpace();
    virtual ~KisWetStickyColorSpace();

public:



    virtual void fromQColor(const QColor& c, quint8 *dst, KoColorProfile *  profile = 0);
    virtual void fromQColor(const QColor& c, quint8 opacity, quint8 *dst, KoColorProfile *  profile = 0);

    virtual void toQColor(const quint8 *src, QColor *c, KoColorProfile *  profile = 0);
    virtual void toQColor(const quint8 *src, QColor *c, quint8 *opacity, KoColorProfile *  profile = 0);

    virtual quint8 getAlpha(const quint8 *pixel) const;
    virtual void setAlpha(quint8 * pixels, quint8 alpha, qint32 nPixels) const;

    virtual void applyAlphaU8Mask(quint8 * pixels, quint8 * alpha, qint32 nPixels);
    virtual void applyInverseAlphaU8Mask(quint8 * pixels, quint8 * alpha, qint32 nPixels);

    virtual quint8 scaleToU8(const quint8 * srcPixel, qint32 channelPos);
    virtual quint16 scaleToU16(const quint8 * srcPixel, qint32 channelPos);

    virtual Q3ValueVector<KoChannelInfo *> channels() const;
    virtual bool hasAlpha() const;
    virtual qint32 nChannels() const;
    virtual qint32 nColorChannels() const;
    virtual qint32 nSubstanceChannels() const;
    virtual qint32 pixelSize() const;

    virtual QString channelValueText(const quint8 *pixel, quint32 channelIndex) const;
    virtual QString normalisedChannelValueText(const quint8 *pixel, quint32 channelIndex) const;

    virtual QImage convertToQImage(const quint8 *data, qint32 width, qint32 height,
                       KoColorProfile *  srcProfile, KoColorProfile *  dstProfile,
                       qint32 renderingIntent = INTENT_PERCEPTUAL,
                       float exposure = 0.0f);


    virtual void mixColors(const quint8 **colors, const quint8 *weights, quint32 nColors, quint8 *dst) const;
    virtual void convolveColors(quint8** colors, qint32* kernelValues, KoChannelInfo::enumChannelFlags channelFlags, quint8 *dst, qint32 factor, qint32 offset, qint32 nColors) const;
    virtual void invertColor(quint8 * src, qint32 nPixels);
    virtual void darken(const quint8 * src, quint8 * dst, qint32 shade, bool compensate, double compensation, qint32 nPixels) const;

    virtual KoCompositeOpList userVisiblecompositeOps() const;

protected:

    virtual void bitBlt(quint8 *dst,
                qint32 dstRowSize,
                const quint8 *src,
                qint32 srcRowStride,
                const quint8 *srcAlphaMask,
                qint32 maskRowStride,
                quint8 opacity,
                qint32 rows,
                qint32 cols,
                const KoCompositeOp& op);


    virtual bool convertPixelsTo(const quint8 * src, KoColorProfile *  srcProfile,
                     quint8 * dst, KisAbstractColorSpace * dstColorSpace, KoColorProfile *  dstProfile,
                     quint32 numPixels,
                     qint32 renderingIntent = INTENT_PERCEPTUAL);


private:

    void compositeOver(quint8 *dst, qint32 dstRowStride, const quint8 *src, qint32 srcRowStride, const quint8 *mask, qint32 maskRowStride, qint32 rows, qint32 columns, quint8 opacity);
    void compositeClear(quint8 *dst, qint32 dstRowStride, const quint8 *src, qint32 srcRowStride, const quint8 *mask, qint32 maskRowStride, qint32 rows, qint32 columns, quint8 opacity);
    void compositeCopy(quint8 *dst, qint32 dstRowStride, const quint8 *src, qint32 srcRowStride, const quint8 *mask, qint32 maskRowStride, qint32 rows, qint32 columns, quint8 opacity);

};

#endif // KIS_COLORSPACE_WET_STICKY_H_
