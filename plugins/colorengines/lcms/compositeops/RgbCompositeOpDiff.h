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

#ifndef RGBCOMPOSITEOPDIFF_H
#define RGBCOMPOSITEOPDIFF_H

#include "KoColorSpaceMaths.h"
#include <KoCompositeOp.h>

#define AbsoluteValue(x)  ((x) < 0 ? -(x) : (x))

template<class _CSTraits>
class RgbCompositeOpDiff : public KoCompositeOp
{
    typedef typename _CSTraits::channels_type channels_type;
    typedef typename KoColorSpaceMathsTraits<typename _CSTraits::channels_type>::compositetype compositetype;

public:

    RgbCompositeOpDiff(KoColorSpace * cs, const bool userVisible = true)
            : KoCompositeOp(cs, COMPOSITE_DIFF, i18n("Diff"), "", userVisible) {
    }

    using KoCompositeOp::composite;

    void composite(quint8 *dstRowStart, qint32 dstRowStride,
                   const quint8 *srcRowStart, qint32 srcRowStride,
                   const quint8 *maskRowStart, qint32 maskRowStride,
                   qint32 rows, qint32 numColumns,
                   quint8 opacity,
                   const QBitArray & channelFlags) const {
        Q_UNUSED(maskRowStart);
        Q_UNUSED(maskRowStride);

        if (opacity == OPACITY_TRANSPARENT_U8)
            return;

        channels_type *d;
        const channels_type *s;

        qint32 i;

        qreal sAlpha, dAlpha;

        while (rows-- > 0) {
            d = reinterpret_cast<channels_type *>(dstRowStart);
            s = reinterpret_cast<const channels_type *>(srcRowStart);
            for (i = numColumns; i > 0; i--, d += _CSTraits::channels_nb, s += _CSTraits::channels_nb) {
                sAlpha = NATIVE_OPACITY_OPAQUE - s[_CSTraits::alpha_pos];
                dAlpha = NATIVE_OPACITY_OPAQUE - d[_CSTraits::alpha_pos];

                if (channelFlags.isEmpty() || channelFlags.testBit(_CSTraits::red_pos))
                    d[_CSTraits::red_pos] = (channels_type)
                                            AbsoluteValue(s[_CSTraits::red_pos] - (qreal) d[_CSTraits::red_pos]);

                if (channelFlags.isEmpty() || channelFlags.testBit(_CSTraits::green_pos))
                    d[_CSTraits::green_pos] = (channels_type)
                                              AbsoluteValue(s[_CSTraits::green_pos] - (qreal) d[_CSTraits::green_pos]);


                if (channelFlags.isEmpty() || channelFlags.testBit(_CSTraits::blue_pos))
                    d[_CSTraits::blue_pos] = (channels_type)
                                             AbsoluteValue(s[_CSTraits::blue_pos] - (qreal) d[_CSTraits::blue_pos]);

                if (channelFlags.isEmpty() || channelFlags.testBit(_CSTraits::alpha_pos))
                    d[_CSTraits::alpha_pos] = NATIVE_OPACITY_OPAQUE - (channels_type)
                                              AbsoluteValue(sAlpha - (qreal) dAlpha);

            }
            dstRowStart += dstRowStride;
            srcRowStart += srcRowStride;

        }
    }

};

#endif
