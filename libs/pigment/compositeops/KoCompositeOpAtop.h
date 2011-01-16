/*
 * Copyright (c) 2011 Silvio Heinrich <plassy@web.de>
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

#ifndef _KOCOMPOSITEO_ATOP_H_
#define _KOCOMPOSITEO_ATOP_H_

#include "KoCompositeOpFunctions.h"
#include "KoCompositeOpBase.h"

template<class Traits>
class KoCompositeOpAtop: public KoCompositeOpBase< Traits, KoCompositeOpAtop<Traits> >
{
    typedef KoCompositeOpBase< Traits, KoCompositeOpAtop<Traits> > base_class;
    typedef typename Traits::channels_type                         channels_type;
    
    static const qint32 channels_nb = Traits::channels_nb;
    static const qint32 alpha_pos   = Traits::alpha_pos;
    
public:
    KoCompositeOpAtop(const KoColorSpace* cs, const QString& category, bool userVisible=true)
        : base_class(cs, COMPOSITE_ATOP, i18n("Atop"), category, userVisible) { }

public:
    template<bool alphaLocked, bool allChannelFlags>
    inline static channels_type composeColorChannels(const channels_type* src, channels_type srcAlpha,
                                                     channels_type*       dst, channels_type dstAlpha,
                                                     channels_type opacity, const QBitArray& channelFlags) {
        srcAlpha = mul(srcAlpha, opacity);
        
        if(dstAlpha != KoColorSpaceMathsTraits<channels_type>::zeroValue) {
            for(qint32 i=0; i <channels_nb; i++) {
                if(i != alpha_pos && (allChannelFlags || channelFlags.testBit(i)))
                    dst[i] = lerp(dst[i], src[i], srcAlpha);
            }
        }
        
        return dstAlpha;
    }
};

#endif // _KOCOMPOSITEO_ATOP_H_
