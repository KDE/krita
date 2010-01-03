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

#ifndef _KOCOMPOSITEOPSCREEN_H_
#define _KOCOMPOSITEOPSCREEN_H_

#include "KoCompositeOpAlphaBase.h"

/**
 * A template version of the burn composite operation to use in colorspaces.
 */
template<class _CSTraits>
class KoCompositeOpScreen : public KoCompositeOpAlphaBase<_CSTraits, KoCompositeOpScreen<_CSTraits> >
{
    typedef typename _CSTraits::channels_type channels_type;
    typedef typename KoColorSpaceMathsTraits<typename _CSTraits::channels_type>::compositetype compositetype;
public:

    KoCompositeOpScreen(const KoColorSpace * cs)
            : KoCompositeOpAlphaBase<_CSTraits, KoCompositeOpScreen<_CSTraits> >(cs, COMPOSITE_SCREEN, i18n("Screen"), KoCompositeOp::categoryArithmetic()) {
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
            if ((int)channel != _CSTraits::alpha_pos && (channelFlags.isEmpty() || channelFlags.testBit(channel))) {
                compositetype srcChannel = src[channel];
                compositetype dstChannel = dst[channel];

                srcChannel = NATIVE_MAX_VALUE - KoColorSpaceMaths<channels_type>::multiply(NATIVE_MAX_VALUE - dstChannel, NATIVE_MAX_VALUE - srcChannel);

                channels_type newChannel = KoColorSpaceMaths<channels_type>::blend(srcChannel, dstChannel, srcBlend);

                dst[channel] = newChannel;
            }
        }
    }


};

#endif
