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
#include <KoCompositeOpFunctions.h>

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
#include "compositeops/KoCompositeOpMarker.h"
#include "compositeops/KoAlphaDarkenParamsWrapper.h"
#include "compositeops/KoColorSpaceBlendingPolicy.h"
#include "compositeops/KoCompositeOpClampPolicy.h"
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
     static constexpr bool IsIntegerSpace = std::numeric_limits<Arg>::is_integer;

     template<CompositeFunc func>
     static void add(KoColorSpace* cs, const QString& id, const QString& category) {
        if constexpr (std::is_base_of_v<KoCmykTraits<typename Traits::channels_type>, Traits>) {
            if (useSubtractiveBlendingForCmykColorSpaces()) {
                cs->addCompositeOp(new KoCompositeOpGenericSC<Traits, func, KoSubtractiveBlendingPolicy<Traits>>(cs, id, category));
            } else {
                cs->addCompositeOp(new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, category));
            }
        } else {
            cs->addCompositeOp(new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, category));
        }
     }

     template<typename Functor>
     static void add(KoColorSpace* cs, const QString& id, const QString& category) {
         if constexpr (std::is_base_of_v<KoCmykTraits<typename Traits::channels_type>, Traits>) {
             if (useSubtractiveBlendingForCmykColorSpaces()) {
                 cs->addCompositeOp(new KoCompositeOpGenericSCFunctor<Traits, Functor, KoSubtractiveBlendingPolicy<Traits>>(cs, id, category));
             } else {
                 cs->addCompositeOp(new KoCompositeOpGenericSCFunctor<Traits, Functor, KoAdditiveBlendingPolicy<Traits>>(cs, id, category));
             }
         } else {
             cs->addCompositeOp(new KoCompositeOpGenericSCFunctor<Traits, Functor, KoAdditiveBlendingPolicy<Traits>>(cs, id, category));
         }
     }

     static void add(KoColorSpace* cs) {
         using namespace KoCompositeOpClampPolicy;

         cs->addCompositeOp(OptimizedOpsSelector<Traits>::createOverOp(cs));
         cs->addCompositeOp(OptimizedOpsSelector<Traits>::createAlphaDarkenOp(cs));
         cs->addCompositeOp(OptimizedOpsSelector<Traits>::createCopyOp(cs));
         cs->addCompositeOp(new KoCompositeOpErase<Traits>(cs));

         if constexpr (std::is_base_of_v<KoCmykTraits<typename Traits::channels_type>, Traits>) {
            if (useSubtractiveBlendingForCmykColorSpaces()) {
                cs->addCompositeOp(new KoCompositeOpBehind<Traits, KoSubtractiveBlendingPolicy<Traits>>(cs));
            } else {
                cs->addCompositeOp(new KoCompositeOpBehind<Traits, KoAdditiveBlendingPolicy<Traits>>(cs));
            }
         } else {
            cs->addCompositeOp(new KoCompositeOpBehind<Traits, KoAdditiveBlendingPolicy<Traits>>(cs));
         }

         cs->addCompositeOp(new KoCompositeOpDestinationIn<Traits>(cs));
         cs->addCompositeOp(new KoCompositeOpDestinationAtop<Traits>(cs));

         if constexpr (std::is_base_of_v<KoCmykTraits<typename Traits::channels_type>, Traits>) {
            if (useSubtractiveBlendingForCmykColorSpaces()) {
                cs->addCompositeOp(new KoCompositeOpGreater<Traits, KoSubtractiveBlendingPolicy<Traits>>(cs));
            } else {
                cs->addCompositeOp(new KoCompositeOpGreater<Traits, KoAdditiveBlendingPolicy<Traits>>(cs));
            }
         } else {
            cs->addCompositeOp(new KoCompositeOpGreater<Traits, KoAdditiveBlendingPolicy<Traits>>(cs));
         }

         add<CFOverlay<Arg>        >(cs, COMPOSITE_OVERLAY       , KoCompositeOp::categoryMix());
         add<CFGrainMerge<Arg>     >(cs, COMPOSITE_GRAIN_MERGE   , KoCompositeOp::categoryMix());
         add<CFGrainExtract<Arg>   >(cs, COMPOSITE_GRAIN_EXTRACT , KoCompositeOp::categoryMix());
         add<FunctorWithSDRClampPolicy<CFHardMix, Arg>>(cs, COMPOSITE_HARD_MIX, KoCompositeOp::categoryMix());
         if constexpr (!IsIntegerSpace) {
             add<CFHardMix<Arg, ClampAsFloatHDR>>(cs, COMPOSITE_HARD_MIX_HDR, KoCompositeOp::categoryMix());
         }

         add<CFHardMixPhotoshop<Arg>>(cs, COMPOSITE_HARD_MIX_PHOTOSHOP, KoCompositeOp::categoryMix());
         add<CFHardMixSofterPhotoshop<Arg>>(cs, COMPOSITE_HARD_MIX_SOFTER_PHOTOSHOP, KoCompositeOp::categoryMix());
         add<CFGeometricMean<Arg>  >(cs, COMPOSITE_GEOMETRIC_MEAN, KoCompositeOp::categoryMix());
         add<CFParallel<Arg>       >(cs, COMPOSITE_PARALLEL      , KoCompositeOp::categoryMix());
         add<&cfAllanon<Arg>       >(cs, COMPOSITE_ALLANON       , KoCompositeOp::categoryMix());
         add<FunctorWithSDRClampPolicy<CFHardOverlay, Arg>>(cs, COMPOSITE_HARD_OVERLAY, KoCompositeOp::categoryMix());
         if constexpr (!IsIntegerSpace) {
             add<CFHardOverlay<Arg, ClampAsFloatHDR>>(cs, COMPOSITE_HARD_OVERLAY_HDR, KoCompositeOp::categoryMix());
         }

         add<&cfInterpolation<Arg> >(cs, COMPOSITE_INTERPOLATION , KoCompositeOp::categoryMix());
         add<&cfInterpolationB<Arg>>(cs, COMPOSITE_INTERPOLATIONB, KoCompositeOp::categoryMix());
         add<CFPenumbraA<Arg>      >(cs, COMPOSITE_PENUMBRAA     , KoCompositeOp::categoryMix());
         add<CFPenumbraB<Arg>      >(cs, COMPOSITE_PENUMBRAB     , KoCompositeOp::categoryMix());
         add<&cfPenumbraC<Arg>     >(cs, COMPOSITE_PENUMBRAC     , KoCompositeOp::categoryMix());
         add<&cfPenumbraD<Arg>     >(cs, COMPOSITE_PENUMBRAD     , KoCompositeOp::categoryMix());
         cs->addCompositeOp(new KoCompositeOpMarker<Traits>(cs));

         add<&cfScreen<Arg>      >(cs, COMPOSITE_SCREEN      , KoCompositeOp::categoryLight());

         add<FunctorWithSDRClampPolicy<CFColorDodge, Arg>>(cs, COMPOSITE_DODGE, KoCompositeOp::categoryLight());
         if constexpr (!IsIntegerSpace) {
             add<CFColorDodge<Arg, ClampAsFloatHDR>>(cs, COMPOSITE_DODGE_HDR, KoCompositeOp::categoryLight());
         }

         add<&cfAddition<Arg>    >(cs, COMPOSITE_LINEAR_DODGE, KoCompositeOp::categoryLight());
         add<&cfLightenOnly<Arg> >(cs, COMPOSITE_LIGHTEN     , KoCompositeOp::categoryLight());
         add<CFHardLight<Arg>    >(cs, COMPOSITE_HARD_LIGHT  , KoCompositeOp::categoryLight());
         add<&cfSoftLightIFSIllusions<Arg>>(cs, COMPOSITE_SOFT_LIGHT_IFS_ILLUSIONS, KoCompositeOp::categoryLight());
         add<&cfSoftLightPegtopDelphi<Arg>>(cs, COMPOSITE_SOFT_LIGHT_PEGTOP_DELPHI, KoCompositeOp::categoryLight());
         add<CFSoftLightSvg<Arg> >(cs, COMPOSITE_SOFT_LIGHT_SVG, KoCompositeOp::categoryLight());
         add<CFSoftLight<Arg>    >(cs, COMPOSITE_SOFT_LIGHT_PHOTOSHOP, KoCompositeOp::categoryLight());
         add<CFGammaLight<Arg>   >(cs, COMPOSITE_GAMMA_LIGHT , KoCompositeOp::categoryLight());
         add<CFGammaIllumination<Arg>>(cs, COMPOSITE_GAMMA_ILLUMINATION, KoCompositeOp::categoryLight());

         add<FunctorWithSDRClampPolicy<CFVividLight, Arg>>(cs, COMPOSITE_VIVID_LIGHT, KoCompositeOp::categoryLight());
         if constexpr (!IsIntegerSpace) {
             add<CFVividLight<Arg, ClampAsFloatHDR>>(cs, COMPOSITE_VIVID_LIGHT_HDR, KoCompositeOp::categoryLight());
         }
         add<CFFlatLight<Arg>    >(cs, COMPOSITE_FLAT_LIGHT  , KoCompositeOp::categoryLight());
         add<CFPinLight<Arg>>(cs, COMPOSITE_PIN_LIGHT, KoCompositeOp::categoryLight()); // using HDR mode as default
         add<&cfLinearLight<Arg> >(cs, COMPOSITE_LINEAR_LIGHT, KoCompositeOp::categoryLight());
         add<&cfPNormA<Arg>      >(cs, COMPOSITE_PNORM_A     , KoCompositeOp::categoryLight());
         add<&cfPNormB<Arg>      >(cs, COMPOSITE_PNORM_B     , KoCompositeOp::categoryLight());
         add<&cfSuperLight<Arg>  >(cs, COMPOSITE_SUPER_LIGHT , KoCompositeOp::categoryLight());
         add<&cfTintIFSIllusions<Arg>>(cs, COMPOSITE_TINT_IFS_ILLUSIONS, KoCompositeOp::categoryLight());
         add<&cfFogLightenIFSIllusions<Arg>>(cs, COMPOSITE_FOG_LIGHTEN_IFS_ILLUSIONS, KoCompositeOp::categoryLight());
         add<&cfEasyDodge<Arg>   >(cs, COMPOSITE_EASY_DODGE  , KoCompositeOp::categoryLight());

         add<CFColorBurn<Arg>>(cs, COMPOSITE_BURN, KoCompositeOp::categoryDark()); // using HDR mode as default
         add<CFLinearBurn<Arg, ClampAsFloatHDR>>(cs, COMPOSITE_LINEAR_BURN , KoCompositeOp::categoryDark()); // using HDR mode as default
         add<&cfDarkenOnly<Arg> >(cs, COMPOSITE_DARKEN      , KoCompositeOp::categoryDark());
         add<CFGammaDark<Arg>   >(cs, COMPOSITE_GAMMA_DARK  , KoCompositeOp::categoryDark());
         add<&cfShadeIFSIllusions<Arg>>(cs, COMPOSITE_SHADE_IFS_ILLUSIONS, KoCompositeOp::categoryDark());
         add<&cfFogDarkenIFSIllusions<Arg>>(cs, COMPOSITE_FOG_DARKEN_IFS_ILLUSIONS, KoCompositeOp::categoryDark());
         add<&cfEasyBurn<Arg>   >(cs, COMPOSITE_EASY_BURN   , KoCompositeOp::categoryDark());

         add<&cfAddition<Arg>        >(cs, COMPOSITE_ADD             , KoCompositeOp::categoryArithmetic());
         add<&cfSubtract<Arg>        >(cs, COMPOSITE_SUBTRACT        , KoCompositeOp::categoryArithmetic());
         add<CFInverseSubtract<Arg>  >(cs, COMPOSITE_INVERSE_SUBTRACT, KoCompositeOp::categoryArithmetic());
         add<&cfMultiply<Arg>        >(cs, COMPOSITE_MULT            , KoCompositeOp::categoryArithmetic());
         add<CFDivide<Arg>           >(cs, COMPOSITE_DIVIDE          , KoCompositeOp::categoryArithmetic());

         add<&cfModulo<Arg>               >(cs, COMPOSITE_MOD                , KoCompositeOp::categoryModulo());
         add<&cfModuloContinuous<Arg>     >(cs, COMPOSITE_MOD_CON            , KoCompositeOp::categoryModulo());
         add<&cfDivisiveModulo<Arg>       >(cs, COMPOSITE_DIVISIVE_MOD       , KoCompositeOp::categoryModulo());
         add<&cfDivisiveModuloContinuous<Arg>>(cs, COMPOSITE_DIVISIVE_MOD_CON, KoCompositeOp::categoryModulo());
         add<&cfModuloShift<Arg>          >(cs, COMPOSITE_MODULO_SHIFT       , KoCompositeOp::categoryModulo());
         add<&cfModuloShiftContinuous<Arg>>(cs, COMPOSITE_MODULO_SHIFT_CON   , KoCompositeOp::categoryModulo());

         add<&cfArcTangent<Arg>         >(cs, COMPOSITE_ARC_TANGENT         , KoCompositeOp::categoryNegative());
         add<&cfDifference<Arg>         >(cs, COMPOSITE_DIFF                , KoCompositeOp::categoryNegative());
         add<CFExclusion<Arg>           >(cs, COMPOSITE_EXCLUSION           , KoCompositeOp::categoryNegative());
         add<CFEquivalence<Arg>         >(cs, COMPOSITE_EQUIVALENCE         , KoCompositeOp::categoryNegative());
         add<CFAdditiveSubtractive<Arg> >(cs, COMPOSITE_ADDITIVE_SUBTRACTIVE, KoCompositeOp::categoryNegative());
         add<CFNegation<Arg>            >(cs, COMPOSITE_NEGATION            , KoCompositeOp::categoryNegative());
         
         if constexpr (IsIntegerSpace) {
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
         }

         add<&cfReflect<Arg>>(cs, COMPOSITE_REFLECT, KoCompositeOp::categoryQuadratic());
         add<&cfGlow<Arg>   >(cs, COMPOSITE_GLOW   , KoCompositeOp::categoryQuadratic());
         add<&cfFreeze<Arg> >(cs, COMPOSITE_FREEZE , KoCompositeOp::categoryQuadratic());
         add<&cfHeat<Arg>   >(cs, COMPOSITE_HEAT   , KoCompositeOp::categoryQuadratic());
         add<CFGleat<Arg>   >(cs, COMPOSITE_GLEAT  , KoCompositeOp::categoryQuadratic());
         add<CFHelow<Arg>   >(cs, COMPOSITE_HELOW  , KoCompositeOp::categoryQuadratic());
         add<CFReeze<Arg>   >(cs, COMPOSITE_REEZE  , KoCompositeOp::categoryQuadratic());
         add<CFFrect<Arg>   >(cs, COMPOSITE_FRECT  , KoCompositeOp::categoryQuadratic());
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
    typedef typename Traits::channels_type channels_type;

    static const qint32 red_pos   = Traits::red_pos;
    static const qint32 green_pos = Traits::green_pos;
    static const qint32 blue_pos  = Traits::blue_pos;

    template<typename Functor>
    static void add(KoColorSpace* cs, const QString& id, const QString& category) {
        cs->addCompositeOp(new KoCompositeOpGenericHSLFunctor<Traits, Functor>(cs, id, category));
    }

    static void add(KoColorSpace* cs) {

        cs->addCompositeOp(new KoCompositeOpCopyChannel<Traits,red_pos  >(cs, COMPOSITE_COPY_RED  , KoCompositeOp::categoryMisc()));
        cs->addCompositeOp(new KoCompositeOpCopyChannel<Traits,green_pos>(cs, COMPOSITE_COPY_GREEN, KoCompositeOp::categoryMisc()));
        cs->addCompositeOp(new KoCompositeOpCopyChannel<Traits,blue_pos >(cs, COMPOSITE_COPY_BLUE , KoCompositeOp::categoryMisc()));

        add<CFTangentNormalmap<channels_type> >(cs, COMPOSITE_TANGENT_NORMALMAP  , KoCompositeOp::categoryMisc());
        add<CFReorientedNormalMapCombine<channels_type>>(cs, COMPOSITE_COMBINE_NORMAL, KoCompositeOp::categoryMisc());
        add<CFColor<HSYType, channels_type>>(cs, COMPOSITE_COLOR, KoCompositeOp::categoryHSY());
        add<CFHue<HSYType, channels_type>>(cs, COMPOSITE_HUE, KoCompositeOp::categoryHSY());
        add<CFTint<HSYType, channels_type>>(cs, COMPOSITE_TINT, KoCompositeOp::categoryHSY());
        add<CFSaturation<HSYType, channels_type>>(cs, COMPOSITE_SATURATION, KoCompositeOp::categoryHSY());
        add<CFIncreaseSaturation<HSYType, channels_type>>(cs, COMPOSITE_INC_SATURATION, KoCompositeOp::categoryHSY());
        add<CFDecreaseSaturation<HSYType, channels_type>>(cs, COMPOSITE_DEC_SATURATION, KoCompositeOp::categoryHSY());

        add<CFLightness<HSYType,channels_type>>(cs, COMPOSITE_LUMINIZE, KoCompositeOp::categoryHSY());
        add<CFIncreaseLightness<HSYType,channels_type>>(cs, COMPOSITE_INC_LUMINOSITY, KoCompositeOp::categoryHSY());
        add<CFDecreaseLightness<HSYType,channels_type>>(cs, COMPOSITE_DEC_LUMINOSITY, KoCompositeOp::categoryHSY());


        add<CFDarkerColor<HSYType, channels_type>>(cs, COMPOSITE_DARKER_COLOR, KoCompositeOp::categoryDark()); //darker color as PSD does it//
        add<CFLighterColor<HSYType, channels_type>>(cs, COMPOSITE_LIGHTER_COLOR, KoCompositeOp::categoryLight()); //lighter color as PSD does it//

        add<CFLambertLighting<HSIType,channels_type>>(cs, COMPOSITE_LAMBERT_LIGHTING, KoCompositeOp::categoryMix());
        add<CFLambertLightingGamma2_2<HSIType, channels_type>>(cs, COMPOSITE_LAMBERT_LIGHTING_GAMMA_2_2, KoCompositeOp::categoryMix());

        add<CFColor             <HSIType, channels_type> >(cs, COMPOSITE_COLOR_HSI         , KoCompositeOp::categoryHSI());
        add<CFHue               <HSIType, channels_type> >(cs, COMPOSITE_HUE_HSI           , KoCompositeOp::categoryHSI());
        add<CFSaturation        <HSIType, channels_type> >(cs, COMPOSITE_SATURATION_HSI    , KoCompositeOp::categoryHSI());
        add<CFIncreaseSaturation<HSIType, channels_type> >(cs, COMPOSITE_INC_SATURATION_HSI, KoCompositeOp::categoryHSI());
        add<CFDecreaseSaturation<HSIType, channels_type> >(cs, COMPOSITE_DEC_SATURATION_HSI, KoCompositeOp::categoryHSI());
        add<CFLightness         <HSIType, channels_type> >(cs, COMPOSITE_INTENSITY         , KoCompositeOp::categoryHSI());
        add<CFIncreaseLightness <HSIType, channels_type> >(cs, COMPOSITE_INC_INTENSITY     , KoCompositeOp::categoryHSI());
        add<CFDecreaseLightness <HSIType, channels_type> >(cs, COMPOSITE_DEC_INTENSITY     , KoCompositeOp::categoryHSI());

        add<CFColor             <HSLType, channels_type> >(cs, COMPOSITE_COLOR_HSL         , KoCompositeOp::categoryHSL());
        add<CFHue               <HSLType, channels_type> >(cs, COMPOSITE_HUE_HSL           , KoCompositeOp::categoryHSL());
        add<CFSaturation        <HSLType, channels_type> >(cs, COMPOSITE_SATURATION_HSL    , KoCompositeOp::categoryHSL());
        add<CFIncreaseSaturation<HSLType, channels_type> >(cs, COMPOSITE_INC_SATURATION_HSL, KoCompositeOp::categoryHSL());
        add<CFDecreaseSaturation<HSLType, channels_type> >(cs, COMPOSITE_DEC_SATURATION_HSL, KoCompositeOp::categoryHSL());
        add<CFLightness         <HSLType, channels_type> >(cs, COMPOSITE_LIGHTNESS         , KoCompositeOp::categoryHSL());
        add<CFIncreaseLightness <HSLType, channels_type> >(cs, COMPOSITE_INC_LIGHTNESS     , KoCompositeOp::categoryHSL());
        add<CFDecreaseLightness <HSLType, channels_type> >(cs, COMPOSITE_DEC_LIGHTNESS     , KoCompositeOp::categoryHSL());

        add<CFColor             <HSVType, channels_type> >(cs, COMPOSITE_COLOR_HSV         , KoCompositeOp::categoryHSV());
        add<CFHue               <HSVType, channels_type> >(cs, COMPOSITE_HUE_HSV           , KoCompositeOp::categoryHSV());
        add<CFSaturation        <HSVType, channels_type> >(cs, COMPOSITE_SATURATION_HSV    , KoCompositeOp::categoryHSV());
        add<CFIncreaseSaturation<HSVType, channels_type> >(cs, COMPOSITE_INC_SATURATION_HSV, KoCompositeOp::categoryHSV());
        add<CFDecreaseSaturation<HSVType, channels_type> >(cs, COMPOSITE_DEC_SATURATION_HSV, KoCompositeOp::categoryHSV());
        add<CFLightness         <HSVType, channels_type> >(cs, COMPOSITE_VALUE             , KoCompositeOp::categoryHSV());
        add<CFIncreaseLightness <HSVType, channels_type> >(cs, COMPOSITE_INC_VALUE         , KoCompositeOp::categoryHSV());
        add<CFDecreaseLightness <HSVType, channels_type> >(cs, COMPOSITE_DEC_VALUE         , KoCompositeOp::categoryHSV());

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
        if constexpr (std::is_base_of_v<KoCmykTraits<typename Traits::channels_type>, Traits>) {
            if (useSubtractiveBlendingForCmykColorSpaces()) {
                cs->addCompositeOp(new KoCompositeOpGenericSCAlpha<Traits, compositeFunc, KoSubtractiveBlendingPolicy<Traits>>(cs, id, category));
            } else {
                cs->addCompositeOp(new KoCompositeOpGenericSCAlpha<Traits, compositeFunc, KoAdditiveBlendingPolicy<Traits>>(cs, id, category));
            }
        } else {
            cs->addCompositeOp(new KoCompositeOpGenericSCAlpha<Traits, compositeFunc, KoAdditiveBlendingPolicy<Traits>>(cs, id, category));
        }
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
