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

#ifndef RGBCOMPOSITEOPCOLOR_H
#define RGBCOMPOSITEOPCOLOR_H

#include "KoColorSpaceMaths.h"
#include "KoColorConversions.h"
#include <KoCompositeOpAlphaBase.h>

#define SCALE_TO_FLOAT( v ) KoColorSpaceMaths< channels_type, float>::scaleToA( v )
#define SCALE_FROM_FLOAT( v ) KoColorSpaceMaths< float, channels_type>::scaleToA( v )

template<class _CSTraits>
class RgbCompositeOpColor : public KoCompositeOpAlphaBase<_CSTraits, RgbCompositeOpColor<_CSTraits>, true >
{
    typedef typename _CSTraits::channels_type channels_type;
    typedef typename KoColorSpaceMathsTraits<typename _CSTraits::channels_type>::compositetype compositetype;

public:

    RgbCompositeOpColor(KoColorSpace * cs)
            : KoCompositeOpAlphaBase<_CSTraits, RgbCompositeOpColor<_CSTraits>, true >(cs, COMPOSITE_COLOR, i18n("Color"), KoCompositeOp::categoryMisc()) {
    }

    inline static channels_type selectAlpha(channels_type srcAlpha, channels_type dstAlpha) {
        return qMin(srcAlpha, dstAlpha);
    }

    inline static void composeColorChannels(channels_type srcBlend,
                                            const channels_type* src,
                                            channels_type* dst,
                                            bool allChannelFlags,
                                            const QBitArray & channelFlags) {

        float dstRed = SCALE_TO_FLOAT(dst[_CSTraits::red_pos]);
        float dstGreen = SCALE_TO_FLOAT(dst[_CSTraits::green_pos]);
        float dstBlue = SCALE_TO_FLOAT(dst[_CSTraits::blue_pos]);

        float srcHue;
        float srcSaturation;
        float srcLightness;
        float dstHue;
        float dstSaturation;
        float dstLightness;

        float srcRed = SCALE_TO_FLOAT(src[_CSTraits::red_pos]);
        float srcGreen = SCALE_TO_FLOAT(src[_CSTraits::green_pos]);
        float srcBlue = SCALE_TO_FLOAT(src[_CSTraits::blue_pos]);

        RGBToHSL(srcRed, srcGreen, srcBlue, &srcHue, &srcSaturation, &srcLightness);
        RGBToHSL(dstRed, dstGreen, dstBlue, &dstHue, &dstSaturation, &dstLightness);
        HSLToRGB(srcHue, srcSaturation, dstLightness, &srcRed, &srcGreen, &srcBlue);

        if (allChannelFlags || channelFlags.testBit(_CSTraits::red_pos))
            dst[_CSTraits::red_pos] = KoColorSpaceMaths<channels_type>::blend(SCALE_FROM_FLOAT(srcRed), SCALE_FROM_FLOAT(dstRed), srcBlend);
        if (allChannelFlags || channelFlags.testBit(_CSTraits::green_pos))
            dst[_CSTraits::green_pos] = KoColorSpaceMaths<channels_type>::blend(SCALE_FROM_FLOAT(srcGreen), SCALE_FROM_FLOAT(dstGreen), srcBlend);
        if (allChannelFlags || channelFlags.testBit(_CSTraits::blue_pos))
            dst[_CSTraits::blue_pos] = KoColorSpaceMaths<channels_type>::blend(SCALE_FROM_FLOAT(srcBlue), SCALE_FROM_FLOAT(dstBlue), srcBlend);
    }
};

#endif
