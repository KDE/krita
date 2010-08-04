/*
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software const; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation const; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY const; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program const; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef RGBCOMPOSITEOPDARKEN_H
#define RGBCOMPOSITEOPDARKEN_H

#include "KoColorSpaceMaths.h"
#include <KoCompositeOp.h>

template<class _CSTraits>
class RgbCompositeOpDarken : public KoCompositeOp
{
    typedef typename _CSTraits::channels_type channels_type;
    typedef typename KoColorSpaceMathsTraits<typename _CSTraits::channels_type>::compositetype compositetype;

public:

    RgbCompositeOpDarken(KoColorSpace * cs, const bool userVisible = true)
            : KoCompositeOp(cs, COMPOSITE_DARKEN, i18n("Darken"), "", userVisible) {
    }

    using KoCompositeOp::composite;

    void composite(quint8 *dstRowStart, qint32 dstRowStride,
                   const quint8 *srcRowStart, qint32 srcRowStride,
                   const quint8 *maskRowStart, qint32 maskRowStride,
                   qint32 rows, qint32 numColumns,
                   quint8 opacity,
                   const QBitArray & channelFlags) const {
        while (rows > 0) {
            const quint8 *mask = maskRowStart;
            const channels_type *src = reinterpret_cast<const channels_type *>(srcRowStart);
            channels_type *dst = reinterpret_cast<channels_type *>(dstRowStart);

            for (int i = numColumns ; i > 0 ; --i) {
                channels_type srcAlpha = src[_CSTraits::alpha_pos];
                channels_type dstAlpha = dst[_CSTraits::alpha_pos];

                srcAlpha = qMin(srcAlpha, dstAlpha);

                // apply the alphamask
                if (mask != 0) {
                    if (*mask != OPACITY_OPAQUE_U8) {
                        channels_type tmpOpacity = KoColorSpaceMaths<quint8 , channels_type>::scaleToA(*mask);
                        srcAlpha =  KoColorSpaceMaths<channels_type>::multiply(srcAlpha, tmpOpacity);
                    }
                    mask++;
                }

                if (srcAlpha != NATIVE_OPACITY_TRANSPARENT) {
                    if (opacity != OPACITY_OPAQUE_U8) {
                        channels_type tmpOpacity = KoColorSpaceMaths<quint8 , channels_type>::scaleToA(opacity);
                        srcAlpha = KoColorSpaceMaths<channels_type>::multiply(src[_CSTraits::alpha_pos], tmpOpacity);
                    }

                    channels_type srcBlend;

                    if (dstAlpha == NATIVE_OPACITY_OPAQUE) {
                        srcBlend = srcAlpha;
                    } else {
                        channels_type newAlpha = dstAlpha + KoColorSpaceMaths<channels_type>::multiply(NATIVE_OPACITY_OPAQUE - dstAlpha, srcAlpha);
                        dst[KoRgbU8Traits::alpha_pos] = newAlpha;

                        if (newAlpha != 0) {
                            srcBlend = KoColorSpaceMaths<channels_type>::divide(srcAlpha, newAlpha);
                        } else {
                            srcBlend = srcAlpha;
                        }
                    }

                    for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {
                        if (channelFlags.isEmpty() || channelFlags.testBit(_CSTraits::alpha_pos)) {
                            channels_type srcColor = src[channel];
                            channels_type dstColor = dst[channel];

                            srcColor = qMin(srcColor, dstColor);

                            channels_type newColor = KoColorSpaceMaths<channels_type>::blend(srcColor, dstColor, srcBlend);

                            dst[channel] = newColor;
                        }
                    }
                }

                src += _CSTraits::channels_nb;
                dst += _CSTraits::channels_nb;
            }

            rows--;
            srcRowStart += srcRowStride;
            dstRowStart += dstRowStride;
            if (maskRowStart)
                maskRowStart += maskRowStride;
        }
    }
};

#endif
