/*
 *  SPDX-FileCopyrightText: 2024 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */


#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoCompositeOpRegistry.h>
#include <KoColorModelStandardIds.h>

//#define USE_LOCAL_BUILD_OF_OPS
//#define BUILD_ONLY_BASIC_OPS

#ifdef USE_LOCAL_BUILD_OF_OPS
#include <KoCompositeOpGeneric2.h>
#include <KoCompositeOpFunctions2.h>
#include <compositeops/KoCompositeOpGreater.h>
#else
#include <KoCompositeOpGeneric.h>
#include <KoCompositeOpFunctions.h>
#endif

#include "compositeops/KoColorSpaceBlendingPolicy.h"
#include "compositeops/KoCompositeOpClampPolicy.h"


#ifdef USE_LOCAL_BUILD_OF_OPS

namespace detail {

template<class Traits>
struct OpsStorage {
    typedef typename Traits::channels_type Arg;
    typedef Arg (*CompositeFunc)(Arg, Arg);
    static const qint32 alpha_pos = Traits::alpha_pos;
    static constexpr bool IsIntegerSpace = std::numeric_limits<Arg>::is_integer;

    template<CompositeFunc func>
    void add(const KoColorSpace* cs, const QString& id, const QString& category) {
        if constexpr (std::is_base_of_v<KoCmykTraits<typename Traits::channels_type>, Traits>) {
            if (useSubtractiveBlendingForCmykColorSpaces()) {
                m_ops.insert(id, new KoCompositeOpGenericSC<Traits, func, KoSubtractiveBlendingPolicy<Traits>>(cs, id, category));
            } else {
                m_ops.insert(id,new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, category));
            }
        } else {
            m_ops.insert(id,new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, category));
        }
    }

    template<typename Functor>
    void add(const KoColorSpace* cs, const QString& id, const QString& category) {
        if constexpr (std::is_base_of_v<KoCmykTraits<typename Traits::channels_type>, Traits>) {
            if (useSubtractiveBlendingForCmykColorSpaces()) {
                m_ops.insert(id, new KoCompositeOpGenericSCFunctor<Traits, Functor, KoSubtractiveBlendingPolicy<Traits>>(cs, id, category));
            } else {
                m_ops.insert(id, new KoCompositeOpGenericSCFunctor<Traits, Functor, KoAdditiveBlendingPolicy<Traits>>(cs, id, category));
            }
        } else {
            m_ops.insert(id, new KoCompositeOpGenericSCFunctor<Traits, Functor, KoAdditiveBlendingPolicy<Traits>>(cs, id, category));
        }
    }

    void add(const KoColorSpace* cs) {
        using namespace KoCompositeOpClampPolicy;

        if constexpr (std::is_base_of_v<KoCmykTraits<typename Traits::channels_type>, Traits>) {
            if (useSubtractiveBlendingForCmykColorSpaces()) {
                m_ops.insert(COMPOSITE_GREATER, new KoCompositeOpGreater<Traits, KoSubtractiveBlendingPolicy<Traits>>(cs));
            } else {
                m_ops.insert(COMPOSITE_GREATER, new KoCompositeOpGreater<Traits, KoAdditiveBlendingPolicy<Traits>>(cs));
            }
        } else {
            m_ops.insert(COMPOSITE_GREATER, new KoCompositeOpGreater<Traits, KoAdditiveBlendingPolicy<Traits>>(cs));
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

#ifndef BUILD_ONLY_BASIC_OPS
        add<&cfInterpolation<Arg> >(cs, COMPOSITE_INTERPOLATION , KoCompositeOp::categoryMix());
        add<&cfInterpolationB<Arg>>(cs, COMPOSITE_INTERPOLATIONB, KoCompositeOp::categoryMix());
        add<CFPenumbraA<Arg>      >(cs, COMPOSITE_PENUMBRAA     , KoCompositeOp::categoryMix());
        add<CFPenumbraB<Arg>      >(cs, COMPOSITE_PENUMBRAB     , KoCompositeOp::categoryMix());
        add<&cfPenumbraC<Arg>     >(cs, COMPOSITE_PENUMBRAC     , KoCompositeOp::categoryMix());
        add<&cfPenumbraD<Arg>     >(cs, COMPOSITE_PENUMBRAD     , KoCompositeOp::categoryMix());
#endif
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

#ifndef BUILD_ONLY_BASIC_OPS
        add<&cfModulo<Arg>               >(cs, COMPOSITE_MOD                , KoCompositeOp::categoryModulo());
        add<&cfModuloContinuous<Arg>     >(cs, COMPOSITE_MOD_CON            , KoCompositeOp::categoryModulo());
        add<&cfDivisiveModulo<Arg>       >(cs, COMPOSITE_DIVISIVE_MOD       , KoCompositeOp::categoryModulo());
        add<&cfDivisiveModuloContinuous<Arg>>(cs, COMPOSITE_DIVISIVE_MOD_CON, KoCompositeOp::categoryModulo());
        add<&cfModuloShift<Arg>          >(cs, COMPOSITE_MODULO_SHIFT       , KoCompositeOp::categoryModulo());
        add<&cfModuloShiftContinuous<Arg>>(cs, COMPOSITE_MODULO_SHIFT_CON   , KoCompositeOp::categoryModulo());
#endif
        add<&cfArcTangent<Arg>         >(cs, COMPOSITE_ARC_TANGENT         , KoCompositeOp::categoryNegative());
        add<&cfDifference<Arg>         >(cs, COMPOSITE_DIFF                , KoCompositeOp::categoryNegative());
        add<CFExclusion<Arg>           >(cs, COMPOSITE_EXCLUSION           , KoCompositeOp::categoryNegative());
        add<CFEquivalence<Arg>         >(cs, COMPOSITE_EQUIVALENCE         , KoCompositeOp::categoryNegative());
        add<CFAdditiveSubtractive<Arg> >(cs, COMPOSITE_ADDITIVE_SUBTRACTIVE, KoCompositeOp::categoryNegative());
        add<CFNegation<Arg>            >(cs, COMPOSITE_NEGATION            , KoCompositeOp::categoryNegative());

#ifndef BUILD_ONLY_BASIC_OPS
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
#endif
    }

    bool isInitialized() const {
        return !m_ops.isEmpty();
    }

    const KoCompositeOp* compositeOp(const QString &id) {
        return m_ops.value(id);
    }

private:

    QMap<QString, KoCompositeOp*> m_ops;
};

template<class Traits>
struct RgbOpsStorage {
    typedef float Arg;

    static const qint32 red_pos   = Traits::red_pos;
    static const qint32 green_pos = Traits::green_pos;
    static const qint32 blue_pos  = Traits::blue_pos;

    template<void compositeFunc(Arg, Arg, Arg, Arg&, Arg&, Arg&)>

    void add(const KoColorSpace* cs, const QString& id, const QString& category) {
        m_ops.insert(id, new KoCompositeOpGenericHSL<Traits, compositeFunc>(cs, id, category));
    }

    void add(const KoColorSpace* cs) {

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

        add<&cfLambertLighting         <HSIType,Arg>   >(cs, COMPOSITE_LAMBERT_LIGHTING, KoCompositeOp::categoryMix());
        add<&cfLambertLightingGamma2_2 <HSIType, Arg>   >(cs, COMPOSITE_LAMBERT_LIGHTING_GAMMA_2_2, KoCompositeOp::categoryMix());

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

    bool isInitialized() const {
        return !m_ops.isEmpty();
    }

    const KoCompositeOp* compositeOp(const QString &id) {
        return m_ops.value(id);
    }

private:

    QMap<QString, KoCompositeOp*> m_ops;
};

OpsStorage<KoRgbU16Traits> sU16Ops;
OpsStorage<KoRgbF16Traits> sF16Ops;
OpsStorage<KoRgbF32Traits> sF32Ops;

RgbOpsStorage<KoRgbU16Traits> sU16RgbOps;
RgbOpsStorage<KoRgbF16Traits> sF16RgbOps;
RgbOpsStorage<KoRgbF32Traits> sF32RgbOps;

}

#endif /* USE_LOCAL_BUILD_OF_OPS */

const KoCompositeOp* createOp(const KoColorSpace *cs, const QString &id, bool isHDR)
{
    using namespace KoCompositeOpClampPolicy;

    const KoCompositeOp *op = nullptr;

#ifdef USE_LOCAL_BUILD_OF_OPS
    const bool isFloat = cs->colorDepthId() == Float32BitsColorDepthID || cs->colorDepthId() == Float16BitsColorDepthID;
    QString effectiveId = id;

    if (isHDR && isFloat) {
        if (id == COMPOSITE_DODGE) {
            effectiveId = COMPOSITE_DODGE_HDR;
        } else if (id == COMPOSITE_VIVID_LIGHT) {
            effectiveId = COMPOSITE_VIVID_LIGHT_HDR;
        } else if (id == COMPOSITE_HARD_MIX) {
            effectiveId = COMPOSITE_HARD_MIX_HDR;
        } else if (id == COMPOSITE_HARD_OVERLAY) {
            effectiveId = COMPOSITE_HARD_OVERLAY_HDR;
        }
    }

    if (cs->colorDepthId() == Integer16BitsColorDepthID) {
        if (!detail::sU16Ops.isInitialized()) {
            detail::sU16Ops.add(cs);
        }

        op = detail::sU16Ops.compositeOp(effectiveId);

        if (!op) {
            if (!detail::sU16RgbOps.isInitialized()) {
                detail::sU16RgbOps.add(cs);
            }

            op = detail::sU16RgbOps.compositeOp(effectiveId);
        }

    } else if (cs->colorDepthId() == Float16BitsColorDepthID) {
        if (!detail::sF16Ops.isInitialized()) {
            detail::sF16Ops.add(cs);
        }

        op = detail::sF16Ops.compositeOp(effectiveId);

        if (!op) {
            if (!detail::sF16RgbOps.isInitialized()) {
                detail::sF16RgbOps.add(cs);
            }

            op = detail::sF16RgbOps.compositeOp(effectiveId);
        }

    } else if (cs->colorDepthId() == Float32BitsColorDepthID) {
        if (!detail::sF32Ops.isInitialized()) {
            detail::sF32Ops.add(cs);
        }

        op = detail::sF32Ops.compositeOp(effectiveId);

        if (!op) {
            if (!detail::sF32RgbOps.isInitialized()) {
                detail::sF32RgbOps.add(cs);
            }

            op = detail::sF32RgbOps.compositeOp(effectiveId);
        }
    }
#else

    /**
     * Some of the blendmodes exist only "in theory". We test them only to make sure
     * they are not actually needed in real life (exactly the same in SDR range and
     * SDR/negative-preserving).
     */
    if (id == COMPOSITE_LINEAR_BURN && !isHDR) { // SDR-only, HDR is the default
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            using func = CFLinearBurn<float, ClampAsFloatSDR>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryDark());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            using func = CFLinearBurn<quint16, ClampAsInteger>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryDark());
        }
    } else if (id == COMPOSITE_DODGE) {
        const bool isFloat = cs->colorDepthId() == Float32BitsColorDepthID || cs->colorDepthId() == Float16BitsColorDepthID;
        const QString newId = isHDR && isFloat ? COMPOSITE_DODGE_HDR : COMPOSITE_DODGE;
        op = cs->compositeOp(newId);
    } else if (id == COMPOSITE_VIVID_LIGHT) {
        const bool isFloat = cs->colorDepthId() == Float32BitsColorDepthID || cs->colorDepthId() == Float16BitsColorDepthID;
        const QString newId = isHDR && isFloat ? COMPOSITE_VIVID_LIGHT_HDR : COMPOSITE_VIVID_LIGHT;
        op = cs->compositeOp(newId);
    } else if (id == COMPOSITE_HARD_MIX) {
        const bool isFloat = cs->colorDepthId() == Float32BitsColorDepthID || cs->colorDepthId() == Float16BitsColorDepthID;
        const QString newId = isHDR && isFloat ? COMPOSITE_HARD_MIX_HDR : COMPOSITE_HARD_MIX;
        op = cs->compositeOp(newId);
    } else if (id == COMPOSITE_HARD_OVERLAY) {
        const bool isFloat = cs->colorDepthId() == Float32BitsColorDepthID || cs->colorDepthId() == Float16BitsColorDepthID;
        const QString newId = isHDR && isFloat? COMPOSITE_HARD_OVERLAY_HDR : COMPOSITE_HARD_OVERLAY;
        op = cs->compositeOp(newId);
    } else {
        op = cs->compositeOp(id);
    }

#endif

    if (!op) {
        qWarning() << "WARNING: fall back to the upstream blendmode" << id;
        op = cs->compositeOp(id);
    }

    return op;
}
