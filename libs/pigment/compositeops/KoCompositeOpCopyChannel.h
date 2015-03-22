/*
 * Copyright (c) 2011 Silvio Heinrich <plassy@web.de>
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

#ifndef _KOCOMPOSITEOP_COPY_CHANNEL_H_
#define _KOCOMPOSITEOP_COPY_CHANNEL_H_

#include "KoCompositeOpBase.h"

/**
 * KoCompositeOpCopyChannel class
 * 
 * This class creates a CompositeOp that will just copy/blend
 * a source channel to a destination channel
 * 
 * @param channel_pos the channel to copy/blend
 */
template<class Traits, qint32 channel_pos>
class KoCompositeOpCopyChannel: public KoCompositeOpBase< Traits, KoCompositeOpCopyChannel<Traits,channel_pos> >
{
    typedef KoCompositeOpBase< Traits, KoCompositeOpCopyChannel<Traits,channel_pos> > base_class;
    typedef typename Traits::channels_type                                            channels_type;
    typedef typename KoColorSpaceMathsTraits<channels_type>::compositetype            composite_type;
    
    static const qint32 alpha_pos = Traits::alpha_pos;
    
public:
    KoCompositeOpCopyChannel(const KoColorSpace* cs, const QString& id, const QString& description, const QString& category)
        : base_class(cs, id, description, category) { }

public:
    template<bool alphaLocked, bool allChannelFlags>
    inline static channels_type composeColorChannels(const channels_type* src, channels_type srcAlpha,
                                                     channels_type*       dst, channels_type dstAlpha, channels_type maskAlpha,
                                                     channels_type opacity, const QBitArray& channelFlags) {
        using namespace Arithmetic;
        opacity = mul(opacity, maskAlpha);
        
        if(allChannelFlags || channelFlags.testBit(channel_pos)) {
            if(channel_pos == alpha_pos)
                return lerp(dstAlpha, srcAlpha, opacity);
            
            srcAlpha = mul(srcAlpha, opacity);
            dst[channel_pos] = lerp(dst[channel_pos], src[channel_pos], srcAlpha);
        }
        
        return dstAlpha;
    }
};

#endif // _KOCOMPOSITEOP_COPY_CHANNEL_H_
