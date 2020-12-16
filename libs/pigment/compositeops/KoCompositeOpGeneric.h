/*
 * SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef _KOCOMPOSITEOP_GENERIC_H_
#define _KOCOMPOSITEOP_GENERIC_H_

#include "KoCompositeOpFunctions.h"
#include "KoCompositeOpBase.h"

/**
 * Generic CompositeOp for separable channel compositing functions
 * 
 * A template to generate a KoCompositeOp class by just specifying a
 * blending/compositing function. This template works with compositing functions
 * for separable channels (means each channel of a pixel can be processed separately)
 */
template<
    class Traits,
    typename Traits::channels_type compositeFunc(typename Traits::channels_type, typename Traits::channels_type)
>
class KoCompositeOpGenericSC: public KoCompositeOpBase< Traits, KoCompositeOpGenericSC<Traits,compositeFunc> >
{
    typedef KoCompositeOpBase< Traits, KoCompositeOpGenericSC<Traits,compositeFunc> > base_class;
    typedef typename Traits::channels_type                                            channels_type;
    
    static const qint32 channels_nb = Traits::channels_nb;
    static const qint32 alpha_pos   = Traits::alpha_pos;
    
public:
    KoCompositeOpGenericSC(const KoColorSpace* cs, const QString& id, const QString& description, const QString& category)
        : base_class(cs, id, description, category) { }

public:
    template<bool alphaLocked, bool allChannelFlags>
    inline static channels_type composeColorChannels(const channels_type* src, channels_type srcAlpha,
                                                     channels_type*       dst, channels_type dstAlpha, channels_type maskAlpha,
                                                     channels_type opacity, const QBitArray& channelFlags) {
        using namespace Arithmetic;
        
        srcAlpha = mul(srcAlpha, maskAlpha, opacity);
        
        if(alphaLocked) {
            if(dstAlpha != zeroValue<channels_type>()) {
                for(qint32 i=0; i <channels_nb; i++) {
                    if(i != alpha_pos && (allChannelFlags || channelFlags.testBit(i)))
                        dst[i] = lerp(dst[i], compositeFunc(src[i],dst[i]), srcAlpha);
                }
            }
            
            return dstAlpha;
        }
        else {
            channels_type newDstAlpha = unionShapeOpacity(srcAlpha, dstAlpha);
            
            if(newDstAlpha != zeroValue<channels_type>()) {
                for(qint32 i=0; i <channels_nb; i++) {
                    if(i != alpha_pos && (allChannelFlags || channelFlags.testBit(i))) {
                        channels_type result = blend(src[i], srcAlpha, dst[i], dstAlpha, compositeFunc(src[i],dst[i]));
                        dst[i] = div(result, newDstAlpha);
                    }
                }
            }
            
            return newDstAlpha;
        }
    }
};


/**
 * Generic CompositeOp for nonseparable/HSL channel compositing functions
 * 
 * A template to generate a KoCompositeOp class by just specifying a
 * blending/compositing function. This template works with compositing functions
 * for RGB channels only (the channels can not be processed separately)
 */
template<class Traits, void compositeFunc(float, float, float, float&, float&, float&)>
class KoCompositeOpGenericHSL: public KoCompositeOpBase< Traits, KoCompositeOpGenericHSL<Traits,compositeFunc> >
{
    typedef KoCompositeOpBase< Traits, KoCompositeOpGenericHSL<Traits,compositeFunc> > base_class;
    typedef typename Traits::channels_type                                             channels_type;
    
    static const qint32 red_pos   = Traits::red_pos;
    static const qint32 green_pos = Traits::green_pos;
    static const qint32 blue_pos  = Traits::blue_pos;
    
public:
    KoCompositeOpGenericHSL(const KoColorSpace* cs, const QString& id, const QString& description, const QString& category)
        : base_class(cs, id, description, category) { }
    
public:
    template<bool alphaLocked, bool allChannelFlags>
    inline static channels_type composeColorChannels(const channels_type* src, channels_type srcAlpha,
                                                     channels_type*       dst, channels_type dstAlpha, channels_type maskAlpha,
                                                     channels_type opacity, const QBitArray& channelFlags) {
        using namespace Arithmetic;

        srcAlpha = mul(srcAlpha, maskAlpha, opacity);

        if(alphaLocked) {
            if(dstAlpha != zeroValue<channels_type>()) {
                float srcR = scale<float>(src[red_pos]);
                float srcG = scale<float>(src[green_pos]);
                float srcB = scale<float>(src[blue_pos]);

                float dstR = scale<float>(dst[red_pos]);
                float dstG = scale<float>(dst[green_pos]);
                float dstB = scale<float>(dst[blue_pos]);

                compositeFunc(srcR, srcG, srcB, dstR, dstG, dstB);

                if(allChannelFlags || channelFlags.testBit(red_pos))
                    dst[red_pos] = lerp(dst[red_pos], scale<channels_type>(dstR), srcAlpha);

                if(allChannelFlags || channelFlags.testBit(green_pos))
                    dst[green_pos] = lerp(dst[green_pos], scale<channels_type>(dstG), srcAlpha);

                if(allChannelFlags || channelFlags.testBit(blue_pos))
                    dst[blue_pos] = lerp(dst[blue_pos], scale<channels_type>(dstB), srcAlpha);
            }

            return dstAlpha;
        }
        else {
            channels_type newDstAlpha = unionShapeOpacity(srcAlpha, dstAlpha);

            if(newDstAlpha != zeroValue<channels_type>()) {
                float srcR = scale<float>(src[red_pos]);
                float srcG = scale<float>(src[green_pos]);
                float srcB = scale<float>(src[blue_pos]);

                float dstR = scale<float>(dst[red_pos]);
                float dstG = scale<float>(dst[green_pos]);
                float dstB = scale<float>(dst[blue_pos]);

                compositeFunc(srcR, srcG, srcB, dstR, dstG, dstB);

                if(allChannelFlags || channelFlags.testBit(red_pos))
                    dst[red_pos] = div(blend(src[red_pos], srcAlpha, dst[red_pos], dstAlpha, scale<channels_type>(dstR)), newDstAlpha);

                if(allChannelFlags || channelFlags.testBit(green_pos))
                    dst[green_pos] = div(blend(src[green_pos], srcAlpha, dst[green_pos], dstAlpha, scale<channels_type>(dstG)), newDstAlpha);

                if(allChannelFlags || channelFlags.testBit(blue_pos))
                    dst[blue_pos] = div(blend(src[blue_pos], srcAlpha, dst[blue_pos], dstAlpha, scale<channels_type>(dstB)), newDstAlpha);
            }

            return newDstAlpha;
        }
    }
};



/**
 * Generic CompositeOp for separable channel + alpha compositing functions
 *
 * A template to generate a KoCompositeOp class by just specifying a
 * blending/compositing function. This template works with compositing functions
 * for separable channels (means each channel of a pixel can be processed separately)
 * with taking apha into consideration.
 * Note that because of special treating of alpha, any composite op function
 * needs to make alpha blending itself - the value of color that is written onto the projection
 * is the same that the composite function gives (compare with KoCompositeOpGenericHSL and KoCompositeOpGenericSC).
 */
template<class Traits, void compositeFunc(float, float, float&, float&)>
class KoCompositeOpGenericSCAlpha: public KoCompositeOpBase< Traits, KoCompositeOpGenericSCAlpha<Traits,compositeFunc> >
{
    typedef KoCompositeOpBase< Traits, KoCompositeOpGenericSCAlpha<Traits,compositeFunc> > base_class;
    typedef typename Traits::channels_type                                             channels_type;

    static const qint32 channels_nb = Traits::channels_nb;
    static const qint32 alpha_pos  = Traits::alpha_pos;

public:
    KoCompositeOpGenericSCAlpha(const KoColorSpace* cs, const QString& id, const QString& description, const QString& category)
        : base_class(cs, id, description, category) { }

public:
    template<bool alphaLocked, bool allChannelFlags>
    inline static channels_type composeColorChannels(const channels_type* src, channels_type srcAlpha,
                                                     channels_type*       dst, channels_type dstAlpha, channels_type maskAlpha,
                                                     channels_type opacity, const QBitArray& channelFlags)
    {
        using namespace Arithmetic;

        srcAlpha = mul(srcAlpha, maskAlpha, opacity);

        if(alphaLocked) {
            channels_type oldAlpha = dstAlpha;
            if(dstAlpha != zeroValue<channels_type>()) {
                for(qint32 i=0; i <channels_nb; i++) {
                    if(i != alpha_pos && (allChannelFlags || channelFlags.testBit(i))) {
                        float dstValueFloat = scale<float>(dst[i]);
                        float dstAlphaFloat = scale<float>(oldAlpha);
                        compositeFunc(scale<float>(src[i]), scale<float>(srcAlpha), dstValueFloat, dstAlphaFloat);
                        dst[i] = scale<channels_type>(dstValueFloat);
                    }
                }
            }

            return dstAlpha;
        }
        else {
            channels_type oldAlpha = dstAlpha;
            channels_type newDstAlpha = unionShapeOpacity(srcAlpha, dstAlpha);

            if(newDstAlpha != zeroValue<channels_type>()) {
                for(qint32 i=0; i <channels_nb; i++) {
                    if(i != alpha_pos && (allChannelFlags || channelFlags.testBit(i))) {
                        float dstFloat = scale<float>(dst[i]);
                        float dstAlphaFloat = scale<float>(oldAlpha);
                        compositeFunc(scale<float>(src[i]), scale<float>(srcAlpha), dstFloat, dstAlphaFloat);
                        dst[i] = scale<channels_type>(dstFloat);
                    }
                }
            }

            return newDstAlpha;
        }
    }
};



#endif // _KOCOMPOSITEOP_GENERIC_H_
