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

#include <KoCompositeOpGeneric.h>
#include <KoCompositeOpFunctions.h>
#include "compositeops/KoColorSpaceBlendingPolicy.h"
#include "compositeops/KoCompositeOpClampPolicy.h"


const KoCompositeOp* createOp(const KoColorSpace *cs, const QString &id, bool isHDR)
{
    using namespace KoCompositeOpClampPolicy;

    const KoCompositeOp *op = nullptr;

    /**
     * Some of the blendmodes exist only "in theory". We test them only to make sure
     * they are not actually needed in real life (exactly the same in SDR range and
     * SDR/negative-preserving).
     */
    if (id == COMPOSITE_BURN && !isHDR) { // SDR-only, HDR is the default
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            using func = CFColorBurn<float, ClampAsFloatSDR>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func,
                                                   KoAdditiveBlendingPolicy<Traits>>
                (cs, id, KoCompositeOp::categoryDark());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            using func = CFColorBurn<quint16, ClampAsInteger>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryDark());
        }
    } else if (id == COMPOSITE_LINEAR_BURN && !isHDR) { // SDR-only, HDR is the default
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
    } else if (id == COMPOSITE_PIN_LIGHT && !isHDR) { // SDR-only, HDR is the default
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            using func = CFPinLight<float, ClampAsFloatSDR>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryDark());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            using func = CFPinLight<quint16, ClampAsInteger>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
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

    KIS_ASSERT(op);

    return op;
}
