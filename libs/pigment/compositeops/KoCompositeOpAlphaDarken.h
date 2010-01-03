/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KOCOMPOSITEOPALPHADARKEN_H_
#define KOCOMPOSITEOPALPHADARKEN_H_

#include "KoColorSpaceMaths.h"
#include "KoCompositeOp.h"

#define NATIVE_OPACITY_OPAQUE KoColorSpaceMathsTraits<channels_type>::unitValue
#define NATIVE_OPACITY_TRANSPARENT KoColorSpaceMathsTraits<channels_type>::zeroValue

/**
 * A template version of the alphadarken composite operation to use in colorspaces<
 */
template<class _CSTraits>
class KoCompositeOpAlphaDarken : public KoCompositeOp
{
    typedef typename _CSTraits::channels_type channels_type;
    typedef typename KoColorSpaceMathsTraits<typename _CSTraits::channels_type>::compositetype compositetype;

public:

    KoCompositeOpAlphaDarken(const KoColorSpace * cs)
            : KoCompositeOp(cs, COMPOSITE_ALPHA_DARKEN, i18n("Alpha darken"), KoCompositeOp::categoryMix()) {
    }

public:
    using KoCompositeOp::composite;

    void composite(quint8 *dstRowStart,
                   qint32 dststride,
                   const quint8 *srcRowStart,
                   qint32 srcstride,
                   const quint8 *maskRowStart,
                   qint32 maskstride,
                   qint32 rows,
                   qint32 cols,
                   quint8 U8_opacity,
                   const QBitArray & channelFlags) const {
        qint32 srcInc = (srcstride == 0) ? 0 : _CSTraits::channels_nb;

        bool testChannelFlags = !channelFlags.isEmpty();
        while (rows-- > 0) {

            const channels_type *s = reinterpret_cast<const channels_type *>(srcRowStart);
            channels_type *d = reinterpret_cast<channels_type *>(dstRowStart);

            const quint8 *mask = maskRowStart;

            channels_type opacity = KoColorSpaceMaths<quint8, channels_type>::scaleToA(U8_opacity);

            for (qint32 i = cols; i > 0; i--, s += srcInc, d += _CSTraits::channels_nb) {

                channels_type srcAlpha = s[_CSTraits::alpha_pos];
                channels_type dstAlpha = d[_CSTraits::alpha_pos];

                // apply the alphamask
                if (mask != 0) {
                    if (*mask != OPACITY_OPAQUE) {
                        channels_type tmpOpacity = KoColorSpaceMaths<quint8 , channels_type>::scaleToA(*mask);
                        srcAlpha =  KoColorSpaceMaths<channels_type>::multiply(srcAlpha, tmpOpacity);
                    }
                    mask++;
                }

                if (opacity != NATIVE_OPACITY_OPAQUE) {
                    srcAlpha = KoColorSpaceMaths<channels_type>::multiply(srcAlpha, opacity);
                }

                // not transparent
                if (srcAlpha != NATIVE_OPACITY_TRANSPARENT && srcAlpha >= dstAlpha) {
                    if (!testChannelFlags) {
                        for (uint i = 0; i < _CSTraits::channels_nb; i++) {
                            if ((int)i != _CSTraits::alpha_pos) {
                                d[i] = s[i];
                            }
                        }
                    } else {
                        for (uint i = 0; i < _CSTraits::channels_nb; i++) {
                            if ((int)i != _CSTraits::alpha_pos && channelFlags.testBit(i)) {
                                d[i] = s[i];
                            }
                        }
                    }
                    d[_CSTraits::alpha_pos] = srcAlpha;
                }
            }

            srcRowStart += srcstride;
            dstRowStart += dststride;
            if (maskRowStart) {
                maskRowStart += maskstride;
            }
        }
    }
};

#endif
