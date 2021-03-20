/*
 *  SPDX-FileCopyrightText: 2012 José Luis Vergara <pentalis@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef _KOCOMPOSITEOPBEHIND_H_
#define _KOCOMPOSITEOPBEHIND_H_

#include "KoCompositeOpBase.h"

/**
 *  Generic implementation of the Behind composite op, which blends the colors of a foreground layer as if it were in the background instead
 */
template<class CS_Traits>
class KoCompositeOpBehind : public KoCompositeOpBase<CS_Traits, KoCompositeOpBehind<CS_Traits> >
{
    typedef KoCompositeOpBase<CS_Traits, KoCompositeOpBehind<CS_Traits> > base_class;
    typedef typename CS_Traits::channels_type channels_type;

    static const qint8 channels_nb = CS_Traits::channels_nb;
    static const qint8 alpha_pos   = CS_Traits::alpha_pos;

public:
    KoCompositeOpBehind(const KoColorSpace * cs)
        : base_class(cs, COMPOSITE_BEHIND, i18n("Behind"), KoCompositeOp::categoryMix()) { }

public:
    template<bool alphaLocked, bool allChannelFlags>
    inline static channels_type composeColorChannels(const channels_type* src, channels_type srcAlpha,
                                                     channels_type*       dst, channels_type dstAlpha,
                                                     channels_type  maskAlpha, channels_type  opacity,
                                                     const QBitArray& channelFlags                    )  {
        using namespace Arithmetic;
                
        if (dstAlpha     == unitValue<channels_type>()) return dstAlpha;
        channels_type appliedAlpha       = mul(maskAlpha, srcAlpha, opacity);
        
        if (appliedAlpha == zeroValue<channels_type>()) return dstAlpha;
        channels_type newDstAlpha        = unionShapeOpacity(dstAlpha, appliedAlpha);
        
        if (dstAlpha != zeroValue<channels_type>()) {
            // blend the color channels as if we were painting on the layer below
            for (qint8 channel = 0; channel < channels_nb; ++channel)
                if(channel != alpha_pos && (allChannelFlags || channelFlags.testBit(channel))) {
                    /*each color blended in proportion to their calculated opacity*/
                    channels_type srcMult= mul(src[channel], appliedAlpha);
                    channels_type blendedValue = lerp(srcMult,dst[channel],dstAlpha);
                    dst[channel] = KoColorSpaceMaths<channels_type>::divide(blendedValue,newDstAlpha);
                }
        }
        else {
            // don't blend if the color of the destination is undefined (has zero opacity)
            // copy the source channel instead
            for (qint8 channel = 0; channel < channels_nb; ++channel)
                if(channel != alpha_pos && (allChannelFlags || channelFlags.testBit(channel)))
                    dst[channel] = src[channel];
        }

        return newDstAlpha;
    }
};

#endif  // _KOCOMPOSITEOPBEHIND_H_
