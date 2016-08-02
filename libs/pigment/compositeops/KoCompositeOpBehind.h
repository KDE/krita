/*
 *  Copyright (c) 2012 José Luis Vergara <pentalis@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
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
