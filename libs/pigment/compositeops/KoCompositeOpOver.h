/*
 *  SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef _KOCOMPOSITEOPOVER_H_
#define _KOCOMPOSITEOPOVER_H_

#include "KoCompositeOpAlphaBase.h"
#include <KoCompositeOpRegistry.h>
#include <klocalizedstring.h>

template<class _CSTraits, int channel>
struct KoCompositeOpOverCompositor {
    typedef typename _CSTraits::channels_type channels_type;
    inline static void composeColorChannels(channels_type srcBlend,
                                            const channels_type* srcN,
                                            channels_type* dstN,
                                            bool allChannelFlags,
                                            const QBitArray & channelFlags) {
        if (channel != _CSTraits::alpha_pos && (allChannelFlags || channelFlags.testBit(channel)))
            dstN[channel] = KoColorSpaceMaths<channels_type>::blend(srcN[channel], dstN[channel], srcBlend);
        KoCompositeOpOverCompositor<_CSTraits, channel - 1>::composeColorChannels(srcBlend, srcN, dstN, allChannelFlags, channelFlags);
    }
};

template<class _CSTraits>
struct KoCompositeOpOverCompositor<_CSTraits, -1> {
    typedef typename _CSTraits::channels_type channels_type;
    inline static void composeColorChannels(channels_type /*srcBlend*/,
                                            const channels_type* /*srcN*/,
                                            channels_type* /*dstN*/,
                                            bool /*allChannelFlags*/,
                                            const QBitArray & /*channelFlags*/) {
    }
};

/**
 * A template version of the over composite operation to use in colorspaces.
 */
template<class _CSTraits>
class KoCompositeOpOver : public KoCompositeOpAlphaBase<_CSTraits, KoCompositeOpOver<_CSTraits>, false >
{
    typedef typename _CSTraits::channels_type channels_type;
public:

    KoCompositeOpOver(const KoColorSpace * cs)
            : KoCompositeOpAlphaBase<_CSTraits, KoCompositeOpOver<_CSTraits>, false >(cs, COMPOSITE_OVER, KoCompositeOp::categoryMix()) {
    }

public:
    inline static channels_type selectAlpha(channels_type srcAlpha, channels_type dstAlpha) {
        Q_UNUSED(dstAlpha);
        return srcAlpha;
    }

public:
   inline static void composeColorChannels(channels_type srcBlend,
                                            const channels_type* srcN,
                                            channels_type* dstN,
                                            bool allChannelFlags,
                                            const QBitArray & channelFlags) {
        if (srcBlend == NATIVE_OPACITY_OPAQUE) {
            for (int i = 0; (uint)i <  _CSTraits::channels_nb; i++) {
                if (i != _CSTraits::alpha_pos && (allChannelFlags || channelFlags.testBit(i)))
                    dstN[i] = srcN[i];
            }
        } else {
            KoCompositeOpOverCompositor<_CSTraits, _CSTraits::channels_nb-1>::composeColorChannels(srcBlend, srcN, dstN, allChannelFlags, channelFlags);
        }
    }

};

#endif
