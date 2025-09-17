/*
 *  SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KOCOMPOSITEOP_FUNCTIONS_H_
#define KOCOMPOSITEOP_FUNCTIONS_H_

#include <KoColorSpaceMaths.h>
#include <KoCompositeOpGenericFunctorBase.h>
#include <KoCompositeOpClampPolicy.h>

#include <type_traits>
#include <cmath>

#ifdef HAVE_OPENEXR
#include "half.h"
#endif

namespace {
template<typename channels_type>
inline void possiblyFixNegativeValuesNearZeroPoint(float& dstR, float& dstG, float& dstB)
{
    /**
     * Some RGB-based blendmodes may generate slightly negative values after
     * applying setLightness(), so we should clamp them if the destination
     * channel is not of an integer type
     */
    if constexpr (!std::numeric_limits<channels_type>::is_integer) {
        dstR = qMax(dstR, 0.0f);
        dstG = qMax(dstG, 0.0f);
        dstB = qMax(dstB, 0.0f);
    }
}

template<typename channels_type>
inline void possiblyClampValuesToSDR(float& dstR, float& dstG, float& dstB)
{
    /**
     * Some RGB-based modes may go out of the SDR-range, e.g. tangent-normal,
     * so we should clamp them in case the target colorspace is not an integer
     * and is not going to be clamped later.
     */
    if constexpr (!std::numeric_limits<channels_type>::is_integer) {
        dstR = qBound(0.0f, dstR, 1.0f);
        dstG = qBound(0.0f, dstG, 1.0f);
        dstB = qBound(0.0f, dstB, 1.0f);
    }
}
}

template<typename channels_type>
struct CFReorientedNormalMapCombine : KoClampedSourceAndDestinationCompositeOpGenericFunctorBase<channels_type>
{
    static void composeChannels(float srcR, float srcG, float srcB, float& dstR, float& dstG, float& dstB) {
        // see https://blog.selfshadow.com/publications/blending-in-detail/ by Barre-Brisebois and Hill

        using namespace Arithmetic;

        /**
         * If the source vector has zero height, then we should just
         * skip all the changes (with seems to be the behavior of the
         * formula around the null point)
         */
        if (isZeroValueFuzzy(srcB)) {
            return;
        }

        /**
         * If the destination vector has zero height, then we should skip its
         * modification (since we cannot rotate it in any way
         */

        float tx = 2*srcR-1;
        float ty = 2*srcG-1;
        float tz = 2*srcB;
        float ux = -2*dstR+1;
        float uy = -2*dstG+1;
        float uz = 2*dstB-1;
        float k = (tx*ux+ty*uy+tz*uz)/tz; // dot(t,u)/t.z
        float rx = tx*k-ux;
        float ry = ty*k-uy;
        float rz = tz*k-uz;

        float finalLength = rx*rx+ry*ry+rz*rz;


        /**
         * If the resulting length of the vector is zero, then just keep the
         * destination unchanged. Usually it means that the destination vector
         * is null (i.e. dstR == dstG == dstB == 0.5)
         */
        if (isZeroValueStrict(finalLength)) {
            return;
        }

        k = 1 / sqrt(finalLength); // normalize result
        rx *= k;
        ry *= k;
        rz *= k;
        dstR = rx*0.5+0.5;
        dstG = ry*0.5+0.5;
        dstB = rz*0.5+0.5;
    }
};

template<class HSXType, typename channels_type>
struct CFColor : KoClampedSourceAndDestinationCompositeOpGenericFunctorBase<channels_type>
{
    static void composeChannels(float srcR, float srcG, float srcB, float& dstR, float& dstG, float& dstB) {
        using namespace Arithmetic;
        float lum = getLightness<HSXType>(dstR, dstG, dstB);
        dstR = srcR;
        dstG = srcG;
        dstB = srcB;
        setLightness<HSXType>(dstR, dstG, dstB, lum);
        possiblyFixNegativeValuesNearZeroPoint<channels_type>(dstR, dstG, dstB);
    }
};

template<class HSXType, typename channels_type>
struct CFTint : KoClampedSourceAndDestinationCompositeOpGenericFunctorBase<channels_type>
{
    static void composeChannels(float srcR, float srcG, float srcB, float& dstR, float& dstG, float& dstB) {
        using namespace Arithmetic;
        // This mode is useful to change the color of a sketch, to make it easier to ink over it.
        // Unlike the Color function above, it changes black to the src color and interpolates to white.

        const float light = getLightness<HSXType>(dstR, dstG, dstB);
        const float oneMinusLight = unitValue<float>() - light;
        dstR = light + srcR * oneMinusLight;
        dstG = light + srcG * oneMinusLight;
        dstB = light + srcB * oneMinusLight;
    }
};

template<class HSXType, typename channels_type>
struct CFLambertLighting : KoClampedSourceAndDestinationCompositeOpGenericFunctorBase<channels_type>
{
    static void composeChannels(float srcR, float srcG, float srcB, float& dstR, float& dstG, float& dstB) {
        float tr = srcR * dstR * (1.0 / 0.215686);
        float tg = srcG * dstG * (1.0 / 0.215686);
        float tb = srcB * dstB * (1.0 / 0.215686);

        if (tr > 1.0) {
            dstR = 1.0 + (tr - 1.0) * (tr - 1.0) * 0.01925;
        }
        else {
            dstR = tr;
        }

        if (tg > 1.0) {
            dstG = 1.0 + (tg - 1.0) * (tg - 1.0) * 0.01925;
        }
        else {
            dstG = tg;
        }

        if (tb > 1.0) {
            dstB = 1.0 + (tb - 1.0) * (tb - 1.0) * 0.01925;
        }
        else {
            dstB = tb;
        }

        ToneMapping<HSXType, float>(dstR, dstG, dstB);
    }
};

template<class HSXType, typename channels_type>
struct CFLambertLightingGamma2_2 : KoClampedSourceAndDestinationCompositeOpGenericFunctorBase<channels_type>
{
    static void composeChannels(float srcR, float srcG, float srcB, float& dstR, float& dstG, float& dstB) {
        float tr = srcR * dstR * 2.0;
        float tg = srcG * dstG * 2.0;
        float tb = srcB * dstB * 2.0;

        if (tr > 1.0) {
            dstR = 1.0 + (tr - 1.0) * (tr - 1.0) * 0.4;
        }
        else {
            dstR = tr;
        }

        if (tg > 1.0) {
            dstG = 1.0 + (tg - 1.0) * (tg - 1.0) * 0.4;
        }
        else {
            dstG = tg;
        }

        if (tb > 1.0) {
            dstB = 1.0 + (tb - 1.0) * (tb - 1.0) * 0.4;
        }
        else {
            dstB = tb;
        }

        ToneMapping<HSXType, float>(dstR, dstG, dstB);
    }
};

template<class HSXType, typename channels_type>
struct CFLightness : KoClampedSourceAndDestinationCompositeOpGenericFunctorBase<channels_type>
{
    static void composeChannels(float srcR, float srcG, float srcB, float& dstR, float& dstG, float& dstB) {
        setLightness<HSXType>(dstR, dstG, dstB, getLightness<HSXType>(srcR, srcG, srcB));
        possiblyFixNegativeValuesNearZeroPoint<channels_type>(dstR, dstG, dstB);
    }
};


template<class HSXType, typename channels_type>
struct CFIncreaseLightness : KoClampedSourceAndDestinationCompositeOpGenericFunctorBase<channels_type>
{
    static void composeChannels(float srcR, float srcG, float srcB, float& dstR, float& dstG, float& dstB) {
        addLightness<HSXType>(dstR, dstG, dstB, getLightness<HSXType>(srcR, srcG, srcB));
        possiblyFixNegativeValuesNearZeroPoint<channels_type>(dstR, dstG, dstB);
    }
};

template<class HSXType, typename channels_type>
struct CFDecreaseLightness : KoClampedSourceAndDestinationCompositeOpGenericFunctorBase<channels_type>
{
    static void composeChannels(float srcR, float srcG, float srcB, float& dstR, float& dstG, float& dstB) {
        addLightness<HSXType>(dstR, dstG, dstB, getLightness<HSXType>(srcR, srcG, srcB) - 1.0f);
        possiblyFixNegativeValuesNearZeroPoint<channels_type>(dstR, dstG, dstB);
    }
};

template<class HSXType, typename channels_type>
struct CFSaturation : KoClampedSourceAndDestinationCompositeOpGenericFunctorBase<channels_type>
{
    static void composeChannels(float srcR, float srcG, float srcB, float& dstR, float& dstG, float& dstB) {
        float sat   = getSaturation<HSXType>(srcR, srcG, srcB);
        float light = getLightness<HSXType>(dstR, dstG, dstB);
        setSaturation<HSXType>(dstR, dstG, dstB, sat);
        setLightness<HSXType>(dstR, dstG, dstB, light);
        possiblyFixNegativeValuesNearZeroPoint<channels_type>(dstR, dstG, dstB);
    }
};

template<class HSXType, typename channels_type>
struct CFIncreaseSaturation : KoClampedSourceAndDestinationCompositeOpGenericFunctorBase<channels_type>
{
    static void composeChannels(float srcR, float srcG, float srcB, float& dstR, float& dstG, float& dstB) {
        using namespace Arithmetic;
        float sat   = lerp(getSaturation<HSXType>(dstR, dstG, dstB), unitValue<float>(), getSaturation<HSXType>(srcR, srcG, srcB));
        float light = getLightness<HSXType>(dstR, dstG, dstB);
        setSaturation<HSXType>(dstR, dstG, dstB, sat);
        setLightness<HSXType>(dstR, dstG, dstB, light);
        possiblyFixNegativeValuesNearZeroPoint<channels_type>(dstR, dstG, dstB);
    }
};

template<class HSXType, typename channels_type>
struct CFDecreaseSaturation : KoClampedSourceAndDestinationCompositeOpGenericFunctorBase<channels_type>
{
    static void composeChannels(float srcR, float srcG, float srcB, float& dstR, float& dstG, float& dstB) {
        using namespace Arithmetic;
        float sat   = lerp(zeroValue<float>(), getSaturation<HSXType>(dstR, dstG, dstB), getSaturation<HSXType>(srcR, srcG, srcB));
        float light = getLightness<HSXType>(dstR, dstG, dstB);
        setSaturation<HSXType>(dstR, dstG, dstB, sat);
        setLightness<HSXType>(dstR, dstG, dstB, light);
        possiblyFixNegativeValuesNearZeroPoint<channels_type>(dstR, dstG, dstB);
    }
};

template<class HSXType, typename channels_type>
struct CFHue : KoClampedSourceAndDestinationCompositeOpGenericFunctorBase<channels_type>
{
    static void composeChannels(float srcR, float srcG, float srcB, float& dstR, float& dstG, float& dstB) {
        float sat = getSaturation<HSXType>(dstR, dstG, dstB);
        float lum = getLightness<HSXType>(dstR, dstG, dstB);

        dstR = srcR;
        dstG = srcG;
        dstB = srcB;

        setSaturation<HSXType>(dstR, dstG, dstB, sat);
        setLightness<HSXType>(dstR, dstG, dstB, lum);
        possiblyFixNegativeValuesNearZeroPoint<channels_type>(dstR, dstG, dstB);
    }
};

template<typename channels_type>
struct CFTangentNormalmap : KoClampedSourceAndDestinationCompositeOpGenericFunctorBase<channels_type>
{
    static void composeChannels(float srcR, float srcG, float srcB, float& dstR, float& dstG, float& dstB) {
        using namespace Arithmetic;
        const float half = halfValue<float>();
        dstR = srcR + (dstR - half);
        dstG = srcG + (dstG - half);
        dstB = srcB + (dstB - unitValue<float>());
        possiblyClampValuesToSDR<channels_type>(dstR, dstG, dstB);
    }
};

template<class HSXType, typename channels_type>
struct CFDarkerColor : KoClampedSourceAndDestinationCompositeOpGenericFunctorBase<channels_type>
{
    static void composeChannels(float srcR, float srcG, float srcB, float& dstR, float& dstG, float& dstB) {
        const float lum = getLightness<HSXType>(dstR, dstG, dstB);
        const float lum2 = getLightness<HSXType>(srcR, srcG, srcB);
        if (lum > lum2) {
            dstR = srcR;
            dstG = srcG;
            dstB = srcB;
        }
    }
};

template<class HSXType, typename channels_type>
struct CFLighterColor : KoClampedSourceAndDestinationCompositeOpGenericFunctorBase<channels_type>
{
    static void composeChannels(float srcR, float srcG, float srcB, float& dstR, float& dstG, float& dstB) {
        const float lum = getLightness<HSXType>(dstR, dstG, dstB);
        const float lum2 = getLightness<HSXType>(srcR, srcG, srcB);
        if (lum < lum2) {
            dstR = srcR;
            dstG = srcG;
            dstB = srcB;
        }
    }
};

template<class T>
struct CFColorBurn : KoClampedSourceAndDestinationCompositeOpGenericFunctorBase<T>
{
    static T composeChannel(T src, T dst) {
        using namespace Arithmetic;

        /**
         * dst-unit is a stable point of color-burn belndmode,
         * it never changes, whatever the value of src channel
         */
        if (isUnitValueStrict(dst)) {
            return unitValue<T>();
        }

        if constexpr (std::numeric_limits<T>::is_integer) {
            // Handle the case where the denominator is 0. See color dodge for a
            // detailed explanation
            if (isZeroValueStrict(src)) {
                return zeroValue<T>();
            }
        }

        using composite_type = typename KoColorSpaceMathsTraits<T>::compositetype;
        composite_type divisionResult = div(inv(dst), src);

        if constexpr (!std::numeric_limits<T>::is_integer) {
            if (!std::isfinite(divisionResult)) {
                return zeroValue<T>();
            }
        }

        return inv<T>(clampToSDR<T>(divisionResult));
    }
};


template<class T,
         template <typename> typename ClampPolicy>
struct CFLinearBurn : KoClampedSourceCompositeOpGenericFunctorBase<T> {
    static inline T composeChannel(T src, T dst) {
        using namespace Arithmetic;
        using clamp_policy = ClampPolicy<T>;
        using composite_type = typename KoColorSpaceMathsTraits<T>::compositetype;

        return clamp_policy::clampResult(composite_type(src) + dst - unitValue<T>());
    }
};

template<typename T,
         template <typename> typename ClampPolicy>
struct CFColorDodge : KoClampedSourceCompositeOpGenericFunctorBase<T> {
    static inline T composeChannel(T src, T dst) {
        using namespace Arithmetic;
        using clamp_policy = ClampPolicy<T>;

        // Handle the case where the denominator is 0.
        // When src is 1 then the denominator (1 - src) becomes 0, and to avoid
        // dividing by 0 we treat the denominator as an infinitely small number,
        // so the result of the formula would approach infinity. As in the generic
        // case, that result is clamped to the maximum value (which for integer
        // types is the same as the unit value).
        // Another special case is when both numerator and denominator are 0. In
        // this case we also treat the denominator as an infinitely small number,
        // and the numerator can remain as 0, so dividing 0 over a number (no matter
        // how small it is) gives 0.
        if (isUnitValueStrict(src)) {
            return isZeroValueClampedStrict(dst) ? zeroValue<T>() : KoColorSpaceMathsTraits<T>::unitValue;
        }

        return clamp_policy::clampResultAllowNegative(
                    clamp_policy::fixInfiniteAfterDivision(
                        div(dst, inv(src))
                        )
                    );
    }
};

template<class T>
inline T cfAddition(T src, T dst) {
    typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;
    return Arithmetic::clamp<T>(composite_type(src) + dst);
}

template<class T>
inline T cfSubtract(T src, T dst) {
    using namespace Arithmetic;
    typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;
    return clamp<T>(composite_type(dst) - src);
}

template<class T>
struct CFInverseSubtract : KoClampedSourceCompositeOpGenericFunctorBase<T> {
    static inline T composeChannel(T src, T dst) {
        using namespace Arithmetic;
        typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;
        return clamp<T>(composite_type(dst) - inv(src));
    }
};

template<class T>
struct CFExclusion : KoClampedSourceAndDestinationCompositeOpGenericFunctorBase<T> {
    static inline T composeChannel(T src, T dst) {
        using namespace Arithmetic;
        typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;

        composite_type x = mul(src, dst);
        return clamp<T>(composite_type(dst) + src - (x + x));
    }
};



template<class T>
struct CFDivide : KoClampedSourceAndDestinationBottomCompositeOpGenericFunctorBase<T> {
    static inline T composeChannel(T src, T dst) {
        using namespace Arithmetic;
        using composite_type = typename KoColorSpaceMathsTraits<T>::compositetype;

        if (isZeroValueStrict(dst)) {
            return zeroValue<T>();
        }

        if (isZeroValueStrict(src)) {
            return unitValue<T>();
        }

        composite_type result = div(dst, src);

        /**
         * Since input values are clamped, we can never get a negative
         * infinity as a result of an operation. Hence we can just clamp the result
         * to SDR safely.
         */
        return clampToSDRTop<T>(result);
    }
};

template<class T>
struct CFHardLight : KoClampedSourceCompositeOpGenericFunctorBase<T> {
    static inline T composeChannel(T src, T dst) {
        using namespace Arithmetic;
        using composite_type = typename KoColorSpaceMathsTraits<T>::compositetype;

        if (isHalfValueFuzzy(src)) {
            return dst;
        }

        composite_type src2 = composite_type(src) + src;

        if(src > halfValue<T>()) {
            // screen(src*2.0 - 1.0, dst)
            src2 -= unitValue<T>();

            // src2 is guaranteed to be smaller than unitValue<T>() now
            return Arithmetic::unionShapeOpacity(T(src2), dst);
        }

        // src2 is guaranteed to be smaller than unitValue<T>() due to 'if'
        return Arithmetic::mul(T(src2), dst);
    }
};

template<class T>
struct CFSoftLightSvg : KoClampedSourceAndDestinationCompositeOpGenericFunctorBase<T> {
    static inline T composeChannel(T src, T dst) {
        using namespace Arithmetic;

        /**
         * soft-light scales the color using sqrt and pow2
         * functions, hence we cannot support HDR values
         */

        qreal fsrc = scale<qreal>(src);
        qreal fdst = scale<qreal>(dst);

        if(fsrc > 0.5) {
            qreal D = (fdst > 0.25) ? sqrt(fdst) : ((16.0*fdst - 12.0)*fdst + 4.0)*fdst;
            return scale<T>(fdst + (2.0*fsrc - 1.0) * (D - fdst));
        }

        return scale<T>(fdst - (1.0 - 2.0 * fsrc) * fdst * (1.0 - fdst));
    }
};


template<class T>
struct CFSoftLight : KoClampedSourceAndDestinationCompositeOpGenericFunctorBase<T> {
    static inline T composeChannel(T src, T dst) {
        using namespace Arithmetic;

        qreal fsrc = scale<qreal>(src);
        qreal fdst = scale<qreal>(dst);

        if(fsrc > 0.5) {
            // lerp(dst, sqrt(dst), (2.0 * fsrc - 1.0))
            return scale<T>(fdst + (2.0 * fsrc - 1.0) * (sqrt(fdst) - fdst));
        }

        // lerp(dst, dst^2, (1.0 - 2.0 * fsrc))
        return scale<T>(fdst - (1.0 - 2.0*fsrc) * fdst * (1.0 - fdst));
    }
};

template<class T,
         template <typename> typename ClampPolicy>
struct CFVividLight : KoClampedSourceCompositeOpGenericFunctorBase<T> {
    static inline T composeChannel(T src, T dst) {
        using namespace Arithmetic;
        using clamp_policy = ClampPolicy<T>;
        using composite_type = typename KoColorSpaceMathsTraits<T>::compositetype;

        if (src < halfValue<T>()) {
            if (isZeroValueStrict(src)) {
                return isUnitValueClampedStrict(dst) ? clamp_policy::clampResultAllowNegative(dst) : zeroValue<T>();
            }

            // min(1,max(0,1-(1-dst) / (2*src)))
            composite_type src2 = composite_type(src) + src;
            composite_type dsti = inv(dst);
            return clamp_policy::clampResult(
                        unitValue<T>() -
                        clamp_policy::fixInfiniteAfterDivision(
                            divideInCompositeSpace<T>(dsti, src2)
                            )
                        );
        }

        if (isUnitValueStrict(src)) {
            return isZeroValueClampedStrict(dst) ? zeroValue<T>() : unitValue<T>();
        }

        // min(1,max(0, dst / (2*(1-src)))
        composite_type srci2 = inv(src);
        srci2 += srci2;
        return clamp_policy::clampResultAllowNegative(
                    clamp_policy::fixInfiniteAfterDivision(
                        divideInCompositeSpace<T>(composite_type(dst), srci2)
                        )
                    );
    }
};

template<class T>
struct CFPinLight : KoClampedSourceAndDestinationCompositeOpGenericFunctorBase<T> {
    static inline T composeChannel(T src, T dst) {
        using namespace Arithmetic;
        using composite_type = typename KoColorSpaceMathsTraits<T>::compositetype;

        // TODO: verify that the formula is correct (the first max would be useless here)
        // max(0, max(2*src-1, min(dst, 2*src)))
        composite_type src2 = composite_type(src) + src;
        composite_type a    = qMin<composite_type>(dst, src2);
        composite_type b    = qMax<composite_type>(src2-Arithmetic::unitValue<T>(), a);

        /**
         * `b` is guaranteed to be in range 0...1 since the inputs are clamped,
         * so no additional clamping is needed
         */
        return b;
    }
};

template<class T>
inline T cfArcTangent(T src, T dst) {
    using namespace Arithmetic;
    return scale<T>(2.0 * atan2(scale<qreal>(src), scale<qreal>(dst)) / Arithmetic::pi);
}

template<class T>
inline T cfAllanon(T src, T dst) {
    using namespace Arithmetic;
    typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;
    // (dst + src) / 2   [or (dst + src) * 0.5]
    return T((composite_type(src) + dst) * halfValue<T>() / unitValue<T>());
}

template<class T>
inline T cfLinearLight(T src, T dst) {
    using namespace Arithmetic;
    typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;
    
    // min(1,max(0,(dst + 2*src)-1))
    return clamp<T>((composite_type(src) + src + dst) - unitValue<T>());
}

template<class T>
struct CFParallel : KoClampedSourceAndDestinationCompositeOpGenericFunctorBase<T> {
    static inline T composeChannel(T src, T dst) {
        using namespace Arithmetic;
        typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;

        const bool srcIsSafe = !isUnsafeAsDivisor(src);
        const bool dstIsSafe = !isUnsafeAsDivisor(dst);

        if (!srcIsSafe || !dstIsSafe) {
            return zeroValue<T>();
        }

        // min(max(2 / (1/dst + 1/src), 0), 1)
        composite_type unit = unitValue<T>();
        composite_type s    = div<T>(unit, src);
        composite_type d    = div<T>(unit, dst);

        return clamp<T>((unit+unit) * unit / (d+s));
    }
};

template<class T>
struct CFEquivalence : KoClampedSourceAndDestinationBottomCompositeOpGenericFunctorBase<T> {
    static inline T composeChannel(T src, T dst) {
        using namespace Arithmetic;
        typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;
        // TODO: is the formula correct?
        // 1 - abs(dst - src)
        composite_type x = qAbs(composite_type(dst) - src);
        return clamp<T>(x);
    }
};

template<class T>
struct CFGrainMerge : KoClampedSourceAndDestinationCompositeOpGenericFunctorBase<T> {
    static inline T composeChannel(T src, T dst) {
        using namespace Arithmetic;
        typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;
        return clampToSDR<T>(composite_type(dst) + src - halfValue<T>());
    }
};

template<class T>
struct CFGrainExtract : KoClampedSourceAndDestinationCompositeOpGenericFunctorBase<T> {
    static inline T composeChannel(T src, T dst) {
        using namespace Arithmetic;
        typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;
        return clampToSDR<T>(composite_type(dst) - src + halfValue<T>());
    }
};

template<class T,
         template <typename> typename ClampPolicy>
struct CFHardMix : KoClampedSourceCompositeOpGenericFunctorBase<T> {
    static inline T composeChannel(T src, T dst) {
        using namespace Arithmetic;

        /**
         * We do not clamp dst here, because it is supposed to be modified by
         * cfColorDodge if it is too bright. The result will be clamped in the
         * subordinate function.
         */
        return dst > halfValue<T>() ?
            CFColorDodge<T, ClampPolicy>::composeChannel(src,dst) :
            CFColorBurn<T>::composeChannel(src,dst);
    }
};

template<class T>
struct CFHardMixPhotoshop : KoClampedSourceAndDestinationCompositeOpGenericFunctorBase<T> {
    static inline T composeChannel(T src, T dst) {
        using namespace Arithmetic;
        typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;

        const composite_type sum = composite_type(src) + dst;

        return sum > unitValue<T>() ? unitValue<T>() : zeroValue<T>();
    }
};

// Approximation of the hard mix mode used by photoshop in the brush texturing
// In contrast to the normal hard mix, this produces antialiased edges, better
// for texturing the brush dab, at least visually

template<class T>
struct CFHardMixSofterPhotoshop : KoClampedSourceAndDestinationCompositeOpGenericFunctorBase<T> {
    static inline T composeChannel(T src, T dst) {
        using namespace Arithmetic;
        typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;

        const composite_type srcScaleFactor = static_cast<composite_type>(2);
        const composite_type dstScaleFactor = static_cast<composite_type>(3);

        return clampToSDR<T>(dstScaleFactor * dst - srcScaleFactor * inv(src));
    }
};

template<class T>
struct CFAdditiveSubtractive : KoClampedSourceAndDestinationBottomCompositeOpGenericFunctorBase<T> {
    static inline T composeChannel(T src, T dst) {
        using namespace Arithmetic;

        // min(1,max(0,abs(sqr(CB)-sqr(CT))))
        qreal x = sqrt(scale<qreal>(dst)) - sqrt(scale<qreal>(src));
        return scale<T>(qAbs(x));
    }
};

template<class T>
struct CFGammaDark : KoClampedSourceAndDestinationCompositeOpGenericFunctorBase<T> {
    static inline T composeChannel(T src, T dst) {
        using namespace Arithmetic;

        const qreal dstReal = scale<qreal>(dst);

        if (isZeroValueFuzzy(dstReal)) {
            return zeroValue<T>();
        }

        if (isUnitValueFuzzy(dstReal)) {
            return unitValue<T>();
        }

        // power(dst, 1/src)

        const qreal srcReal = scale<qreal>(src);
        const qreal exponent = 1.0 / srcReal;

        if (std::isfinite(exponent)) {
            return scale<T>(
                std::min<qreal>(pow(dstReal, exponent),
                                KoColorSpaceMathsTraits<T>::max));
        } else {
            return zeroValue<T>();
        }
    }
};

template<class T>
struct CFGammaLight : KoClampedSourceAndDestinationCompositeOpGenericFunctorBase<T> {
    static inline T composeChannel(T src, T dst) {
        using namespace Arithmetic;

        const qreal dstReal = scale<qreal>(dst);

        if (isZeroValueFuzzy(dstReal)) {
            return zeroValue<T>();
        }

        if (isUnitValueFuzzy(dstReal)) {
            return unitValue<T>();
        }

        const qreal srcReal = scale<qreal>(src);

        return scale<T>(pow(dstReal, srcReal));
    }
};

template<class T>
struct CFGammaIllumination : KoClampedSourceAndDestinationCompositeOpGenericFunctorBase<T> {
    static inline T composeChannel(T src, T dst) {
        using namespace Arithmetic;
        return inv(CFGammaDark<T>::composeChannel(inv(src),inv(dst)));
    }
};

template<class T>
struct CFGeometricMean : KoClampedSourceAndDestinationBottomCompositeOpGenericFunctorBase<T> {
    static inline T composeChannel(T src, T dst) {
        using namespace Arithmetic;

        if (isUnitValueFuzzy(src) && isUnitValueFuzzy(dst)) {
            return unitValue<T>();
        }

        return scale<T>(sqrt(scale<qreal>(dst) * scale<qreal>(src)));
    }
};

template<class T>
inline T cfOver(T src, T dst) { Q_UNUSED(dst); return src; }

template<class T>
struct CFOverlay : CFHardLight<T> {
    static inline T composeChannel(T src, T dst) {
        return CFHardLight<T>::composeChannel(dst,src);
    }
};

template<class T>
inline T cfMultiply(T src, T dst) { return Arithmetic::mul(src, dst); }

template<class T,
         template <typename> typename ClampPolicy>
struct CFHardOverlay : KoClampedSourceFullAndDestinationBottomCompositeOpGenericFunctorBase<T> {
    static inline T composeChannel(T src, T dst) {
        using namespace Arithmetic;
        using clamp_policy = ClampPolicy<T>;
        using composite_type = typename KoColorSpaceMathsTraits<T>::compositetype;

        if (isZeroValueStrict(dst)) {
            return zeroValue<T>();
        }

        if (isUnitValueStrict(src)) {
            return clamp_policy::clippedMaxValue();
        }

        if(src >= halfValue<T>()) {
            return clamp_policy::clampResultAllowNegative(
                        clamp_policy::fixInfiniteAfterDivision(
                            divideInCompositeSpace<T>(composite_type(dst),
                                                           2 * composite_type(inv(src)))
                            )
                        );
        }

        return clamp_policy::clampResultAllowNegative(
                    multiplyInCompositeSpace<T>(composite_type(dst),
                                                     2 * composite_type(src))
                    );
    }
};

template<class T>
inline T cfDifference(T src, T dst) { return qMax(src,dst) - qMin(src,dst); }

template<class T>
inline T cfScreen(T src, T dst) { return Arithmetic::unionShapeOpacity(src, dst); }

template<class T>
inline T cfDarkenOnly(T src, T dst) { return qMin(src, dst); }

template<class T>
inline T cfLightenOnly(T src, T dst) { return qMax(src, dst); }

template<class T>
inline T cfGlow(T src, T dst) {
    using namespace Arithmetic;
        // see http://www.pegtop.net/delphi/articles/blendmodes/quadratic.htm for formulas of Quadratic Blending Modes like Glow, Reflect, Freeze, and Heat

    if (dst == unitValue<T>()) {
        return unitValue<T>();
    }

    return clamp<T>(div(mul(src, src), inv(dst)));
}

template<class T>
inline T cfReflect(T src, T dst) {
    using namespace Arithmetic;
    
    
    return clamp<T>(cfGlow(dst,src));
}

template<class T>
inline T cfHeat(T src, T dst) {
    using namespace Arithmetic;
    
    if(src == unitValue<T>()) {
        return unitValue<T>();
    }

    if(dst == zeroValue<T>()) {
        return zeroValue<T>();
    }

    return inv(clamp<T>(div(mul(inv(src), inv(src)),dst)));
}

template<class T>
inline T cfFreeze(T src, T dst) {
    using namespace Arithmetic;
    
    return (cfHeat(dst,src)); 
}

template<typename T>
struct CFHelow : KoClampedSourceAndDestinationCompositeOpGenericFunctorBase<T> {
    static inline T composeChannel(T src, T dst) {
        using namespace Arithmetic;
        // see http://www.pegtop.net/delphi/articles/blendmodes/quadratic.htm for formulas of Quadratic Blending Modes like Glow, Reflect, Freeze, and Heat

        if (isUnitValueFuzzy<T>(CFHardMixPhotoshop<T>::composeChannel(src,dst))) {
            return cfHeat(src,dst);
        }

        if (isZeroValueFuzzy<T>(src)) {
            return zeroValue<T>();
        }

        return (cfGlow(src,dst));
    }
};

template<typename T>
struct CFFrect : KoClampedSourceAndDestinationCompositeOpGenericFunctorBase<T> {
    static inline T composeChannel(T src, T dst) {
        using namespace Arithmetic;

        if (isUnitValueFuzzy<T>(CFHardMixPhotoshop<T>::composeChannel(src,dst))) {
            return cfFreeze(src,dst);
        }

        if (isZeroValueFuzzy<T>(dst)) {
            return zeroValue<T>();
        }

        return (cfReflect(src,dst));
    }
};

template<typename T>
struct CFGleat : KoClampedSourceAndDestinationCompositeOpGenericFunctorBase<T> {
    static inline T composeChannel(T src, T dst) {
        using namespace Arithmetic;
        // see http://www.pegtop.net/delphi/articles/blendmodes/quadratic.htm for formulas of Quadratic Blending Modes like Glow, Reflect, Freeze, and Heat

        if(isUnitValueFuzzy<T>(dst)) {
            return unitValue<T>();
        }

        if(isUnitValueFuzzy<T>(CFHardMixPhotoshop<T>::composeChannel(src, dst))) {
            return cfGlow(src,dst);
        }

        return (cfHeat(src,dst));
    }
};

template<class T>
struct CFReeze : CFGleat<T>
{
    static inline T composeChannel(T src, T dst) {
        return CFGleat<T>::composeChannel(dst,src);
    }
};

template<class T>
inline T cfFhyrd(T src, T dst) {
    using namespace Arithmetic;
    
    return (cfAllanon(CFFrect<T>::composeChannel(src,dst),
                      CFHelow<T>::composeChannel(src,dst)));
}

template<class T>
inline T cfInterpolation(T src, T dst) {
    using namespace Arithmetic;

    qreal fsrc = scale<qreal>(src);
    qreal fdst = scale<qreal>(dst);
    
    if(dst == zeroValue<T>() && src == zeroValue<T>()) {
        return zeroValue<T>();
    } 

    return scale<T>(.5f-.25f*cos(pi*(fsrc))-.25f*cos(pi*(fdst)));
}

template<class T>
inline T cfInterpolationB(T src, T dst) {
    using namespace Arithmetic;

    return cfInterpolation(cfInterpolation(src,dst),cfInterpolation(src,dst));
}


template<typename T>
struct CFPenumbraB : KoClampedSourceAndDestinationCompositeOpGenericFunctorBase<T> {
    static inline T composeChannel(T src, T dst) {
        using namespace Arithmetic;
        using namespace KoCompositeOpClampPolicy;

        if (dst == unitValue<T>()) {
            return unitValue<T>();
        }
        if (dst + src < unitValue<T>()) {
            return FunctorWithSDRClampPolicy<CFColorDodge, T>::composeChannel(dst,src) / 2;
        }
        if (src == zeroValue<T>()) {
            return zeroValue<T>();
        }

        return inv(clamp<T>(div(inv(dst),src)/2));
    }
};

template<class T>
inline T cfPenumbraD(T src, T dst) {
    using namespace Arithmetic;
    
    if (dst == unitValue<T>()) {
        return unitValue<T>();
    }
    
    return cfArcTangent(src,inv(dst));
}

template<class T>
inline T cfPenumbraC(T src, T dst) {
    using namespace Arithmetic;
    
    return cfPenumbraD(dst,src);
}


template<typename T>
struct CFPenumbraA : CFPenumbraB<T> {
    static inline T composeChannel(T src, T dst) {
        return CFPenumbraB<T>::composeChannel(dst, src);
    }
};

template<class T>
inline T cfSoftLightIFSIllusions(T src, T dst) {
    using namespace Arithmetic;
    
    qreal fsrc = scale<qreal>(src);
    qreal fdst = scale<qreal>(dst);
    
    return scale<T>(pow(fdst,pow(2.0,(mul(2.0,.5f-fsrc))))); 
}

template<class T>
inline T cfSoftLightPegtopDelphi(T src, T dst) {
    using namespace Arithmetic;

    return clamp<T>(cfAddition(mul(dst,cfScreen(src,dst)),mul(mul(src,dst),inv(dst)))); 
}


template<class T>
struct CFNegation : KoClampedSourceAndDestinationCompositeOpGenericFunctorBase<T> {
    static inline T composeChannel(T src, T dst) {
        using namespace Arithmetic;
        typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;
        
        composite_type unit = unitValue<T>();
        composite_type a = unit - src - dst;
        composite_type s = std::abs(a);
        composite_type d = unit - s;
        
        return T(d);
    }
};

template<class T>
inline T cfNor(T src, T dst) {
    using namespace Arithmetic;
        
    return and(src,dst);
}

template<class T>
inline T cfNand(T src, T dst) {
    using namespace Arithmetic;
        
    return or(src,dst);
}

template<class T>
inline T cfXor(T src, T dst) {
    using namespace Arithmetic;
    
    return xor(src,dst);
}

template<class T>
inline T cfXnor(T src, T dst) {
    using namespace Arithmetic;
    
    return cfXor(src,inv(dst));
}

template<class T>
inline T cfAnd(T src, T dst) {
    using namespace Arithmetic;
        
    return cfNor(inv(src),inv(dst));
}

template<class T>
inline T cfOr(T src, T dst) {
    using namespace Arithmetic;
        
    return cfNand(inv(src),inv(dst));
}

template<class T>
inline T cfConverse(T src, T dst) {
    using namespace Arithmetic;
        
    return cfOr(inv(src),dst);
}

template<class T>
inline T cfNotConverse(T src, T dst) {
    using namespace Arithmetic;
        
    return cfAnd(src,inv(dst));
}

template<class T>
inline T cfImplies(T src, T dst) {
    using namespace Arithmetic;
        
    return cfOr(src,inv(dst));
}

template<class T>
inline T cfNotImplies(T src, T dst) {
    using namespace Arithmetic;
        
    return cfAnd(inv(src),dst);
}

template<class T>
inline T cfPNormA(T src, T dst) {
    using namespace Arithmetic;
    //This is also known as P-Norm mode with factor of 2.3333 See IMBLEND image blending mode samples, and please see imblend.m file found on Additional Blending Mode thread at Phabricator. 1/2.3333 is .42875...
    
    return clamp<T>(pow(pow((float)dst, 2.3333333333333333) + pow((float)src, 2.3333333333333333), 0.428571428571434)); 
}

template<class T>
inline T cfPNormB(T src, T dst) {
    using namespace Arithmetic;
    //This is also known as P-Norm mode with factor of 2.3333 See IMBLEND image blending mode samples, and please see imblend.m file found on Additional Blending Mode thread at Phabricator. 1/2.3333 is .42875...
    
    return clamp<T>(pow(pow(dst,4)+pow(src,4),0.25)); 
}

template<class T>
inline T cfSuperLight(T src, T dst) {
    using namespace Arithmetic;
    //4.0 can be adjusted to taste. 4.0 is picked for being the best in terms of contrast and details. See imblend.m file.
        
    qreal fsrc = scale<qreal>(src);
    qreal fdst = scale<qreal>(dst);
    
    if (fsrc < .5) {
        return scale<T>(inv(pow(pow(inv(fdst),2.875)+pow(inv(2.0*fsrc),2.875),1.0/2.875)));
    }   
    
    return scale<T>(pow(pow(fdst,2.875)+pow(2.0*fsrc-1.0,2.875),1.0/2.875)); 
}

template<class T>
inline T cfTintIFSIllusions(T src, T dst) {
    using namespace Arithmetic;
    //Known as Light Blending mode found in IFS Illusions. Picked this name because it results into a very strong tint, and has better naming convention.
    
    qreal fsrc = scale<qreal>(src);
    qreal fdst = scale<qreal>(dst);
    
    return scale<T>(fsrc*inv(fdst)+sqrt(fdst)); 
}

template<class T>
inline T cfShadeIFSIllusions(T src, T dst) {
    using namespace Arithmetic;
    //Known as Shadow Blending mode found in IFS Illusions. Picked this name because it is the opposite of Tint (IFS Illusion Blending mode).
    
    qreal fsrc = scale<qreal>(src);
    qreal fdst = scale<qreal>(dst);
    
    return scale<T>(inv((inv(fdst)*fsrc)+sqrt(inv(fsrc)))); 
}

template<class T>
inline T cfFogLightenIFSIllusions(T src, T dst) {
    using namespace Arithmetic;
    //Known as Bright Blending mode found in IFS Illusions. Picked this name because the shading reminds me of fog when overlaying with a gradient.
    
    qreal fsrc = scale<qreal>(src);
    qreal fdst = scale<qreal>(dst);
    
    if (fsrc < .5) {
        return scale<T>(inv(inv(fsrc)*fsrc)-inv(fdst)*inv(fsrc));
    }  
    
    return scale<T>(fsrc-inv(fdst)*inv(fsrc)+pow(inv(fsrc),2)); 
}

template<class T>
inline T cfFogDarkenIFSIllusions(T src, T dst) {
    using namespace Arithmetic;
    //Known as Dark Blending mode found in IFS Illusions. Picked this name because the shading reminds me of fog when overlaying with a gradient.
    
    qreal fsrc = scale<qreal>(src);
    qreal fdst = scale<qreal>(dst);
    
    if (fsrc < .5) {
        return scale<T>(inv(fsrc)*fsrc+fsrc*fdst);
    }  
    
    return scale<T>(fsrc*fdst+fsrc-pow(fsrc,2)); 
}

template<class T>
inline T cfModulo(T src, T dst) {
    using namespace Arithmetic;
    
    return mod(dst,src); 
}

template<class T>
inline T cfModuloShift(T src, T dst) {
    using namespace Arithmetic;
    qreal fsrc = scale<qreal>(src);
    qreal fdst = scale<qreal>(dst);
    
    if (fsrc == 1.0 && fdst == 0.0) {
    return scale<T>(0.0);
    }  

    
    return scale<T>(mod((fdst+fsrc),1.0000000000)); 
}

template<class T>
inline T cfModuloShiftContinuous(T src, T dst) {
    using namespace Arithmetic;
    //This blending mode do not behave like difference/equivalent with destination layer inverted if you use group layer on addition while the content of group layer contains several addition-mode layers, it works as expected on float images. So, no need to change this.
    qreal fsrc = scale<qreal>(src);
    qreal fdst = scale<qreal>(dst);
    
    if (fsrc == 1.0 && fdst == 0.0) {
    return scale<T>(1.0);
    }  
    
    return scale<T>((int(ceil(fdst+fsrc)) % 2 != 0) || (fdst == zeroValue<T>()) ? cfModuloShift(fsrc,fdst) : inv(cfModuloShift(fsrc,fdst))); 
}

template<class T>
inline T cfDivisiveModulo(T src, T dst) {
    using namespace Arithmetic;
    //I have to use 1.00000 as unitValue failed to work for those area.
    
    qreal fsrc = scale<qreal>(src);
    qreal fdst = scale<qreal>(dst);
    
        if (fsrc == zeroValue<T>()) {
        return scale<T>(mod(((1.0000000000/epsilon<T>()) * fdst),1.0000000000));
    }  
    
    return scale<T>(mod(((1.0000000000/fsrc) * fdst),1.0000000000)); 
}

template<class T>
inline T cfDivisiveModuloContinuous(T src, T dst) {
    using namespace Arithmetic;
    
    qreal fsrc = scale<qreal>(src);
    qreal fdst = scale<qreal>(dst);
    
    if (fdst == zeroValue<T>()) {
    return zeroValue<T>();
    }  
    
    if (fsrc == zeroValue<T>()) {
    return cfDivisiveModulo(fsrc,fdst);
    }  

    
    return scale<T>( int(ceil(fdst/fsrc)) % 2 != 0 ? cfDivisiveModulo(fsrc,fdst) : inv(cfDivisiveModulo(fsrc,fdst))); 
}

template<class T>
inline T cfModuloContinuous(T src, T dst) {
    using namespace Arithmetic;
    
    return cfMultiply(cfDivisiveModuloContinuous(src,dst),src); 
}

template<class T>
inline T cfEasyDodge(T src, T dst) {
    using namespace Arithmetic;
    // The 13 divided by 15 can be adjusted to taste. See imgblend.m
    
    qreal fsrc = scale<qreal>(src);
    qreal fdst = scale<qreal>(dst);
    
    if (fsrc == 1.0) {
        return scale<T>(1.0);}

    
    return scale<T>(pow(fdst,mul(inv(fsrc != 1.0 ? fsrc : .999999999999),1.039999999))); 
}

template<class T>
inline T cfEasyBurn(T src, T dst) {
    using namespace Arithmetic;
    // The 13 divided by 15 can be adjusted to taste. See imgblend.m
    
    qreal fsrc = scale<qreal>(src);
    qreal fdst = scale<qreal>(dst);

    
    return scale<T>(inv(pow(inv(fsrc != 1.0 ? fsrc : .999999999999),mul(fdst,1.039999999)))); 
}

template<typename T>
struct CFFlatLight : KoClampedSourceAndDestinationCompositeOpGenericFunctorBase<T> {
    static inline T composeChannel(T src, T dst) {
        using namespace Arithmetic;

        if (isZeroValueFuzzy<T>(src)) {
            return zeroValue<T>();
        }

        return clamp<T>(isUnitValueFuzzy<T>(CFHardMixPhotoshop<T>::composeChannel(inv(src),dst)) ?
                            CFPenumbraB<T>::composeChannel(src,dst) :
                            CFPenumbraA<T>::composeChannel(src,dst));
    }
};


template<class HSXType, class TReal>
inline void cfAdditionSAI(TReal src, TReal sa, TReal& dst, TReal& da)
{
    using namespace Arithmetic;
    typedef typename KoColorSpaceMathsTraits<TReal>::compositetype composite_type;

    Q_UNUSED(da);
    composite_type newsrc;
    newsrc = mul(src, sa);
    dst = clamp<TReal>(newsrc + dst);
}



#endif // KOCOMPOSITEOP_FUNCTIONS_H_
