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

#ifndef RGBCOMPOSITEOPBUMPMAP_H
#define RGBCOMPOSITEOPBUMPMAP_H

#include "KoColorSpaceMaths.h"
#include <KoCompositeOpAlphaBase.h>

template<class _CSTraits>
class RgbCompositeOpBumpmap : public KoCompositeOpAlphaBase<_CSTraits, RgbCompositeOpBumpmap<_CSTraits>, true >
{
    typedef typename _CSTraits::channels_type channels_type;
    typedef typename KoColorSpaceMathsTraits<typename _CSTraits::channels_type>::compositetype compositetype;

public:

    RgbCompositeOpBumpmap(KoColorSpace * cs)
            : KoCompositeOpAlphaBase<_CSTraits, RgbCompositeOpBumpmap<_CSTraits>, true >(cs, COMPOSITE_BUMPMAP, i18n("Bumpmap"),  KoCompositeOp::categoryMisc()) {
    }                       
                       
    inline static channels_type selectAlpha(channels_type srcAlpha, channels_type dstAlpha) {
        return qMin(srcAlpha, dstAlpha);
    }

    inline static void composeColorChannels(channels_type srcBlend,
                                            const channels_type* src,
                                            channels_type* dst,
                                            bool allChannelFlags,
                                            const QBitArray & channelFlags) {
        qreal intensity;
        
        // And I'm not sure whether this is correct, either.
        intensity = ((qreal)306.0 * src[_CSTraits::red_pos] +
                        (qreal)601.0 * src[_CSTraits::green_pos] +
                        (qreal)117.0 * src[_CSTraits::blue_pos]) / 1024.0;
                        
        for (uint i = 0; i < _CSTraits::channels_nb; i++) {
            if ((int)i != _CSTraits::alpha_pos  && (allChannelFlags || channelFlags.testBit(i))) {
                channels_type srcChannel = (channels_type)(((qreal)
                                            intensity * dst[i]) / NATIVE_OPACITY_OPAQUE + 0.5);
                channels_type dstChannel = dst[i];

                dst[i] = KoColorSpaceMaths<channels_type>::blend(srcChannel, dstChannel, srcBlend);
            }
        }
    }

};

#endif
