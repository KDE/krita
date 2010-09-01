/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2010 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free hardware; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Hardware Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Hardware Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KOCOMPOSITEOPHARDLIGHT_H_
#define KOCOMPOSITEOPHARDLIGHT_H_

#include "KoCompositeOpAlphaBase.h"

/**
 * A template version of the hard light composite operation to use in colorspaces.
 */
template<class _CSTraits>
class KoCompositeOpHardlight : public KoCompositeOpAlphaBase<_CSTraits, KoCompositeOpHardlight<_CSTraits>, true >
{
    typedef typename _CSTraits::channels_type channels_type;
public:

    KoCompositeOpHardlight(const KoColorSpace * cs)
            : KoCompositeOpAlphaBase<_CSTraits, KoCompositeOpHardlight<_CSTraits>, true >(cs, COMPOSITE_HARD_LIGHT, i18n("Hardlight"), KoCompositeOp::categoryLight()) {
    }

public:
    inline static channels_type selectAlpha(channels_type srcAlpha, channels_type dstAlpha) {
        return qMin(srcAlpha, dstAlpha);
    }

    inline static void composeColorChannels(channels_type srcBlend,
                                            const channels_type* src,
                                            channels_type* dst,
                                            bool allChannelFlags,
                                            const QBitArray & channelFlags) {
        for (uint i = 0; i < _CSTraits::channels_nb; i++) {
            if ((int)i != _CSTraits::alpha_pos  && (allChannelFlags || channelFlags.testBit(i))) {

                channels_type srcChannel = src[i];
                channels_type dstChannel = dst[i];

                channels_type multiplied = KoColorSpaceMaths<channels_type>::multiply(srcChannel, dstChannel);
                channels_type screen = NATIVE_MAX_VALUE - KoColorSpaceMaths<channels_type>::multiply(NATIVE_MAX_VALUE - dstChannel, NATIVE_MAX_VALUE - srcChannel);

                channels_type combination = KoColorSpaceMaths<channels_type>::multiply((255 - srcChannel), multiplied) + KoColorSpaceMaths<channels_type>::multiply(srcChannel, screen);
                dst[i] = KoColorSpaceMaths<channels_type>::blend(combination, dstChannel, srcBlend);

            }
        }
    }

};

#endif
