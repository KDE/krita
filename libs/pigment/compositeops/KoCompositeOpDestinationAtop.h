/*
 *  SPDX-FileCopyrightText: 2016 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *  SPDX-FileCopyrightText: 2012 José Luis Vergara <pentalis@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef _KOCOMPOSITEOPDESTINATIONATOP_H_
#define _KOCOMPOSITEOPDESTINATIONATOP_H_

#include "KoCompositeOpBase.h"

/**
 *  Generic implementation of the Destination-atop composite op, based off the behind composite op.
 *  This is necessary for Open Raster support.
 *  https://www.w3.org/TR/compositing-1/
 */
template<class CS_Traits>
class KoCompositeOpDestinationAtop : public KoCompositeOpBase<CS_Traits, KoCompositeOpDestinationAtop<CS_Traits> >
{
    typedef KoCompositeOpBase<CS_Traits, KoCompositeOpDestinationAtop<CS_Traits> > base_class;
    typedef typename CS_Traits::channels_type channels_type;

    static const qint8 channels_nb = CS_Traits::channels_nb;
    static const qint8 alpha_pos   = CS_Traits::alpha_pos;

public:
    KoCompositeOpDestinationAtop(const KoColorSpace * cs)
        : base_class(cs, COMPOSITE_DESTINATION_ATOP, KoCompositeOp::categoryMix()) { }

public:
    template<bool alphaLocked, bool allChannelFlags>
    inline static channels_type composeColorChannels(const channels_type* src, channels_type srcAlpha,
                                                     channels_type*       dst, channels_type dstAlpha,
                                                     channels_type  maskAlpha, channels_type  opacity,
                                                     const QBitArray& channelFlags                    )  {
        using namespace Arithmetic;

        channels_type appliedAlpha       = mul(maskAlpha, srcAlpha, opacity);

        channels_type newDstAlpha        = appliedAlpha;

        if (dstAlpha != zeroValue<channels_type>() && srcAlpha != zeroValue<channels_type>()) {
            // blend the color channels as if we were painting on the layer below
            for (qint8 channel = 0; channel < channels_nb; ++channel)
                if(channel != alpha_pos && (allChannelFlags || channelFlags.testBit(channel))) {
                    dst[channel] = lerp(src[channel],dst[channel],dstAlpha);
                }
        }
        else if (srcAlpha != zeroValue<channels_type>()) {
            // don't blend if the color of the destination is undefined (has zero opacity)
            // copy the source channel instead
            for (qint8 channel = 0; channel < channels_nb; ++channel)
                if(channel != alpha_pos && (allChannelFlags || channelFlags.testBit(channel)))
                    dst[channel] = src[channel];
        }

        return newDstAlpha;
    }
};

#endif  // _KOCOMPOSITEOPDESTINATIONATOP_H_
