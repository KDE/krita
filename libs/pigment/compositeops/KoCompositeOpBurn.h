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

#ifndef KOCOMPOSITEOPBURN_H_
#define KOCOMPOSITEOPBURN_H_

#include "KoCompositeOpAlphaBase.h"

/**
 * A template version of the burn composite operation to use in colorspaces.
 */
template<class _CSTraits>
class KoCompositeOpBurn : public KoCompositeOpAlphaBase<_CSTraits, KoCompositeOpBurn<_CSTraits>, true >
{
    typedef typename _CSTraits::channels_type channels_type;
    typedef typename KoColorSpaceMathsTraits<channels_type>::compositetype composite_type;
    
public:

    KoCompositeOpBurn(const KoColorSpace * cs)
            : KoCompositeOpAlphaBase<_CSTraits, KoCompositeOpBurn<_CSTraits>, true >(cs, COMPOSITE_BURN, i18n("Burn"), KoCompositeOp::categoryLight()) {
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
            if ((int)i != _CSTraits::alpha_pos && (allChannelFlags || channelFlags.testBit(i))) {
                composite_type unitValue = KoColorSpaceMathsTraits<channels_type>::unitValue;
                composite_type invDst    = unitValue - dst[i];
                
                if(src[i] != KoColorSpaceMathsTraits<channels_type>::zeroValue) {
                    composite_type result = unitValue - qMin<composite_type>(invDst * unitValue / src[i], unitValue);
                    dst[i] = KoColorSpaceMaths<channels_type>::blend(result, dst[i], srcBlend);
                }
                else {
                    //composite_type result = KoColorSpaceMathsTraits<channels_type>::zeroValue;
                    composite_type result = unitValue - qMin<composite_type>(invDst * unitValue, unitValue);
                    dst[i] = KoColorSpaceMaths<channels_type>::blend(result, dst[i], srcBlend);
                }
            }
        }
    }

};

#endif
