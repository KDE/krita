/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef _KOCOMPOSITEOPOVERLAY_H_
#define _KOCOMPOSITEOPOVERLAY_H_

#include "KoCompositeOpAlphaBase.h"

/**
 * A template version of the burn composite operation to use in colorspaces.
 */
template<class _CSTraits>
class KoCompositeOpOverlay : public KoCompositeOpAlphaBase<_CSTraits, KoCompositeOpOverlay<_CSTraits> >
{
    typedef typename _CSTraits::channels_type channels_type;
    typedef typename KoColorSpaceMathsTraits<typename _CSTraits::channels_type>::compositetype compositetype;
public:

    KoCompositeOpOverlay(const KoColorSpace * cs)
            : KoCompositeOpAlphaBase<_CSTraits, KoCompositeOpOverlay<_CSTraits> >(cs, COMPOSITE_OVERLAY, i18n("Overlay"), KoCompositeOp::categoryArithmetic()) {
    }

public:
    inline static channels_type selectAlpha(channels_type srcAlpha, channels_type dstAlpha) {
        return qMin(srcAlpha, dstAlpha);
    }

    inline static void composeColorChannels(channels_type srcBlend,
                                            const channels_type* src,
                                            channels_type* dst,
                                            qint32 pixelSize,
                                            const QBitArray & channelFlags) {
        Q_UNUSED(pixelSize);
        for (uint channel = 0; channel < _CSTraits::channels_nb; channel++) {
            if ((int)channel != _CSTraits::alpha_pos  && (channelFlags.isEmpty() || channelFlags.testBit(channel))) {
                compositetype srcColor = src[channel];
                compositetype dstColor = dst[channel];

//                     srcColor = UINT8_MULT(dstColor, dstColor + UINT8_MULT(2 * srcColor, UINT8_MAX - dstColor));
//                     KoColorSpaceMaths<channels_type>::multiply
                srcColor = KoColorSpaceMaths<channels_type>::multiply(dstColor, dstColor + KoColorSpaceMaths<channels_type>::multiply(2 * srcColor, NATIVE_MAX_VALUE - dstColor));

                channels_type newColor = KoColorSpaceMaths<channels_type>::blend(srcColor, dstColor, srcBlend);

                dst[channel] = newColor;
            }
        }
    }


};

#endif
