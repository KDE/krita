/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef _KOCOMPOSITEOPS_H_
#define _KOCOMPOSITEOPS_H_

#include <boost/type_traits.hpp>

#include <KoColorSpace.h>
#include <KoColorSpaceTraits.h>
#include <KoColorSpaceMaths.h>

#include "compositeops/KoCompositeOpGeneric.h"
#include "compositeops/KoCompositeOpOver.h"
#include "compositeops/KoCompositeOpCopyChannel.h"
#include "compositeops/KoCompositeOpAlphaDarken.h"
#include "compositeops/KoCompositeOpErase.h"
#include "compositeops/KoCompositeOpCopy2.h"
#include "compositeops/KoCompositeOpDissolve.h"
#include "compositeops/KoCompositeOpBehind.h"
#include "compositeops/KoCompositeOpDestinationIn.h"
#include "compositeops/KoCompositeOpDestinationAtop.h"
#include "compositeops/KoCompositeOpGreater.h"
#include "compositeops/KoAlphaDarkenParamsWrapper.h"
#include "KoOptimizedCompositeOpFactory.h"

namespace _Private {

template<class Traits, bool flag>
struct AddGeneralOps
{
    static void add(KoColorSpace* cs) { Q_UNUSED(cs); }
};

template<class Traits>
struct OptimizedOpsSelector
{
    static KoCompositeOp* createAlphaDarkenOp(const KoColorSpace *cs) {
        if (useCreamyAlphaDarken()) {
            return new KoCompositeOpAlphaDarken<Traits, KoAlphaDarkenParamsWrapperCreamy>(cs);
        } else {
            return new KoCompositeOpAlphaDarken<Traits, KoAlphaDarkenParamsWrapperHard>(cs);
        }
    }
    static KoCompositeOp* createOverOp(const KoColorSpace *cs) {
        return new KoCompositeOpOver<Traits>(cs);
    }

    static KoCompositeOp* createCopyOp(const KoColorSpace *cs) {
        return new KoCompositeOpCopy2<Traits>(cs);
    }
};

template<>
struct OptimizedOpsSelector<KoBgrU8Traits>
{
    static KoCompositeOp* createAlphaDarkenOp(const KoColorSpace *cs) {
        return useCreamyAlphaDarken() ?
            KoOptimizedCompositeOpFactory::createAlphaDarkenOpCreamy32(cs) :
            KoOptimizedCompositeOpFactory::createAlphaDarkenOpHard32(cs);
    }
    static KoCompositeOp* createOverOp(const KoColorSpace *cs) {
        return KoOptimizedCompositeOpFactory::createOverOp32(cs);
    }

    static KoCompositeOp* createCopyOp(const KoColorSpace *cs) {
        return KoOptimizedCompositeOpFactory::createCopyOp32(cs);
    }
};

template<>
struct OptimizedOpsSelector<KoLabU8Traits>
{
    static KoCompositeOp* createAlphaDarkenOp(const KoColorSpace *cs) {
        return useCreamyAlphaDarken() ?
            KoOptimizedCompositeOpFactory::createAlphaDarkenOpCreamy32(cs) :
            KoOptimizedCompositeOpFactory::createAlphaDarkenOpHard32(cs);
    }
    static KoCompositeOp* createOverOp(const KoColorSpace *cs) {
        return KoOptimizedCompositeOpFactory::createOverOp32(cs);
    }
    static KoCompositeOp* createCopyOp(const KoColorSpace *cs) {
        return KoOptimizedCompositeOpFactory::createCopyOp32(cs);
    }
};

template<>
struct OptimizedOpsSelector<KoRgbF32Traits>
{
    static KoCompositeOp* createAlphaDarkenOp(const KoColorSpace *cs) {
        return useCreamyAlphaDarken() ?
            KoOptimizedCompositeOpFactory::createAlphaDarkenOpCreamy128(cs) :
            KoOptimizedCompositeOpFactory::createAlphaDarkenOpHard128(cs);

    }
    static KoCompositeOp* createOverOp(const KoColorSpace *cs) {
        return KoOptimizedCompositeOpFactory::createOverOp128(cs);
    }
    static KoCompositeOp* createCopyOp(const KoColorSpace *cs) {
        return KoOptimizedCompositeOpFactory::createCopyOp128(cs);
    }
};

template<>
struct OptimizedOpsSelector<KoBgrU16Traits>
{
    static KoCompositeOp* createAlphaDarkenOp(const KoColorSpace *cs) {
        return useCreamyAlphaDarken() ?
            KoOptimizedCompositeOpFactory::createAlphaDarkenOpCreamyU64(cs) :
            KoOptimizedCompositeOpFactory::createAlphaDarkenOpHardU64(cs);

    }
    static KoCompositeOp* createOverOp(const KoColorSpace *cs) {
        return KoOptimizedCompositeOpFactory::createOverOpU64(cs);
    }
    static KoCompositeOp* createCopyOp(const KoColorSpace *cs) {
        return KoOptimizedCompositeOpFactory::createCopyOpU64(cs);
    }
};


template<class Traits>
struct AddGeneralOps<Traits, true>
{
     typedef typename Traits::channels_type Arg;
     typedef Arg (*CompositeFunc)(Arg, Arg);
     static const qint32 alpha_pos = Traits::alpha_pos;

     template<CompositeFunc func>
     static void add(KoColorSpace* cs, const QString& id, const QString& category) {
         cs->addCompositeOp(new KoCompositeOpGenericSC<Traits, func>(cs, id, category));
     }

     static void add(KoColorSpace* cs) {
         cs->addCompositeOp(OptimizedOpsSelector<Traits>::createOverOp(cs));
         cs->addCompositeOp(OptimizedOpsSelector<Traits>::createAlphaDarkenOp(cs));
         cs->addCompositeOp(OptimizedOpsSelector<Traits>::createCopyOp(cs));
         cs->addCompositeOp(new KoCompositeOpErase<Traits>(cs));
         cs->addCompositeOp(new KoCompositeOpBehind<Traits>(cs));
         cs->addCompositeOp(new KoCompositeOpDestinationIn<Traits>(cs));
         cs->addCompositeOp(new KoCompositeOpDestinationAtop<Traits>(cs));
         cs->addCompositeOp(new KoCompositeOpGreater<Traits>(cs));

         add<&cfOverlay<Arg>       >(cs, COMPOSITE_OVERLAY       , KoCompositeOp::categoryMix());
         add<&cfGrainMerge<Arg>    >(cs, COMPOSITE_GRAIN_MERGE   , KoCompositeOp::categoryMix());
         add<&cfGrainExtract<Arg>  >(cs, COMPOSITE_GRAIN_EXTRACT , KoCompositeOp::categoryMix());
         add<&cfHardMix<Arg>       >(cs, COMPOSITE_HARD_MIX      , KoCompositeOp::categoryMix());
         add<&cfHardMixPhotoshop<Arg>>(cs, COMPOSITE_HARD_MIX_PHOTOSHOP, KoCompositeOp::categoryMix());
         add<&cfHardMixSofterPhotoshop<Arg>>(cs, COMPOSITE_HARD_MIX_SOFTER_PHOTOSHOP, KoCompositeOp::categoryMix());
         add<&cfGeometricMean<Arg> >(cs, COMPOSITE_GEOMETRIC_MEAN, KoCompositeOp::categoryMix());
         add<&cfParallel<Arg>      >(cs, COMPOSITE_PARALLEL      , KoCompositeOp::categoryMix());
         add<&cfAllanon<Arg>       >(cs, COMPOSITE_ALLANON       , KoCompositeOp::categoryMix());
         add<&cfHardOverlay<Arg>   >(cs, COMPOSITE_HARD_OVERLAY  , KoCompositeOp::categoryMix());
         add<&cfInterpolation<Arg> >(cs, COMPOSITE_INTERPOLATION , KoCompositeOp::categoryMix());
         add<&cfInterpolationB<Arg>>(cs, COMPOSITE_INTERPOLATIONB, KoCompositeOp::categoryMix());
         add<&cfPenumbraA<Arg>     >(cs, COMPOSITE_PENUMBRAA     , KoCompositeOp::categoryMix());
         add<&cfPenumbraB<Arg>     >(cs, COMPOSITE_PENUMBRAB     , KoCompositeOp::categoryMix());
         add<&cfPenumbraC<Arg>     >(cs, COMPOSITE_PENUMBRAC     , KoCompositeOp::categoryMix());
         add<&cfPenumbraD<Arg>     >(cs, COMPOSITE_PENUMBRAD     , KoCompositeOp::categoryMix());

         add<&cfScreen<Arg>      >(cs, COMPOSITE_SCREEN      , KoCompositeOp::categoryLight());
         add<&cfColorDodge<Arg>  >(cs, COMPOSITE_DODGE       , KoCompositeOp::categoryLight());
         add<&cfAddition<Arg>    >(cs, COMPOSITE_LINEAR_DODGE, KoCompositeOp::categoryLight());
         add<&cfLightenOnly<Arg> >(cs, COMPOSITE_LIGHTEN     , KoCompositeOp::categoryLight());
         add<&cfHardLight<Arg>   >(cs, COMPOSITE_HARD_LIGHT  , KoCompositeOp::categoryLight());
         add<&cfSoftLightIFSIllusions<Arg>>(cs, COMPOSITE_SOFT_LIGHT_IFS_ILLUSIONS, KoCompositeOp::categoryLight());
         add<&cfSoftLightPegtopDelphi<Arg>>(cs, COMPOSITE_SOFT_LIGHT_PEGTOP_DELPHI, KoCompositeOp::categoryLight());
         add<&cfSoftLightSvg<Arg>>(cs, COMPOSITE_SOFT_LIGHT_SVG, KoCompositeOp::categoryLight());
         add<&cfSoftLight<Arg>   >(cs, COMPOSITE_SOFT_LIGHT_PHOTOSHOP, KoCompositeOp::categoryLight());
         add<&cfGammaLight<Arg>  >(cs, COMPOSITE_GAMMA_LIGHT , KoCompositeOp::categoryLight());
         add<&cfGammaIllumination<Arg>>(cs, COMPOSITE_GAMMA_ILLUMINATION, KoCompositeOp::categoryLight());
         add<&cfVividLight<Arg>  >(cs, COMPOSITE_VIVID_LIGHT , KoCompositeOp::categoryLight());
         add<&cfFlatLight<Arg>   >(cs, COMPOSITE_FLAT_LIGHT  , KoCompositeOp::categoryLight());
         add<&cfPinLight<Arg>    >(cs, COMPOSITE_PIN_LIGHT   , KoCompositeOp::categoryLight());
         add<&cfLinearLight<Arg> >(cs, COMPOSITE_LINEAR_LIGHT, KoCompositeOp::categoryLight());
         add<&cfPNormA<Arg>      >(cs, COMPOSITE_PNORM_A     , KoCompositeOp::categoryLight());
         add<&cfPNormB<Arg>      >(cs, COMPOSITE_PNORM_B     , KoCompositeOp::categoryLight());
         add<&cfSuperLight<Arg>  >(cs, COMPOSITE_SUPER_LIGHT , KoCompositeOp::categoryLight());
         add<&cfTintIFSIllusions<Arg>>(cs, COMPOSITE_TINT_IFS_ILLUSIONS, KoCompositeOp::categoryLight());
         add<&cfFogLightenIFSIllusions<Arg>>(cs, COMPOSITE_FOG_LIGHTEN_IFS_ILLUSIONS, KoCompositeOp::categoryLight());
         add<&cfEasyDodge<Arg>   >(cs, COMPOSITE_EASY_DODGE  , KoCompositeOp::categoryLight());

         add<&cfColorBurn<Arg>  >(cs, COMPOSITE_BURN        , KoCompositeOp::categoryDark());
         add<&cfLinearBurn<Arg> >(cs, COMPOSITE_LINEAR_BURN , KoCompositeOp::categoryDark());
         add<&cfDarkenOnly<Arg> >(cs, COMPOSITE_DARKEN      , KoCompositeOp::categoryDark());
         add<&cfGammaDark<Arg>  >(cs, COMPOSITE_GAMMA_DARK  , KoCompositeOp::categoryDark());
         add<&cfShadeIFSIllusions<Arg>>(cs, COMPOSITE_SHADE_IFS_ILLUSIONS, KoCompositeOp::categoryDark());
         add<&cfFogDarkenIFSIllusions<Arg>>(cs, COMPOSITE_FOG_DARKEN_IFS_ILLUSIONS, KoCompositeOp::categoryDark());
         add<&cfEasyBurn<Arg>   >(cs, COMPOSITE_EASY_BURN   , KoCompositeOp::categoryDark());

         add<&cfAddition<Arg>        >(cs, COMPOSITE_ADD             , KoCompositeOp::categoryArithmetic());
         add<&cfSubtract<Arg>        >(cs, COMPOSITE_SUBTRACT        , KoCompositeOp::categoryArithmetic());
         add<&cfInverseSubtract<Arg> >(cs, COMPOSITE_INVERSE_SUBTRACT, KoCompositeOp::categoryArithmetic());
         add<&cfMultiply<Arg>        >(cs, COMPOSITE_MULT            , KoCompositeOp::categoryArithmetic());
         add<&cfDivide<Arg>          >(cs, COMPOSITE_DIVIDE          , KoCompositeOp::categoryArithmetic());

         add<&cfModulo<Arg>               >(cs, COMPOSITE_MOD                , KoCompositeOp::categoryModulo());
         add<&cfModuloContinuous<Arg>     >(cs, COMPOSITE_MOD_CON            , KoCompositeOp::categoryModulo());
         add<&cfDivisiveModulo<Arg>       >(cs, COMPOSITE_DIVISIVE_MOD       , KoCompositeOp::categoryModulo());
         add<&cfDivisiveModuloContinuous<Arg>>(cs, COMPOSITE_DIVISIVE_MOD_CON, KoCompositeOp::categoryModulo());
         add<&cfModuloShift<Arg>          >(cs, COMPOSITE_MODULO_SHIFT       , KoCompositeOp::categoryModulo());
         add<&cfModuloShiftContinuous<Arg>>(cs, COMPOSITE_MODULO_SHIFT_CON   , KoCompositeOp::categoryModulo());

         add<&cfArcTangent<Arg>         >(cs, COMPOSITE_ARC_TANGENT         , KoCompositeOp::categoryNegative());
         add<&cfDifference<Arg>         >(cs, COMPOSITE_DIFF                , KoCompositeOp::categoryNegative());
         add<&cfExclusion<Arg>          >(cs, COMPOSITE_EXCLUSION           , KoCompositeOp::categoryNegative());
         add<&cfEquivalence<Arg>        >(cs, COMPOSITE_EQUIVALENCE         , KoCompositeOp::categoryNegative());
         add<&cfAdditiveSubtractive<Arg>>(cs, COMPOSITE_ADDITIVE_SUBTRACTIVE, KoCompositeOp::categoryNegative());
         add<&cfNegation<Arg>           >(cs, COMPOSITE_NEGATION            , KoCompositeOp::categoryNegative());
         
         add<&cfXor<Arg>        >(cs, COMPOSITE_XOR            , KoCompositeOp::categoryBinary());
         add<&cfOr<Arg>         >(cs, COMPOSITE_OR             , KoCompositeOp::categoryBinary());
         add<&cfAnd<Arg>        >(cs, COMPOSITE_AND            , KoCompositeOp::categoryBinary());
         add<&cfNand<Arg>       >(cs, COMPOSITE_NAND           , KoCompositeOp::categoryBinary());
         add<&cfNor<Arg>        >(cs, COMPOSITE_NOR            , KoCompositeOp::categoryBinary());
         add<&cfXnor<Arg>       >(cs, COMPOSITE_XNOR           , KoCompositeOp::categoryBinary());
         add<&cfImplies<Arg>    >(cs, COMPOSITE_IMPLICATION    , KoCompositeOp::categoryBinary());
         add<&cfNotImplies<Arg> >(cs, COMPOSITE_NOT_IMPLICATION, KoCompositeOp::categoryBinary());
         add<&cfConverse<Arg>   >(cs, COMPOSITE_CONVERSE       , KoCompositeOp::categoryBinary());
         add<&cfNotConverse<Arg>>(cs, COMPOSITE_NOT_CONVERSE   , KoCompositeOp::categoryBinary());

         add<&cfReflect<Arg>>(cs, COMPOSITE_REFLECT, KoCompositeOp::categoryQuadratic());
         add<&cfGlow<Arg>   >(cs, COMPOSITE_GLOW   , KoCompositeOp::categoryQuadratic());
         add<&cfFreeze<Arg> >(cs, COMPOSITE_FREEZE , KoCompositeOp::categoryQuadratic());
         add<&cfHeat<Arg>   >(cs, COMPOSITE_HEAT   , KoCompositeOp::categoryQuadratic());
         add<&cfGleat<Arg>  >(cs, COMPOSITE_GLEAT  , KoCompositeOp::categoryQuadratic());
         add<&cfHelow<Arg>  >(cs, COMPOSITE_HELOW  , KoCompositeOp::categoryQuadratic());
         add<&cfReeze<Arg>  >(cs, COMPOSITE_REEZE  , KoCompositeOp::categoryQuadratic());
         add<&cfFrect<Arg>  >(cs, COMPOSITE_FRECT  , KoCompositeOp::categoryQuadratic());
         add<&cfFhyrd<Arg>  >(cs, COMPOSITE_FHYRD  , KoCompositeOp::categoryQuadratic());

         cs->addCompositeOp(new KoCompositeOpDissolve<Traits>(cs, KoCompositeOp::categoryMisc()));
     }
};

template<class Traits, bool flag>
struct AddRGBOps
{
    static void add(KoColorSpace* cs) { Q_UNUSED(cs); }
};

template<class Traits>
struct AddRGBOps<Traits, true>
{
    typedef float Arg;

    static const qint32 red_pos   = Traits::red_pos;
    static const qint32 green_pos = Traits::green_pos;
    static const qint32 blue_pos  = Traits::blue_pos;

    template<void compositeFunc(Arg, Arg, Arg, Arg&, Arg&, Arg&)>

    static void add(KoColorSpace* cs, const QString& id, const QString& category) {
        cs->addCompositeOp(new KoCompositeOpGenericHSL<Traits, compositeFunc>(cs, id, category));
    }

    static void add(KoColorSpace* cs) {

        cs->addCompositeOp(new KoCompositeOpCopyChannel<Traits,red_pos  >(cs, COMPOSITE_COPY_RED  , KoCompositeOp::categoryMisc()));
        cs->addCompositeOp(new KoCompositeOpCopyChannel<Traits,green_pos>(cs, COMPOSITE_COPY_GREEN, KoCompositeOp::categoryMisc()));
        cs->addCompositeOp(new KoCompositeOpCopyChannel<Traits,blue_pos >(cs, COMPOSITE_COPY_BLUE , KoCompositeOp::categoryMisc()));
        add<&cfTangentNormalmap  <HSYType,Arg> >(cs, COMPOSITE_TANGENT_NORMALMAP  , KoCompositeOp::categoryMisc());
        add<&cfReorientedNormalMapCombine <HSYType, Arg> >(cs, COMPOSITE_COMBINE_NORMAL, KoCompositeOp::categoryMisc());

        add<&cfColor             <HSYType,Arg> >(cs, COMPOSITE_COLOR         , KoCompositeOp::categoryHSY());
        add<&cfHue               <HSYType,Arg> >(cs, COMPOSITE_HUE           , KoCompositeOp::categoryHSY());
        add<&cfSaturation        <HSYType,Arg> >(cs, COMPOSITE_SATURATION    , KoCompositeOp::categoryHSY());
        add<&cfIncreaseSaturation<HSYType,Arg> >(cs, COMPOSITE_INC_SATURATION, KoCompositeOp::categoryHSY());
        add<&cfDecreaseSaturation<HSYType,Arg> >(cs, COMPOSITE_DEC_SATURATION, KoCompositeOp::categoryHSY());
        add<&cfLightness         <HSYType,Arg> >(cs, COMPOSITE_LUMINIZE      , KoCompositeOp::categoryHSY());
        add<&cfIncreaseLightness <HSYType,Arg> >(cs, COMPOSITE_INC_LUMINOSITY, KoCompositeOp::categoryHSY());
        add<&cfDecreaseLightness <HSYType,Arg> >(cs, COMPOSITE_DEC_LUMINOSITY, KoCompositeOp::categoryHSY());
        add<&cfDarkerColor <HSYType,Arg> >(cs, COMPOSITE_DARKER_COLOR        , KoCompositeOp::categoryDark());//darker color as PSD does it//
        add<&cfLighterColor <HSYType,Arg> >(cs, COMPOSITE_LIGHTER_COLOR      , KoCompositeOp::categoryLight());//lighter color as PSD does it//

        add<&cfColor             <HSIType,Arg> >(cs, COMPOSITE_COLOR_HSI         , KoCompositeOp::categoryHSI());
        add<&cfHue               <HSIType,Arg> >(cs, COMPOSITE_HUE_HSI           , KoCompositeOp::categoryHSI());
        add<&cfSaturation        <HSIType,Arg> >(cs, COMPOSITE_SATURATION_HSI    , KoCompositeOp::categoryHSI());
        add<&cfIncreaseSaturation<HSIType,Arg> >(cs, COMPOSITE_INC_SATURATION_HSI, KoCompositeOp::categoryHSI());
        add<&cfDecreaseSaturation<HSIType,Arg> >(cs, COMPOSITE_DEC_SATURATION_HSI, KoCompositeOp::categoryHSI());
        add<&cfLightness         <HSIType,Arg> >(cs, COMPOSITE_INTENSITY         , KoCompositeOp::categoryHSI());
        add<&cfIncreaseLightness <HSIType,Arg> >(cs, COMPOSITE_INC_INTENSITY     , KoCompositeOp::categoryHSI());
        add<&cfDecreaseLightness <HSIType,Arg> >(cs, COMPOSITE_DEC_INTENSITY     , KoCompositeOp::categoryHSI());

        add<&cfColor             <HSLType,Arg> >(cs, COMPOSITE_COLOR_HSL         , KoCompositeOp::categoryHSL());
        add<&cfHue               <HSLType,Arg> >(cs, COMPOSITE_HUE_HSL           , KoCompositeOp::categoryHSL());
        add<&cfSaturation        <HSLType,Arg> >(cs, COMPOSITE_SATURATION_HSL    , KoCompositeOp::categoryHSL());
        add<&cfIncreaseSaturation<HSLType,Arg> >(cs, COMPOSITE_INC_SATURATION_HSL, KoCompositeOp::categoryHSL());
        add<&cfDecreaseSaturation<HSLType,Arg> >(cs, COMPOSITE_DEC_SATURATION_HSL, KoCompositeOp::categoryHSL());
        add<&cfLightness         <HSLType,Arg> >(cs, COMPOSITE_LIGHTNESS         , KoCompositeOp::categoryHSL());
        add<&cfIncreaseLightness <HSLType,Arg> >(cs, COMPOSITE_INC_LIGHTNESS     , KoCompositeOp::categoryHSL());
        add<&cfDecreaseLightness <HSLType,Arg> >(cs, COMPOSITE_DEC_LIGHTNESS     , KoCompositeOp::categoryHSL());

        add<&cfColor             <HSVType,Arg> >(cs, COMPOSITE_COLOR_HSV         , KoCompositeOp::categoryHSV());
        add<&cfHue               <HSVType,Arg> >(cs, COMPOSITE_HUE_HSV           , KoCompositeOp::categoryHSV());
        add<&cfSaturation        <HSVType,Arg> >(cs, COMPOSITE_SATURATION_HSV    , KoCompositeOp::categoryHSV());
        add<&cfIncreaseSaturation<HSVType,Arg> >(cs, COMPOSITE_INC_SATURATION_HSV, KoCompositeOp::categoryHSV());
        add<&cfDecreaseSaturation<HSVType,Arg> >(cs, COMPOSITE_DEC_SATURATION_HSV, KoCompositeOp::categoryHSV());
        add<&cfLightness         <HSVType,Arg> >(cs, COMPOSITE_VALUE             , KoCompositeOp::categoryHSV());
        add<&cfIncreaseLightness <HSVType,Arg> >(cs, COMPOSITE_INC_VALUE         , KoCompositeOp::categoryHSV());
        add<&cfDecreaseLightness <HSVType,Arg> >(cs, COMPOSITE_DEC_VALUE         , KoCompositeOp::categoryHSV());
    }
};




template<class Traits, bool flag>
struct AddGeneralAlphaOps
{
    static void add(KoColorSpace* cs) { Q_UNUSED(cs); }
};

template<class Traits>
struct AddGeneralAlphaOps<Traits, true>
{
    typedef float Arg;
    static const qint32 alpha_pos  = Traits::alpha_pos;
    template<void compositeFunc(Arg, Arg, Arg&, Arg&)>


    static void add(KoColorSpace* cs, const QString& id, const QString& category)
    {
        cs->addCompositeOp(new KoCompositeOpGenericSCAlpha<Traits, compositeFunc>(cs, id, category));
    }

    static void add(KoColorSpace* cs)
    {
        add<&cfAdditionSAI<HSVType,Arg>>(cs, COMPOSITE_LUMINOSITY_SAI, KoCompositeOp::categoryHSV());
    }


};





}





/**
 * This function add to the colorspace all the composite ops defined by
 * the pigment library.
 */
template<class _Traits_>
void addStandardCompositeOps(KoColorSpace* cs)
{
    typedef typename _Traits_::channels_type channels_type;

    static const bool useGeneralOps = true;
    static const bool useRGBOps = (boost::is_base_of<KoBgrTraits<channels_type>, _Traits_>::value
                                || boost::is_base_of<KoRgbTraits<channels_type>, _Traits_>::value);

    _Private::AddGeneralOps      <_Traits_, useGeneralOps>::add(cs);
    _Private::AddRGBOps          <_Traits_, useRGBOps    >::add(cs);
    _Private::AddGeneralAlphaOps <_Traits_, useGeneralOps>::add(cs);

}

template<class _Traits_>
KoCompositeOp* createAlphaDarkenCompositeOp(const KoColorSpace *cs)
{
    return _Private::OptimizedOpsSelector<_Traits_>::createAlphaDarkenOp(cs);
}

#endif
