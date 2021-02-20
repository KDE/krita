/*
 * SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
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
