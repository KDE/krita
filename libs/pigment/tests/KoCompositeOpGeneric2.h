/*
 * SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifdef _KOCOMPOSITEOP_GENERIC_H_
#error "ssksjdkjs"
#endif


#ifndef _KOCOMPOSITEOP_GENERIC_H_
#define _KOCOMPOSITEOP_GENERIC_H_

#include "KoCompositeOpBase.h"

namespace tmp2 {
template <typename T>
Q_REQUIRED_RESULT static inline Q_DECL_UNUSED  bool isZeroValue(T v)
{
    return KoColorSpaceMaths<T>::isZeroValue(v);
}

template <typename T>
Q_REQUIRED_RESULT static inline Q_DECL_UNUSED  T clampChannelToSDRBottom(T v)
{
    return v;
}

template <>
Q_REQUIRED_RESULT  inline Q_DECL_UNUSED  float clampChannelToSDRBottom<float>(float v)
{
    return qMax<float>(KoColorSpaceMathsTraits<float>::zeroValue, v);
}

template <typename T>
Q_REQUIRED_RESULT static inline Q_DECL_UNUSED  bool isUnitValue(T v)
{
    return KoColorSpaceMaths<T>::isUnitValue(v);
}

}

template <class Traits,
          typename Traits::channels_type compositeFunc(typename Traits::channels_type, typename Traits::channels_type)>
struct CompositeFunctionWrapper
{
    using channels_type = typename Traits::channels_type;

    static inline channels_type composeChannel(channels_type src, channels_type dst) {
        return compositeFunc(src, dst);
    }

    static inline channels_type clampSourceChannelValue(channels_type value) {
        return value;
    }

    static inline channels_type clampDestinationChannelValue(channels_type value) {
        return value;
    }
};

template <typename T>
struct ClampedSourceCompositeFunctorBase
{
    static inline T clampSourceChannelValue(T value) {
        using namespace Arithmetic;
        return clampChannelToSDR(value);
    }

    static inline T clampDestinationChannelValue(T value) {
        return value;
    }
};

template <typename T>
struct ClampedSourceAndDestinationBottomCompositeFunctorBase
{
    static inline T clampSourceChannelValue(T value) {
        using namespace Arithmetic;
        return tmp2::clampChannelToSDRBottom(value);
    }

    static inline T clampDestinationChannelValue(T value) {
        using namespace Arithmetic;
        return tmp2::clampChannelToSDRBottom(value);
    }
};

template <typename T>
struct ClampedSourceAndDestinationCompositeFunctorBase
{
    static inline T clampSourceChannelValue(T value) {
        using namespace Arithmetic;
        return clampChannelToSDR(value);
    }

    static inline T clampDestinationChannelValue(T value) {
        using namespace Arithmetic;
        return clampChannelToSDR(value);
    }
};

/**
 * Generic CompositeOp for separable channel compositing functions
 * 
 * A template to generate a KoCompositeOp class by just specifying a
 * blending/compositing function. This template works with compositing functions
 * for separable channels (means each channel of a pixel can be processed separately)
 */
template<
    class Traits,
    typename CompositeOpFunctor,
    typename BlendingPolicy
>
class KoCompositeOpGenericSCFunctor: public KoCompositeOpBase< Traits, KoCompositeOpGenericSCFunctor<Traits,CompositeOpFunctor,BlendingPolicy> >
{
    typedef KoCompositeOpBase< Traits, KoCompositeOpGenericSCFunctor<Traits,CompositeOpFunctor,BlendingPolicy> > base_class;
    typedef typename Traits::channels_type                                            channels_type;
    
    static const qint32 channels_nb = Traits::channels_nb;
    static const qint32 alpha_pos   = Traits::alpha_pos;
    
public:
    KoCompositeOpGenericSCFunctor(const KoColorSpace* cs, const QString& id, const QString& category)
        : base_class(cs, id, category) { }

public:
    template<bool alphaLocked, bool allChannelFlags>
    inline static channels_type composeColorChannels(const channels_type* src, channels_type srcAlpha,
                                                     channels_type*       dst, channels_type dstAlpha, channels_type maskAlpha,
                                                     channels_type opacity, const QBitArray& channelFlags) {
        using namespace Arithmetic;
        
        srcAlpha = mul(srcAlpha, maskAlpha, opacity);

        if (tmp2::isZeroValue(srcAlpha)) {
            return dstAlpha;
        }

        if(alphaLocked) {
            if(!tmp2::isZeroValue(dstAlpha)) {
                for(qint32 i=0; i <channels_nb; i++) {
                    if(i != alpha_pos && (allChannelFlags || channelFlags.testBit(i))) {
                        const channels_type srcInBlendSpace =
                                CompositeOpFunctor::clampSourceChannelValue(
                                    BlendingPolicy::toAdditiveSpace(
                                        src[i]));
                        const channels_type dstInBlendSpace =
                                CompositeOpFunctor::clampDestinationChannelValue(
                                    BlendingPolicy::toAdditiveSpace(
                                        dst[i]));

                        dst[i] = BlendingPolicy::fromAdditiveSpace(
                            lerp(dstInBlendSpace,
                                 CompositeOpFunctor::composeChannel(srcInBlendSpace, dstInBlendSpace),
                                 srcAlpha));
                    }
                }
            }
            
            return dstAlpha;
        } else if (tmp2::isZeroValue(dstAlpha)) {
            for(qint32 i=0; i <channels_nb; i++) {
                if(i != alpha_pos && (allChannelFlags || channelFlags.testBit(i))) {
                    dst[i] = BlendingPolicy::fromAdditiveSpace(
                                CompositeOpFunctor::clampSourceChannelValue(
                                    BlendingPolicy::toAdditiveSpace(src[i])));
                }
            }
            return srcAlpha;
        } else if (tmp2::isUnitValue(dstAlpha)) {
            for(qint32 i=0; i <channels_nb; i++) {
                if(i != alpha_pos && (allChannelFlags || channelFlags.testBit(i))) {
                    const channels_type srcInBlendSpace =
                            CompositeOpFunctor::clampSourceChannelValue(
                                BlendingPolicy::toAdditiveSpace(
                                    src[i]));
                    const channels_type dstInBlendSpace =
                            CompositeOpFunctor::clampDestinationChannelValue(
                                BlendingPolicy::toAdditiveSpace(
                                    dst[i]));

                    dst[i] = BlendingPolicy::fromAdditiveSpace(
                        lerp(dstInBlendSpace,
                             CompositeOpFunctor::composeChannel(srcInBlendSpace, dstInBlendSpace),
                             srcAlpha));
                }
            }
            return unitValue<channels_type>();
        }  else if (tmp2::isUnitValue(srcAlpha)) {
            for(qint32 i=0; i <channels_nb; i++) {
                if(i != alpha_pos && (allChannelFlags || channelFlags.testBit(i))) {
                    const channels_type srcInBlendSpace =
                            CompositeOpFunctor::clampSourceChannelValue(
                                BlendingPolicy::toAdditiveSpace(
                                    src[i]));
                    const channels_type dstInBlendSpace =
                            CompositeOpFunctor::clampDestinationChannelValue(
                                BlendingPolicy::toAdditiveSpace(
                                    dst[i]));

                    dst[i] = BlendingPolicy::fromAdditiveSpace(
                        lerp(srcInBlendSpace,
                             CompositeOpFunctor::composeChannel(srcInBlendSpace, dstInBlendSpace),
                             dstAlpha));
                }
            }
            return unitValue<channels_type>();
        } else {
            channels_type newDstAlpha = unionShapeOpacity(srcAlpha, dstAlpha);

            if (!tmp2::isZeroValue(newDstAlpha)) {

                for(qint32 i=0; i <channels_nb; i++) {
                    if(i != alpha_pos && (allChannelFlags || channelFlags.testBit(i))) {
                        const channels_type srcInBlendSpace =
                                CompositeOpFunctor::clampSourceChannelValue(
                                    BlendingPolicy::toAdditiveSpace(
                                        src[i]));
                        const channels_type dstInBlendSpace =
                                CompositeOpFunctor::clampDestinationChannelValue(
                                    BlendingPolicy::toAdditiveSpace(
                                        dst[i]));

                        channels_type result =
                            blend(srcInBlendSpace, srcAlpha,
                                  dstInBlendSpace, dstAlpha,
                                  CompositeOpFunctor::composeChannel(srcInBlendSpace, dstInBlendSpace));

                        dst[i] = BlendingPolicy::fromAdditiveSpace(div(result, newDstAlpha));

//                         qDebug() << i
//                                  << fixed << qSetRealNumberPrecision(10)
//                                  << ppVar(src[i])
//                                  << ppVar(dst[i])
//                                  << ppVar(CompositeOpFunctor::composeChannel(srcInBlendSpace, dstInBlendSpace))
//                                  << ppVar(result)
//                                  << div(result, newDstAlpha)
//                                  << reset
//                                  << ppVar(newDstAlpha)
//                                  << ppVar(srcAlpha)
//                                  << ppVar(dstAlpha)
//                                  << ppVar(tmp2::isUnitValue(result))
//                                  << ppVar(tmp2::isUnitValue(newDstAlpha));
                    }
                }
            }
            
            return newDstAlpha;
        }
    }
};


template<
    class Traits,
    typename Traits::channels_type compositeFunc(typename Traits::channels_type, typename Traits::channels_type),
    typename BlendingPolicy
>
class KoCompositeOpGenericSC : public KoCompositeOpGenericSCFunctor<Traits, CompositeFunctionWrapper<Traits, compositeFunc>, BlendingPolicy>
{
protected:
    using base_class = KoCompositeOpGenericSCFunctor<Traits, CompositeFunctionWrapper<Traits, compositeFunc>, BlendingPolicy>;
public:
    using base_class::base_class;
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
    KoCompositeOpGenericHSL(const KoColorSpace* cs, const QString& id, const QString& category)
        : base_class(cs, id, category) { }
    
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
 * with taking alpha into consideration.
 * Note that because of special treating of alpha, any composite op function
 * needs to make alpha blending itself - the value of color that is written onto the projection
 * is the same that the composite function gives (compare with KoCompositeOpGenericHSL and KoCompositeOpGenericSC).
 */
template<class Traits, void compositeFunc(float, float, float&, float&), typename BlendingPolicy>
class KoCompositeOpGenericSCAlpha: public KoCompositeOpBase< Traits, KoCompositeOpGenericSCAlpha<Traits,compositeFunc,BlendingPolicy> >
{
    typedef KoCompositeOpBase< Traits, KoCompositeOpGenericSCAlpha<Traits,compositeFunc,BlendingPolicy> > base_class;
    typedef typename Traits::channels_type                                             channels_type;

    static const qint32 channels_nb = Traits::channels_nb;
    static const qint32 alpha_pos  = Traits::alpha_pos;

public:
    KoCompositeOpGenericSCAlpha(const KoColorSpace* cs, const QString& id, const QString& category)
        : base_class(cs, id, category) { }

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
                        float dstValueFloat = scale<float>(BlendingPolicy::toAdditiveSpace(dst[i]));
                        float dstAlphaFloat = scale<float>(oldAlpha);
                        compositeFunc(scale<float>(BlendingPolicy::toAdditiveSpace(src[i])), scale<float>(srcAlpha), dstValueFloat, dstAlphaFloat);
                        dst[i] = BlendingPolicy::fromAdditiveSpace(scale<channels_type>(dstValueFloat));
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
                        float dstFloat = scale<float>(BlendingPolicy::toAdditiveSpace(dst[i]));
                        float dstAlphaFloat = scale<float>(oldAlpha);
                        compositeFunc(scale<float>(BlendingPolicy::toAdditiveSpace(src[i])), scale<float>(srcAlpha), dstFloat, dstAlphaFloat);
                        dst[i] = BlendingPolicy::fromAdditiveSpace(scale<channels_type>(dstFloat));
                    }
                }
            }

            return newDstAlpha;
        }
    }
};



#endif // _KOCOMPOSITEOP_GENERIC_H_
