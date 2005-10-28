/*
 *  Copyright (c) 2002 Patrick Julien  <freak@codepimps.org>
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
#ifndef KIS_STRATEGY_COLORSPACE_TESTCS_H_
#define KIS_STRATEGY_COLORSPACE_TESTCS_H_

#include <qcolor.h>

#include "kis_global.h"
#include "kis_abstract_colorspace.h"
#include "kis_pixel.h"
#include "koffice_export.h"

class KRITATOOL_EXPORT KisTestColorSpace : public KisAbstractColorSpace {
public:
    KisTestColorSpace();
    virtual ~KisTestColorSpace();

public:
    virtual void fromQColor(const QColor& c, Q_UINT8 *dst, KisProfile *  profile = 0);
    virtual void fromQColor(const QColor& c, Q_UINT8 opacity, Q_UINT8 *dst, KisProfile *  profile = 0);

    virtual void getAlpha(const Q_UINT8 *pixel, Q_UINT8 *alpha);

    virtual void toQColor(const Q_UINT8 *src, QColor *c, KisProfile *  profile = 0);
    virtual void toQColor(const Q_UINT8 *src, QColor *c, Q_UINT8 *opacity, KisProfile *  profile = 0);

    virtual KisPixelRO toKisPixelRO(const Q_UINT8 *src, KisProfile *  profile = 0)
        { return KisPixelRO (src, src + 4, this, profile); }
    virtual KisPixel toKisPixel(Q_UINT8 *src, KisProfile *  profile = 0)
        { return KisPixel (src, src + 4, this, profile); }

    virtual Q_INT8 difference(const Q_UINT8 *src1, const Q_UINT8 *src2);
    virtual void mixColors(const Q_UINT8 **colors, const Q_UINT8 *weights, Q_UINT32 nColors, Q_UINT8 *dst) const;

    virtual QValueVector<KisChannelInfo *> channels() const;
    virtual bool hasAlpha() const;
    virtual Q_INT32 nChannels() const;
    virtual Q_INT32 nColorChannels() const;
    virtual Q_INT32 pixelSize() const;

    virtual QString channelValueText(const Q_UINT8 *pixel, Q_UINT32 channelIndex) const;
    virtual QString normalisedChannelValueText(const Q_UINT8 *pixel, Q_UINT32 channelIndex) const;

    virtual QImage convertToQImage(const Q_UINT8 *data, Q_INT32 width, Q_INT32 height,
                       KisProfile *  srcProfile, KisProfile *  dstProfile,
                       Q_INT32 renderingIntent,
                       float exposure = 0.0f);

    virtual KisCompositeOpList userVisiblecompositeOps() const;


protected:

    virtual void bitBlt(Q_UINT8 *dst,
                Q_INT32 dstRowStride,
                const Q_UINT8 *src,
                Q_INT32 srcRowStride,
                const Q_UINT8 *srcAlphaMask,
                Q_INT32 maskRowStride,
                Q_UINT8 opacity,
                Q_INT32 rows,
                Q_INT32 cols,
                const KisCompositeOp& op);

    void compositeOver(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *srcAlphaMask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, Q_UINT8 opacity);
    void compositeErase(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *srcAlphaMask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, Q_UINT8 opacity);
    void compositeCopy(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *srcAlphaMask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, Q_UINT8 opacity);

};

#endif // KIS_STRATEGY_COLORSPACE_TESTCS_H_
