/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2011 Silvio Heinrich <plassy@web.de>
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
};

template<class Traits>
struct AddGeneralOps<Traits, true>
{
     typedef typename Traits::channels_type Arg;
     typedef Arg (*CompositeFunc)(Arg, Arg);
     static const qint32 alpha_pos = Traits::alpha_pos;

     template<CompositeFunc func>
     static void add(KoColorSpace* cs, const QString& id, const QString& description, const QString& category) {
         cs->addCompositeOp(new KoCompositeOpGenericSC<Traits, func>(cs, id, description, category));
     }

     static void add(KoColorSpace* cs) {
         cs->addCompositeOp(OptimizedOpsSelector<Traits>::createOverOp(cs));
         cs->addCompositeOp(OptimizedOpsSelector<Traits>::createAlphaDarkenOp(cs));
         cs->addCompositeOp(new KoCompositeOpCopy2<Traits>(cs));
         cs->addCompositeOp(new KoCompositeOpErase<Traits>(cs));
         cs->addCompositeOp(new KoCompositeOpBehind<Traits>(cs));
         cs->addCompositeOp(new KoCompositeOpDestinationIn<Traits>(cs));
         cs->addCompositeOp(new KoCompositeOpDestinationAtop<Traits>(cs));
         cs->addCompositeOp(new KoCompositeOpGreater<Traits>(cs));

         add<&cfOverlay<Arg>       >(cs, COMPOSITE_OVERLAY       , i18n("Overlay")       , KoCompositeOp::categoryMix());
         add<&cfGrainMerge<Arg>    >(cs, COMPOSITE_GRAIN_MERGE   , i18n("Grain Merge")   , KoCompositeOp::categoryMix());
         add<&cfGrainExtract<Arg>  >(cs, COMPOSITE_GRAIN_EXTRACT , i18n("Grain Extract") , KoCompositeOp::categoryMix());
         add<&cfHardMix<Arg>       >(cs, COMPOSITE_HARD_MIX      , i18n("Hard Mix")      , KoCompositeOp::categoryMix());
         add<&cfHardMixPhotoshop<Arg>>(cs, COMPOSITE_HARD_MIX_PHOTOSHOP, i18n("Hard Mix (Photoshop)")      , KoCompositeOp::categoryMix());
         add<&cfGeometricMean<Arg> >(cs, COMPOSITE_GEOMETRIC_MEAN, i18n("Geometric Mean"), KoCompositeOp::categoryMix());
         add<&cfParallel<Arg>      >(cs, COMPOSITE_PARALLEL      , i18n("Parallel")      , KoCompositeOp::categoryMix());
         add<&cfAllanon<Arg>       >(cs, COMPOSITE_ALLANON       , i18n("Allanon")       , KoCompositeOp::categoryMix());
         add<&cfHardOverlay<Arg>   >(cs, COMPOSITE_HARD_OVERLAY  , i18n("Hard Overlay")  , KoCompositeOp::categoryMix());
         add<&cfInterpolation<Arg> >(cs, COMPOSITE_INTERPOLATION , i18n("Interpolation") , KoCompositeOp::categoryMix());
         add<&cfInterpolationB<Arg> >(cs, COMPOSITE_INTERPOLATIONB , i18n("Interpolation - 2X") , KoCompositeOp::categoryMix());
         add<&cfPenumbraA<Arg>     >(cs, COMPOSITE_PENUMBRAA     , i18n("Penumbra A")    , KoCompositeOp::categoryMix());
         add<&cfPenumbraB<Arg>     >(cs, COMPOSITE_PENUMBRAB     , i18n("Penumbra B")    , KoCompositeOp::categoryMix());
         add<&cfPenumbraC<Arg>     >(cs, COMPOSITE_PENUMBRAC     , i18n("Penumbra C")    , KoCompositeOp::categoryMix());
         add<&cfPenumbraD<Arg>     >(cs, COMPOSITE_PENUMBRAD     , i18n("Penumbra D")    , KoCompositeOp::categoryMix());

         add<&cfScreen<Arg>      >(cs, COMPOSITE_SCREEN      , i18n("Screen")      , KoCompositeOp::categoryLight());
         add<&cfColorDodge<Arg>  >(cs, COMPOSITE_DODGE       , i18n("Color Dodge") , KoCompositeOp::categoryLight());
         add<&cfAddition<Arg>    >(cs, COMPOSITE_LINEAR_DODGE, i18n("Linear Dodge"), KoCompositeOp::categoryLight());
         add<&cfLightenOnly<Arg> >(cs, COMPOSITE_LIGHTEN     , i18n("Lighten")     , KoCompositeOp::categoryLight());
         add<&cfHardLight<Arg>   >(cs, COMPOSITE_HARD_LIGHT  , i18n("Hard Light")  , KoCompositeOp::categoryLight());
         add<&cfSoftLightIFSIllusions<Arg>>(cs, COMPOSITE_SOFT_LIGHT_IFS_ILLUSIONS, i18n("Soft Light (IFS Illusions)")  , KoCompositeOp::categoryLight());
         add<&cfSoftLightPegtopDelphi<Arg>>(cs, COMPOSITE_SOFT_LIGHT_PEGTOP_DELPHI, i18n("Soft Light (Pegtop-Delphi)")  , KoCompositeOp::categoryLight());
         add<&cfSoftLightSvg<Arg>   >(cs, COMPOSITE_SOFT_LIGHT_SVG, i18n("Soft Light (SVG)")  , KoCompositeOp::categoryLight());
         add<&cfSoftLight<Arg>   >(cs, COMPOSITE_SOFT_LIGHT_PHOTOSHOP, i18n("Soft Light (Photoshop)")  , KoCompositeOp::categoryLight());
         add<&cfGammaLight<Arg>  >(cs, COMPOSITE_GAMMA_LIGHT , i18n("Gamma Light") , KoCompositeOp::categoryLight());
         add<&cfGammaIllumination<Arg>  >(cs, COMPOSITE_GAMMA_ILLUMINATION , i18n("Gamma Illumination") , KoCompositeOp::categoryLight());
         add<&cfVividLight<Arg>  >(cs, COMPOSITE_VIVID_LIGHT , i18n("Vivid Light") , KoCompositeOp::categoryLight());
         add<&cfFlatLight<Arg>  >(cs, COMPOSITE_FLAT_LIGHT   , i18n("Flat Light") , KoCompositeOp::categoryLight());
         add<&cfPinLight<Arg>    >(cs, COMPOSITE_PIN_LIGHT   , i18n("Pin Light")   , KoCompositeOp::categoryLight());
         add<&cfLinearLight<Arg> >(cs, COMPOSITE_LINEAR_LIGHT, i18n("Linear Light"), KoCompositeOp::categoryLight());
         add<&cfPNormA<Arg>       >(cs, COMPOSITE_PNORM_A       , i18n("P-Norm A")      , KoCompositeOp::categoryLight());
         add<&cfPNormB<Arg>       >(cs, COMPOSITE_PNORM_B       , i18n("P-Norm B")      , KoCompositeOp::categoryLight());
         add<&cfSuperLight<Arg> >(cs, COMPOSITE_SUPER_LIGHT  , i18n("Super Light") , KoCompositeOp::categoryLight());
         add<&cfTintIFSIllusions<Arg>     >(cs, COMPOSITE_TINT_IFS_ILLUSIONS     , i18n("Tint (IFS Illusions)")    , KoCompositeOp::categoryLight());
         add<&cfFogLightenIFSIllusions<Arg>     >(cs, COMPOSITE_FOG_LIGHTEN_IFS_ILLUSIONS     , i18n("Fog Lighten (IFS Illusions)")    , KoCompositeOp::categoryLight());
         add<&cfEasyDodge<Arg>  >(cs, COMPOSITE_EASY_DODGE       , i18n("Easy Dodge") , KoCompositeOp::categoryLight());

         add<&cfColorBurn<Arg>  >(cs, COMPOSITE_BURN        , i18n("Color Burn") , KoCompositeOp::categoryDark());
         add<&cfLinearBurn<Arg> >(cs, COMPOSITE_LINEAR_BURN , i18n("Linear Burn"), KoCompositeOp::categoryDark());
         add<&cfDarkenOnly<Arg> >(cs, COMPOSITE_DARKEN      , i18n("Darken")     , KoCompositeOp::categoryDark());
         add<&cfGammaDark<Arg>  >(cs, COMPOSITE_GAMMA_DARK  , i18n("Gamma Dark") , KoCompositeOp::categoryDark());
         add<&cfShadeIFSIllusions<Arg>     >(cs, COMPOSITE_SHADE_IFS_ILLUSIONS     , i18n("Shade (IFS_Illusions)")    , KoCompositeOp::categoryDark());
         add<&cfFogDarkenIFSIllusions<Arg>     >(cs, COMPOSITE_FOG_DARKEN_IFS_ILLUSIONS     , i18n("Fog Darken (IFS Illusions)")    , KoCompositeOp::categoryDark());
         add<&cfEasyBurn<Arg>  >(cs, COMPOSITE_EASY_BURN        , i18n("Easy Burn") , KoCompositeOp::categoryDark());

         add<&cfAddition<Arg>        >(cs, COMPOSITE_ADD             , i18n("Addition")         , KoCompositeOp::categoryArithmetic());
         add<&cfSubtract<Arg>        >(cs, COMPOSITE_SUBTRACT        , i18n("Subtract")         , KoCompositeOp::categoryArithmetic());
         add<&cfInverseSubtract<Arg> >(cs, COMPOSITE_INVERSE_SUBTRACT, i18n("Inversed-Subtract"), KoCompositeOp::categoryArithmetic());
         add<&cfMultiply<Arg>        >(cs, COMPOSITE_MULT            , i18n("Multiply")         , KoCompositeOp::categoryArithmetic());
         add<&cfDivide<Arg>          >(cs, COMPOSITE_DIVIDE          , i18n("Divide")           , KoCompositeOp::categoryArithmetic());

         add<&cfModulo<Arg>               >(cs, COMPOSITE_MOD                    , i18n("Modulo")                   , KoCompositeOp::categoryModulo());
         add<&cfModuloContinuous<Arg>     >(cs, COMPOSITE_MOD_CON                , i18n("Modulo - Continuous")      , KoCompositeOp::categoryModulo());
         add<&cfDivisiveModulo<Arg>       >(cs, COMPOSITE_DIVISIVE_MOD           , i18n("Divisive Modulo")          , KoCompositeOp::categoryModulo());
         add<&cfDivisiveModuloContinuous<Arg> >(cs, COMPOSITE_DIVISIVE_MOD_CON   , i18n("Divisive Modulo - Continuous")   , KoCompositeOp::categoryModulo());
         add<&cfModuloShift<Arg>          >(cs, COMPOSITE_MODULO_SHIFT           , i18n("Modulo Shift")             , KoCompositeOp::categoryModulo());
         add<&cfModuloShiftContinuous<Arg> >(cs, COMPOSITE_MODULO_SHIFT_CON      , i18n("Modulo Shift - Continuous")      , KoCompositeOp::categoryModulo());

         add<&cfArcTangent<Arg>           >(cs, COMPOSITE_ARC_TANGENT          , i18n("Arcus Tangent")        , KoCompositeOp::categoryNegative());
         add<&cfDifference<Arg>           >(cs, COMPOSITE_DIFF                 , i18n("Difference")           , KoCompositeOp::categoryNegative());
         add<&cfExclusion<Arg>            >(cs, COMPOSITE_EXCLUSION            , i18n("Exclusion")            , KoCompositeOp::categoryNegative());
         add<&cfEquivalence<Arg>          >(cs, COMPOSITE_EQUIVALENCE          , i18n("Equivalence")          , KoCompositeOp::categoryNegative());
         add<&cfAdditiveSubtractive<Arg>  >(cs, COMPOSITE_ADDITIVE_SUBTRACTIVE , i18n("Additive-Subtractive") , KoCompositeOp::categoryNegative());
         add<&cfNegation<Arg>             >(cs, COMPOSITE_NEGATION             , i18n("Negation")             , KoCompositeOp::categoryNegative());
         
         add<&cfXor<Arg>                  >(cs, COMPOSITE_XOR                  , i18n("XOR")                  , KoCompositeOp::categoryBinary());
         add<&cfOr<Arg>                   >(cs, COMPOSITE_OR                   , i18n("OR")                   , KoCompositeOp::categoryBinary());
         add<&cfAnd<Arg>                  >(cs, COMPOSITE_AND                  , i18n("AND")                  , KoCompositeOp::categoryBinary());
         add<&cfNand<Arg>                 >(cs, COMPOSITE_NAND                 , i18n("NAND")                 , KoCompositeOp::categoryBinary());
         add<&cfNor<Arg>                  >(cs, COMPOSITE_NOR                  , i18n("NOR")                  , KoCompositeOp::categoryBinary());
         add<&cfXnor<Arg>                 >(cs, COMPOSITE_XNOR                 , i18n("XNOR")                 , KoCompositeOp::categoryBinary());
         add<&cfImplies<Arg>              >(cs, COMPOSITE_IMPLICATION          , i18n("IMPLICATION")          , KoCompositeOp::categoryBinary());
         add<&cfNotImplies<Arg>           >(cs, COMPOSITE_NOT_IMPLICATION      , i18n("NOT IMPLICATION")      , KoCompositeOp::categoryBinary());
         add<&cfConverse<Arg>             >(cs, COMPOSITE_CONVERSE             , i18n("CONVERSE")             , KoCompositeOp::categoryBinary());
         add<&cfNotConverse<Arg>          >(cs, COMPOSITE_NOT_CONVERSE         , i18n("NOT CONVERSE")         , KoCompositeOp::categoryBinary());

         add<&cfReflect<Arg> >(cs, COMPOSITE_REFLECT                         , i18n("Reflect")                , KoCompositeOp::categoryQuadratic());
         add<&cfGlow<Arg>    >(cs, COMPOSITE_GLOW                            , i18n("Glow")                   , KoCompositeOp::categoryQuadratic());
         add<&cfFreeze<Arg>  >(cs, COMPOSITE_FREEZE                          , i18n("Freeze")                 , KoCompositeOp::categoryQuadratic());
         add<&cfHeat<Arg>    >(cs, COMPOSITE_HEAT                            , i18n("Heat")                   , KoCompositeOp::categoryQuadratic());
         add<&cfGleat<Arg>   >(cs, COMPOSITE_GLEAT                           , i18n("Glow-Heat")                  , KoCompositeOp::categoryQuadratic());
         add<&cfHelow<Arg>   >(cs, COMPOSITE_HELOW                           , i18n("Heat-Glow")                  , KoCompositeOp::categoryQuadratic());
         add<&cfReeze<Arg>   >(cs, COMPOSITE_REEZE                           , i18n("Reflect-Freeze")                  , KoCompositeOp::categoryQuadratic());
         add<&cfFrect<Arg>   >(cs, COMPOSITE_FRECT                           , i18n("Freeze-Reflect")                  , KoCompositeOp::categoryQuadratic());
         add<&cfFhyrd<Arg>   >(cs, COMPOSITE_FHYRD                           , i18n("Heat-Glow & Freeze-Reflect Hybrid")     , KoCompositeOp::categoryQuadratic());

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

    static void add(KoColorSpace* cs, const QString& id, const QString& description, const QString& category) {
        cs->addCompositeOp(new KoCompositeOpGenericHSL<Traits, compositeFunc>(cs, id, description, category));
    }

    static void add(KoColorSpace* cs) {

        cs->addCompositeOp(new KoCompositeOpCopyChannel<Traits,red_pos  >(cs, COMPOSITE_COPY_RED  , i18n("Copy Red")  , KoCompositeOp::categoryMisc()));
        cs->addCompositeOp(new KoCompositeOpCopyChannel<Traits,green_pos>(cs, COMPOSITE_COPY_GREEN, i18n("Copy Green"), KoCompositeOp::categoryMisc()));
        cs->addCompositeOp(new KoCompositeOpCopyChannel<Traits,blue_pos >(cs, COMPOSITE_COPY_BLUE , i18n("Copy Blue") , KoCompositeOp::categoryMisc()));
        add<&cfTangentNormalmap  <HSYType,Arg> >(cs, COMPOSITE_TANGENT_NORMALMAP  , i18n("Tangent Normalmap")  , KoCompositeOp::categoryMisc());
        add<&cfReorientedNormalMapCombine <HSYType, Arg> >(cs, COMPOSITE_COMBINE_NORMAL, i18n("Combine Normal Maps"), KoCompositeOp::categoryMisc());

        add<&cfColor             <HSYType,Arg> >(cs, COMPOSITE_COLOR         , i18n("Color")              , KoCompositeOp::categoryHSY());
        add<&cfHue               <HSYType,Arg> >(cs, COMPOSITE_HUE           , i18n("Hue")                , KoCompositeOp::categoryHSY());
        add<&cfSaturation        <HSYType,Arg> >(cs, COMPOSITE_SATURATION    , i18n("Saturation")         , KoCompositeOp::categoryHSY());
        add<&cfIncreaseSaturation<HSYType,Arg> >(cs, COMPOSITE_INC_SATURATION, i18n("Increase Saturation"), KoCompositeOp::categoryHSY());
        add<&cfDecreaseSaturation<HSYType,Arg> >(cs, COMPOSITE_DEC_SATURATION, i18n("Decrease Saturation"), KoCompositeOp::categoryHSY());
        add<&cfLightness         <HSYType,Arg> >(cs, COMPOSITE_LUMINIZE      , i18n("Luminosity")         , KoCompositeOp::categoryHSY());
        add<&cfIncreaseLightness <HSYType,Arg> >(cs, COMPOSITE_INC_LUMINOSITY, i18n("Increase Luminosity"), KoCompositeOp::categoryHSY());
        add<&cfDecreaseLightness <HSYType,Arg> >(cs, COMPOSITE_DEC_LUMINOSITY, i18n("Decrease Luminosity"), KoCompositeOp::categoryHSY());
        add<&cfDarkerColor <HSYType,Arg> >(cs, COMPOSITE_DARKER_COLOR, i18n("Darker Color"), KoCompositeOp::categoryDark());//darker color as PSD does it//
        add<&cfLighterColor <HSYType,Arg> >(cs, COMPOSITE_LIGHTER_COLOR, i18n("Lighter Color"), KoCompositeOp::categoryLight());//lighter color as PSD does it//

        add<&cfColor             <HSIType,Arg> >(cs, COMPOSITE_COLOR_HSI         , i18n("Color HSI")              , KoCompositeOp::categoryHSI());
        add<&cfHue               <HSIType,Arg> >(cs, COMPOSITE_HUE_HSI           , i18n("Hue HSI")                , KoCompositeOp::categoryHSI());
        add<&cfSaturation        <HSIType,Arg> >(cs, COMPOSITE_SATURATION_HSI    , i18n("Saturation HSI")         , KoCompositeOp::categoryHSI());
        add<&cfIncreaseSaturation<HSIType,Arg> >(cs, COMPOSITE_INC_SATURATION_HSI, i18n("Increase Saturation HSI"), KoCompositeOp::categoryHSI());
        add<&cfDecreaseSaturation<HSIType,Arg> >(cs, COMPOSITE_DEC_SATURATION_HSI, i18n("Decrease Saturation HSI"), KoCompositeOp::categoryHSI());
        add<&cfLightness         <HSIType,Arg> >(cs, COMPOSITE_INTENSITY         , i18n("Intensity")              , KoCompositeOp::categoryHSI());
        add<&cfIncreaseLightness <HSIType,Arg> >(cs, COMPOSITE_INC_INTENSITY     , i18n("Increase Intensity")     , KoCompositeOp::categoryHSI());
        add<&cfDecreaseLightness <HSIType,Arg> >(cs, COMPOSITE_DEC_INTENSITY     , i18n("Decrease Intensity")     , KoCompositeOp::categoryHSI());

        add<&cfColor             <HSLType,Arg> >(cs, COMPOSITE_COLOR_HSL         , i18n("Color HSL")              , KoCompositeOp::categoryHSL());
        add<&cfHue               <HSLType,Arg> >(cs, COMPOSITE_HUE_HSL           , i18n("Hue HSL")                , KoCompositeOp::categoryHSL());
        add<&cfSaturation        <HSLType,Arg> >(cs, COMPOSITE_SATURATION_HSL    , i18n("Saturation HSL")         , KoCompositeOp::categoryHSL());
        add<&cfIncreaseSaturation<HSLType,Arg> >(cs, COMPOSITE_INC_SATURATION_HSL, i18n("Increase Saturation HSL"), KoCompositeOp::categoryHSL());
        add<&cfDecreaseSaturation<HSLType,Arg> >(cs, COMPOSITE_DEC_SATURATION_HSL, i18n("Decrease Saturation HSL"), KoCompositeOp::categoryHSL());
        add<&cfLightness         <HSLType,Arg> >(cs, COMPOSITE_LIGHTNESS         , i18n("Lightness")              , KoCompositeOp::categoryHSL());
        add<&cfIncreaseLightness <HSLType,Arg> >(cs, COMPOSITE_INC_LIGHTNESS     , i18n("Increase Lightness")     , KoCompositeOp::categoryHSL());
        add<&cfDecreaseLightness <HSLType,Arg> >(cs, COMPOSITE_DEC_LIGHTNESS     , i18n("Decrease Lightness")     , KoCompositeOp::categoryHSL());

        add<&cfColor             <HSVType,Arg> >(cs, COMPOSITE_COLOR_HSV         , i18n("Color HSV")              , KoCompositeOp::categoryHSV());
        add<&cfHue               <HSVType,Arg> >(cs, COMPOSITE_HUE_HSV           , i18n("Hue HSV")                , KoCompositeOp::categoryHSV());
        add<&cfSaturation        <HSVType,Arg> >(cs, COMPOSITE_SATURATION_HSV    , i18n("Saturation HSV")         , KoCompositeOp::categoryHSV());
        add<&cfIncreaseSaturation<HSVType,Arg> >(cs, COMPOSITE_INC_SATURATION_HSV, i18n("Increase Saturation HSV"), KoCompositeOp::categoryHSV());
        add<&cfDecreaseSaturation<HSVType,Arg> >(cs, COMPOSITE_DEC_SATURATION_HSV, i18n("Decrease Saturation HSV"), KoCompositeOp::categoryHSV());
        add<&cfLightness         <HSVType,Arg> >(cs, COMPOSITE_VALUE             , i18nc("HSV Value","Value")                  , KoCompositeOp::categoryHSV());
        add<&cfIncreaseLightness <HSVType,Arg> >(cs, COMPOSITE_INC_VALUE         , i18n("Increase Value")         , KoCompositeOp::categoryHSV());
        add<&cfDecreaseLightness <HSVType,Arg> >(cs, COMPOSITE_DEC_VALUE         , i18n("Decrease Value")         , KoCompositeOp::categoryHSV());
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

    _Private::AddGeneralOps<_Traits_, useGeneralOps>::add(cs);
    _Private::AddRGBOps    <_Traits_, useRGBOps    >::add(cs);
}

template<class _Traits_>
KoCompositeOp* createAlphaDarkenCompositeOp(const KoColorSpace *cs)
{
    return _Private::OptimizedOpsSelector<_Traits_>::createAlphaDarkenOp(cs);
}

#endif
