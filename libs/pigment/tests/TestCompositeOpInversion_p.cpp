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

#include <KoCompositeOpGeneric2.h>
#include <KoCompositeOpFunctions2.h>
#include "compositeops/KoColorSpaceBlendingPolicy.h"


const KoCompositeOp* createOp(const KoColorSpace *cs, const QString &id, bool isHDR)
{
    const KoCompositeOp *op = nullptr;

    if (id == COMPOSITE_DODGE) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            if (isHDR) {
                using func = CFColorDodge<float, ClampPolicyFloatHDR>;
                op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryLight());
            } else {
                using func = CFColorDodge<float, ClampPolicyFloatSDR>;
                op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryLight());
            }
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            using func = CFColorDodge<quint16, ClampPolicyInteger>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryLight());
        }
    } else if (id == COMPOSITE_BURN) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            if (isHDR) {
                using func = CFColorBurn<float, ClampPolicyFloatHDR>;
                op = new KoCompositeOpGenericSCFunctor<Traits, func,
                                                       KoAdditiveBlendingPolicy<Traits>>
                    (cs, id, KoCompositeOp::categoryDark());
            } else {
                using func = CFColorBurn<float, ClampPolicyFloatSDR>;
                op = new KoCompositeOpGenericSCFunctor<Traits, func,
                                                       KoAdditiveBlendingPolicy<Traits>>
                    (cs, id, KoCompositeOp::categoryDark());
            }
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            using func = CFColorBurn<quint16, ClampPolicyInteger>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryDark());
        }
    } else if (id == COMPOSITE_LINEAR_BURN) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            if (isHDR) {
                using func = CFLinearBurn<float, ClampPolicyFloatHDR>;
                op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryDark());
            } else {
                using func = CFLinearBurn<float, ClampPolicyFloatSDR>;
                op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryDark());
            }
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            using func = CFLinearBurn<quint16, ClampPolicyInteger>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryDark());
        }

    } else if (id == COMPOSITE_HARD_LIGHT) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            using func = CFHardLight<float>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryDark());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            using func = CFHardLight<quint16>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_SOFT_LIGHT_PHOTOSHOP) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            using func = CFSoftLight<float>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryDark());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            using func = CFSoftLight<quint16>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_SOFT_LIGHT_SVG) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            using func = CFSoftLightSvg<float>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryDark());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            using func = CFSoftLightSvg<quint16>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_VIVID_LIGHT) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            if (isHDR) {
                using func = CFVividLight<float, ClampPolicyFloatHDR>;
                op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryDark());
            } else {
                using func = CFVividLight<float, ClampPolicyFloatSDR>;
                op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryDark());
            }
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            using func = CFVividLight<quint16, ClampPolicyInteger>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_HARD_OVERLAY) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            if (isHDR) {
                using func = CFHardOverlay<float, ClampPolicyFloatHDR>;
                op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryDark());
            } else {
                using func = CFHardOverlay<float, ClampPolicyFloatSDR>;
                op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryDark());
            }
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            using func = CFHardOverlay<quint16, ClampPolicyInteger>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_PIN_LIGHT) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            if (isHDR) {
                using func = CFPinLight<float, ClampPolicyFloatHDR>;
                op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryDark());
            } else {
                using func = CFPinLight<float, ClampPolicyFloatSDR>;
                op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryDark());
            }
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            using func = CFPinLight<quint16, ClampPolicyInteger>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_ADD) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            constexpr auto func = &cfAddition<float>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            constexpr auto func = &cfAddition<quint16>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_SUBTRACT) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            constexpr auto func = &cfSubtract<float>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            constexpr auto func = &cfSubtract<quint16>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_INVERSE_SUBTRACT) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            constexpr auto func = &cfInverseSubtract<float>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            constexpr auto func = &cfInverseSubtract<quint16>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_MULT) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            constexpr auto func = &cfMultiply<float>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            constexpr auto func = &cfMultiply<quint16>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_DIVIDE) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            constexpr auto func = &cfDivide<float>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            constexpr auto func = &cfDivide<quint16>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_EXCLUSION) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            constexpr auto func = &cfExclusion<float>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            constexpr auto func = &cfExclusion<quint16>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_DIFF) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            constexpr auto func = &cfDifference<float>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            constexpr auto func = &cfDifference<quint16>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_SCREEN) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            constexpr auto func = &cfScreen<float>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            constexpr auto func = &cfScreen<quint16>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_DARKEN) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            constexpr auto func = &cfDarkenOnly<float>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            constexpr auto func = &cfDarkenOnly<quint16>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_LIGHTEN) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            constexpr auto func = &cfLightenOnly<float>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            constexpr auto func = &cfLightenOnly<quint16>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_NEGATION) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            constexpr auto func = &cfNegation<float>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            constexpr auto func = &cfNegation<quint16>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_HARD_MIX) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            if (isHDR) {
                using func = CFHardMix<float, ClampPolicyFloatHDR>;
                op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryDark());
            } else {
                using func = CFHardMix<float, ClampPolicyFloatSDR>;
                op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryDark());
            }
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            using func = CFHardMix<quint16, ClampPolicyInteger>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_HARD_MIX_PHOTOSHOP) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            constexpr auto func = &cfHardMixPhotoshop<float>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            constexpr auto func = &cfHardMixPhotoshop<quint16>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_HARD_MIX_PHOTOSHOP) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            constexpr auto func = &cfHardMixSofterPhotoshop<float>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            constexpr auto func = &cfHardMixSofterPhotoshop<quint16>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_LUMINOSITY_SAI) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            constexpr auto func = &cfAdditionSAI<HSVType, float>;
            op = new KoCompositeOpGenericSCAlpha<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            constexpr auto func = &cfAdditionSAI<HSVType, float>;
            op = new KoCompositeOpGenericSCAlpha<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_ARC_TANGENT) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            constexpr auto func = &cfArcTangent<float>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            constexpr auto func = &cfArcTangent<quint16>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_ALLANON) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            constexpr auto func = &cfAllanon<float>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            constexpr auto func = &cfAllanon<quint16>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_PARALLEL) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            using func = CFParallel<float>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            using func = CFParallel<quint16>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_EQUIVALENCE) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            using func = CFEquivalence<float>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            using func = CFEquivalence<quint16>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_GRAIN_MERGE) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            constexpr auto func = &cfGrainMerge<float>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            constexpr auto func = &cfGrainMerge<quint16>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_GRAIN_EXTRACT) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            constexpr auto func = &cfGrainExtract<float>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            constexpr auto func = &cfGrainExtract<quint16>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_GEOMETRIC_MEAN) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            using func = CFGeometricMean<float>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            using func = CFGeometricMean<quint16>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_ADDITIVE_SUBTRACTIVE) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            using func = CFAdditiveSubtractive<float>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            using func = CFAdditiveSubtractive<quint16>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_GAMMA_DARK) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            using func = CFGammaDark<float>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            using func = CFGammaDark<quint16>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_GAMMA_LIGHT) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            using func = CFGammaLight<float>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            using func = CFGammaLight<quint16>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_GAMMA_ILLUMINATION) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            using func = CFGammaIllumination<float>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            using func = CFGammaIllumination<quint16>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_DISSOLVE ||
               id == COMPOSITE_GREATER ||
               id == COMPOSITE_OVER ||
               id == COMPOSITE_ALPHA_DARKEN ||
               id == COMPOSITE_DESTINATION_ATOP ||
               id == COMPOSITE_DESTINATION_IN ||
               id == COMPOSITE_ERASE ||
               id == COMPOSITE_COPY) {
        op = cs->compositeOp(id);
    }

    KIS_ASSERT(op);

    return op;
}
