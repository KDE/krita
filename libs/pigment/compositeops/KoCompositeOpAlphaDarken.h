/*
 * Copyright (c) 2006 Cyrille Berger  <cberger@cberger.net>
 * Copyright (c) 2011 Silvio Heinrich <plassy@web.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KOCOMPOSITEOPALPHADARKEN_H_
#define KOCOMPOSITEOPALPHADARKEN_H_

#include "KoCompositeOpFunctions.h"
#include "KoCompositeOpBase.h"

/**
 * A template version of the alphadarken composite operation to use in colorspaces<
 */
template<class Traits>
class KoCompositeOpAlphaDarken: public KoCompositeOpBase< Traits, KoCompositeOpAlphaDarken<Traits> >
{
    typedef KoCompositeOpBase< Traits, KoCompositeOpAlphaDarken<Traits> > base_class;
    typedef typename Traits::channels_type                                channels_type;
    
    static const qint32 channels_nb = Traits::channels_nb;
    static const qint32 alpha_pos   = Traits::alpha_pos;
    
public:
    KoCompositeOpAlphaDarken(const KoColorSpace* cs):
        base_class(cs, COMPOSITE_ALPHA_DARKEN, i18n("Alpha darken"), KoCompositeOp::categoryMix(), false) { }
    
public:
    template<bool alphaLocked, bool allChannelFlags>
    inline static channels_type composeColorChannels(const channels_type* src, channels_type srcAlpha,
                                                     channels_type*       dst, channels_type dstAlpha,
                                                     channels_type opacity, channels_type flow, const QBitArray& channelFlags) {
        using namespace Arithmetic;
        
        srcAlpha = mul(srcAlpha, opacity);
        
        if(dstAlpha != zeroValue<channels_type>()) {
            for(qint32 i=0; i <channels_nb; i++) {
                if(i != alpha_pos && (allChannelFlags || channelFlags.testBit(i)))
                    dst[i] = lerp(dst[i], src[i], srcAlpha);
            }
        }
        else {
            for(qint32 i=0; i <channels_nb; i++) {
                if(i != alpha_pos && (allChannelFlags || channelFlags.testBit(i)))
                    dst[i] = src[i];
            }
        }
        
        channels_type alpha1 = unionShapeOpacity(srcAlpha, dstAlpha);       // alpha with 0% flow
        channels_type alpha2 = (dstAlpha > srcAlpha) ? dstAlpha : srcAlpha; // alpha with 100% flow
        return lerp(alpha1, alpha2, flow);
    }
};

#endif // KOCOMPOSITEOPALPHADARKEN_H_
