/*
 *  Copyright (c) 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
