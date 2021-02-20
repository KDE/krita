/*
 *  SPDX-FileCopyrightText: 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KOALPHADARKENPARAMSWRAPPER_H
#define KOALPHADARKENPARAMSWRAPPER_H

#include <KoCompositeOp.h>
#include "KoColorSpaceMaths.h"

bool KRITAPIGMENT_EXPORT useCreamyAlphaDarken();

struct KoAlphaDarkenParamsWrapperHard {
    KoAlphaDarkenParamsWrapperHard(const KoCompositeOp::ParameterInfo& params)
        : opacity(params.flow * params.opacity),
          flow(params.flow),
          averageOpacity(params.flow * (*params.lastOpacity))
    {
    }
    float opacity;
    float flow;
    float averageOpacity;

    template <typename T>
    static inline T calculateZeroFlowAlpha(T srcAlpha, T dstAlpha, T normCoeff) {
        return srcAlpha + dstAlpha - srcAlpha * dstAlpha * normCoeff;
    }

    template <typename T>
    static inline T calculateZeroFlowAlpha(T srcAlpha, T dstAlpha) {
        return srcAlpha + dstAlpha - srcAlpha * dstAlpha;
    }

    template<typename channels_type>
    static inline channels_type calculateZeroFlowAlphaLegacy(channels_type srcAlpha, channels_type dstAlpha) {
        return Arithmetic::unionShapeOpacity(srcAlpha, dstAlpha);
    }
};


struct KoAlphaDarkenParamsWrapperCreamy {
    KoAlphaDarkenParamsWrapperCreamy(const KoCompositeOp::ParameterInfo& params)
        : opacity(params.opacity),
          flow(params.flow),
          averageOpacity(*params.lastOpacity)
    {
    }
    float opacity;
    float flow;
    float averageOpacity;

    template <typename T>
    static inline T calculateZeroFlowAlpha(T srcAlpha, T dstAlpha, T normCoeff) {
        Q_UNUSED(srcAlpha);
        Q_UNUSED(normCoeff);

        return dstAlpha;
    }

    template <typename T>
    static inline T calculateZeroFlowAlpha(T srcAlpha, T dstAlpha) {
        Q_UNUSED(srcAlpha);

        return dstAlpha;
    }

    template<typename channels_type>
    static inline channels_type calculateZeroFlowAlphaLegacy(channels_type srcAlpha, channels_type dstAlpha) {
        Q_UNUSED(srcAlpha);
        return dstAlpha;
    }
};

#endif // KOALPHADARKENPARAMSWRAPPER_H
