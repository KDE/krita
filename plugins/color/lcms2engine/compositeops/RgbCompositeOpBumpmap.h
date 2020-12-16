/*
 *  SPDX-FileCopyrightText: 2006 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef RGBCOMPOSITEOPBUMPMAP_H
#define RGBCOMPOSITEOPBUMPMAP_H

#include <KoCompositeOpAlphaBase.h>

template<class _CSTraits>
class RgbCompositeOpBumpmap : public KoCompositeOpAlphaBase<_CSTraits, RgbCompositeOpBumpmap<_CSTraits>, true >
{
    typedef typename _CSTraits::channels_type channels_type;
    typedef typename KoColorSpaceMathsTraits<typename _CSTraits::channels_type>::compositetype compositetype;

public:

    RgbCompositeOpBumpmap(KoColorSpace *cs)
        : KoCompositeOpAlphaBase<_CSTraits, RgbCompositeOpBumpmap<_CSTraits>, true >(cs, COMPOSITE_BUMPMAP, i18n("Bumpmap"),  KoCompositeOp::categoryMisc())
    {
    }

    inline static channels_type selectAlpha(channels_type srcAlpha, channels_type dstAlpha)
    {
        return qMin(srcAlpha, dstAlpha);
    }

    inline static void composeColorChannels(channels_type srcBlend,
                                            const channels_type *src,
                                            channels_type *dst,
                                            bool allChannelFlags,
                                            const QBitArray &channelFlags)
    {
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
