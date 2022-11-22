/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <xsimd_extensions/xsimd.hpp>

#if defined(HAVE_XSIMD) && !defined(XSIMD_NO_SUPPORTED_ARCHITECTURE) && XSIMD_UNIVERSAL_BUILD_PASS

#include "kis_circle_mask_generator.h"
#include "kis_circle_mask_generator_p.h"
#include "kis_curve_circle_mask_generator.h"
#include "kis_curve_circle_mask_generator_p.h"
#include "kis_curve_rect_mask_generator.h"
#include "kis_curve_rect_mask_generator_p.h"
#include "kis_gauss_circle_mask_generator.h"
#include "kis_gauss_circle_mask_generator_p.h"
#include "kis_gauss_rect_mask_generator.h"
#include "kis_gauss_rect_mask_generator_p.h"
#include "kis_rect_mask_generator.h"
#include "kis_rect_mask_generator_p.h"

#include "kis_brush_mask_applicator_base.h"
#include "kis_brush_mask_vector_applicator.h"

#include "vc_extra_math.h"

#define a(_s) #_s
#define b(_s) a(_s)

template<>
template<>
void FastRowProcessor<KisCircleMaskGenerator>::process<xsimd::current_arch>(float *buffer,
                                                                            int width,
                                                                            float y,
                                                                            float cosa,
                                                                            float sina,
                                                                            float centerX,
                                                                            float centerY)
{
    using float_v = xsimd::batch<float, xsimd::current_arch>;
    using float_m = typename float_v::batch_bool_type;

    const bool useSmoothing = d->copyOfAntialiasEdges;

    const float y_ = y - centerY;
    const float sinay_ = sina * y_;
    const float cosay_ = cosa * y_;

    float *bufferPointer = buffer;

    float_v currentIndices =
        xsimd::detail::make_sequence_as_batch<float_v>();

    const float_v increment((float)float_v::size);
    const float_v vCenterX(centerX);

    const float_v vCosa(cosa);
    const float_v vSina(sina);
    const float_v vCosaY_(cosay_);
    const float_v vSinaY_(sinay_);

    const float_v vXCoeff(static_cast<float>(d->xcoef));
    const float_v vYCoeff(static_cast<float>(d->ycoef));

    const float_v vTransformedFadeX(static_cast<float>(d->transformedFadeX));
    const float_v vTransformedFadeY(static_cast<float>(d->transformedFadeY));

    const float_v vOne(1);

    for (size_t i = 0; i < static_cast<size_t>(width); i += float_v::size) {
        const float_v x_ = currentIndices - vCenterX;

        float_v xr = x_ * vCosa - vSinaY_;
        float_v yr = x_ * vSina + vCosaY_;

        const float_v n = xsimd::pow2(xr * vXCoeff) + xsimd::pow2(yr * vYCoeff);
        const float_m outsideMask = n > vOne;

        if (!xsimd::all(outsideMask)) {
            if (useSmoothing) {
                xr = xsimd::abs(xr) + vOne;
                yr = xsimd::abs(yr) + vOne;
            }
            float_v vNormFade = xsimd::pow2(xr * vTransformedFadeX)
                + xsimd::pow2(yr * vTransformedFadeY);
            const float_m vNormLowMask = vNormFade < vOne;
            vNormFade = xsimd::set_zero(vNormFade, vNormLowMask);

            // 255 * n * (normeFade - 1) / (normeFade - n)
            float_v vFade = n * (vNormFade - vOne) / (vNormFade - n);

            // Mask in the inner circle of the mask
            const float_m mask = vNormFade < vOne;
            vFade = xsimd::set_zero(vFade, mask);

            // Mask out the outer circle of the mask
            vFade = xsimd::set_one(vFade, outsideMask);

            vFade.store_aligned(bufferPointer);
        } else {
            // Mask out everything outside the circle
            vOne.store_aligned(bufferPointer);
        }

        currentIndices = currentIndices + increment;

        bufferPointer += float_v::size;
    }
}

template<>
template<>
void FastRowProcessor<KisGaussCircleMaskGenerator>::process<xsimd::current_arch>(float *buffer,
                                                                                 int width,
                                                                                 float y,
                                                                                 float cosa,
                                                                                 float sina,
                                                                                 float centerX,
                                                                                 float centerY)
{
    using float_v = xsimd::batch<float, xsimd::current_arch>;
    using float_m = float_v::batch_bool_type;

    const float y_ = y - centerY;
    const float sinay_ = sina * y_;
    const float cosay_ = cosa * y_;

    float *bufferPointer = buffer;

    float_v currentIndices = xsimd::detail::make_sequence_as_batch<float_v>();

    const float_v increment(static_cast<float>(float_v::size));
    const float_v vCenterX(centerX);
    const float_v vCenter(static_cast<float>(d->center));

    const float_v vCosa(cosa);
    const float_v vSina(sina);
    const float_v vCosaY_(cosay_);
    const float_v vSinaY_(sinay_);

    const float_v vYCoeff(static_cast<float>(d->ycoef));
    const float_v vDistfactor(static_cast<float>(d->distfactor));
    const float_v vAlphafactor(static_cast<float>(d->alphafactor));

    const float_v vZero(0);
    const float_v vValMax(255.f);

    for (size_t i = 0; i < static_cast<size_t>(width); i += float_v::size) {
        const float_v x_ = currentIndices - vCenterX;

        const float_v xr = x_ * vCosa - vSinaY_;
        const float_v yr = x_ * vSina + vCosaY_;

        float_v dist =
            xsimd::sqrt(xsimd::pow2(xr) + xsimd::pow2(yr * vYCoeff));

        // Apply FadeMaker mask and operations
        const float_m excludeMask = d->fadeMaker.needFade(dist);

        if (!xsimd::all(excludeMask)) {
            const float_v valDist = dist * vDistfactor;
            float_v fullFade = vAlphafactor
                * (VcExtraMath::erf(valDist + vCenter)
                   - VcExtraMath::erf(valDist - vCenter));

            // Mask in the inner circle of the mask
            const float_m mask = fullFade < vZero;
            fullFade = xsimd::set_zero(fullFade, mask);

            // Mask the outer circle
            const float_m outerMask = fullFade > 254.974f;
            fullFade = xsimd::select(outerMask, vValMax, fullFade);

            // Mask (value - value), precision errors.
            float_v vFade = (vValMax - fullFade) / vValMax;

            // return original dist values before vFade transform
            vFade = xsimd::select(excludeMask, dist, vFade);
            vFade.store_aligned(bufferPointer);
        } else {
            dist.store_aligned(bufferPointer);
        }
        currentIndices = currentIndices + increment;

        bufferPointer += float_v::size;
    }
}

template<>
template<>
void FastRowProcessor<KisCurveCircleMaskGenerator>::process<xsimd::current_arch>(float *buffer,
                                                                                 int width,
                                                                                 float y,
                                                                                 float cosa,
                                                                                 float sina,
                                                                                 float centerX,
                                                                                 float centerY)
{
    using int_v = xsimd::batch<int, xsimd::current_arch>;
    using float_v = xsimd::batch<float, xsimd::current_arch>;
    using float_m = float_v::batch_bool_type;

    const float y_ = y - centerY;
    const float sinay_ = sina * y_;
    const float cosay_ = cosa * y_;

    float *bufferPointer = buffer;

    const qreal *curveDataPointer = d->curveData.data();

    float_v currentIndices = xsimd::detail::make_sequence_as_batch<float_v>();

    const float_v increment((float)float_v::size);
    const float_v vCenterX(centerX);

    const float_v vCosa(cosa);
    const float_v vSina(sina);
    const float_v vCosaY_(cosay_);
    const float_v vSinaY_(sinay_);

    const float_v vYCoeff(static_cast<float>(d->ycoef));
    const float_v vXCoeff(static_cast<float>(d->xcoef));
    const float_v vCurveResolution(static_cast<float>(d->curveResolution));

    float_v vCurvedData(0);
    float_v vCurvedData1(0);

    const float_v vOne(1);
    const float_v vZero(0);

    for (size_t i = 0; i < static_cast<size_t>(width); i += float_v::size) {
        const float_v x_ = currentIndices - vCenterX;

        const float_v xr = x_ * vCosa - vSinaY_;
        const float_v yr = x_ * vSina + vCosaY_;

        float_v dist = xsimd::pow2(xr * vXCoeff) + xsimd::pow2(yr * vYCoeff);

        // Apply FadeMaker mask and operations
        const float_m excludeMask = d->fadeMaker.needFade(dist);

        if (!xsimd::all(excludeMask)) {
            const float_v valDist = dist * vCurveResolution;
            // truncate
            int_v vAlphaValue = xsimd::to_int(valDist);
            const float_v vFloatAlphaValue = xsimd::to_float(vAlphaValue);

            const float_v alphaValueF = valDist - vFloatAlphaValue;

            const auto alphaMask = vAlphaValue < int_v(0);
            vAlphaValue = xsimd::set_zero(vAlphaValue, alphaMask);

            vCurvedData = float_v::gather(curveDataPointer, vAlphaValue);
            vCurvedData1 = float_v::gather(curveDataPointer, vAlphaValue + 1);

            // vAlpha
            float_v fullFade = ((vOne - alphaValueF) * vCurvedData
                                      + alphaValueF * vCurvedData1);

            // Mask in the inner circle of the mask
            const float_m mask = fullFade < vZero;
            fullFade = xsimd::set_zero(fullFade, mask);

            // Mask outer circle of mask
            const float_m outerMask = fullFade >= vOne;
            float_v vFade = (vOne - fullFade);
            vFade = xsimd::set_zero(vFade, outerMask);

            // return original dist values before vFade transform
            vFade = xsimd::select(excludeMask, dist, vFade);
            vFade.store_aligned(bufferPointer);
        } else {
            dist.store_aligned(bufferPointer);
        }
        currentIndices = currentIndices + increment;

        bufferPointer += float_v::size;
    }
}

template<>
template<>
void FastRowProcessor<KisRectangleMaskGenerator>::process<xsimd::current_arch>(float *buffer,
                                                                               int width,
                                                                               float y,
                                                                               float cosa,
                                                                               float sina,
                                                                               float centerX,
                                                                               float centerY)
{
    using float_v = xsimd::batch<float, xsimd::current_arch>;
    using float_m = float_v::batch_bool_type;

    const bool useSmoothing = d->copyOfAntialiasEdges;

    const float y_ = y - centerY;
    const float sinay_ = sina * y_;
    const float cosay_ = cosa * y_;

    float *bufferPointer = buffer;

    float_v currentIndices = xsimd::detail::make_sequence_as_batch<float_v>();

    const float_v increment((float)float_v::size);
    const float_v vCenterX(centerX);

    const float_v vCosa(cosa);
    const float_v vSina(sina);
    const float_v vCosaY_(cosay_);
    const float_v vSinaY_(sinay_);

    const float_v vXCoeff(static_cast<float>(d->xcoeff));
    const float_v vYCoeff(static_cast<float>(d->ycoeff));

    const float_v vTransformedFadeX(static_cast<float>(d->transformedFadeX));
    const float_v vTransformedFadeY(static_cast<float>(d->transformedFadeY));

    const float_v vOne(1);
    const float_v vZero(0);
    const float_v vTolerance(10000.f);

    for (size_t i = 0; i < static_cast<size_t>(width); i += float_v::size) {
        const float_v x_ = currentIndices - vCenterX;

        float_v xr = xsimd::abs(x_ * vCosa - vSinaY_);
        float_v yr = xsimd::abs(x_ * vSina + vCosaY_);

        const float_v nxr = xr * vXCoeff;
        const float_v nyr = yr * vYCoeff;

        float_m outsideMask = (nxr > vOne) || (nyr > vOne);

        if (!xsimd::all(outsideMask)) {
            if (useSmoothing) {
                xr = xsimd::abs(xr) + vOne;
                yr = xsimd::abs(yr) + vOne;
            }

            const float_v fxr = xr * vTransformedFadeX;
            const float_v fyr = yr * vTransformedFadeY;

            const float_v fxrNorm = nxr * (fxr - vOne) / (fxr - nxr);
            const float_v fyrNorm = nyr * (fyr - vOne) / (fyr - nyr);

            float_v vFade(vZero);

            const float_m vFadeMask = fxrNorm < fyrNorm;
            float_v vMaxVal = vFade;
            vMaxVal = xsimd::select(fxr > vOne, fxrNorm, vMaxVal);
            vMaxVal = xsimd::select(vFadeMask && fyr > vOne, fyrNorm, vMaxVal);
            vFade = vMaxVal;

            // Mask out the outer circle of the mask
            vFade = xsimd::select(outsideMask, vOne, vFade);
            vFade.store_aligned(bufferPointer);
        } else {
            // Mask out everything outside the circle
            vOne.store_aligned(bufferPointer);
        }

        currentIndices = currentIndices + increment;

        bufferPointer += float_v::size;
    }
}

template<>
template<>
void FastRowProcessor<KisGaussRectangleMaskGenerator>::process<xsimd::current_arch>(float *buffer,
                                                                                    int width,
                                                                                    float y,
                                                                                    float cosa,
                                                                                    float sina,
                                                                                    float centerX,
                                                                                    float centerY)
{
    using float_v = xsimd::batch<float, xsimd::current_arch>;
    using float_m = float_v::batch_bool_type;

    const float y_ = y - centerY;
    const float sinay_ = sina * y_;
    const float cosay_ = cosa * y_;

    float *bufferPointer = buffer;

    float_v currentIndices = xsimd::detail::make_sequence_as_batch<float_v>();

    const float_v increment((float)float_v::size);
    const float_v vCenterX(centerX);

    const float_v vCosa(cosa);
    const float_v vSina(sina);
    const float_v vCosaY_(cosay_);
    const float_v vSinaY_(sinay_);

    const float_v vhalfWidth(static_cast<float>(d->halfWidth));
    const float_v vhalfHeight(static_cast<float>(d->halfHeight));
    const float_v vXFade(static_cast<float>(d->xfade));
    const float_v vYFade(static_cast<float>(d->yfade));

    const float_v vAlphafactor(static_cast<float>(d->alphafactor));

    const float_v vOne(1);
    const float_v vZero(0);
    const float_v vValMax(255.f);

    for (size_t i = 0; i < static_cast<size_t>(width); i += float_v::size) {
        const float_v x_ = currentIndices - vCenterX;

        float_v xr = x_ * vCosa - vSinaY_;
        float_v yr = xsimd::abs(x_ * vSina + vCosaY_);

        // check if we need to apply fader on values
        float_m excludeMask = d->fadeMaker.needFade(xr, yr);
        const float_v vValue = xsimd::select(excludeMask, vOne, vValue);

        if (!xsimd::all(excludeMask)) {
            float_v fullFade = vValMax
                - (vAlphafactor * (VcExtraMath::erf((vhalfWidth + xr) * vXFade) + VcExtraMath::erf((vhalfWidth - xr) * vXFade))
                   * (VcExtraMath::erf((vhalfHeight + yr) * vYFade) + VcExtraMath::erf((vhalfHeight - yr) * vYFade)));

            // apply antialias fader
            d->fadeMaker.apply2DFader(fullFade, excludeMask, xr, yr);

            // Mask in the inner circle of the mask
            const float_m mask = fullFade < vZero;
            fullFade = xsimd::set_zero(fullFade, mask);

            // Mask the outer circle
            const float_m outerMask = fullFade > 254.974f;
            fullFade = xsimd::select(outerMask, vValMax, fullFade);

            // Mask (value - value), precision errors.
            float_v vFade = fullFade / vValMax;

            // return original vValue values before vFade transform
            vFade = xsimd::select(excludeMask, vValue, vFade);
            vFade.store_aligned(bufferPointer);

        } else {
            vValue.store_aligned(bufferPointer);
        }
        currentIndices = currentIndices + increment;

        bufferPointer += float_v::size;
    }
}

template<>
template<>
void FastRowProcessor<KisCurveRectangleMaskGenerator>::process<xsimd::current_arch>(float *buffer,
                                                                                    int width,
                                                                                    float y,
                                                                                    float cosa,
                                                                                    float sina,
                                                                                    float centerX,
                                                                                    float centerY)
{
    using float_v = xsimd::batch<float, xsimd::current_arch>;
    using float_m = float_v::batch_bool_type;

    const float y_ = y - centerY;
    const float sinay_ = sina * y_;
    const float cosay_ = cosa * y_;

    float *bufferPointer = buffer;

    const qreal *curveDataPointer = d->curveData.data();

    float_v currentIndices = xsimd::detail::make_sequence_as_batch<float_v>();

    const float_v increment((float)float_v::size);
    const float_v vCenterX(centerX);

    const float_v vCosa(cosa);
    const float_v vSina(sina);
    const float_v vCosaY_(cosay_);
    const float_v vSinaY_(sinay_);

    const float_v vYCoeff(static_cast<float>(d->ycoeff));
    const float_v vXCoeff(static_cast<float>(d->xcoeff));
    const float_v vCurveResolution(static_cast<float>(d->curveResolution));

    const float_v vOne(1);
    const float_v vZero(0);
    const float_v vValMax(255.f);

    for (size_t i = 0; i < static_cast<size_t>(width); i += float_v::size) {
        const float_v x_ = currentIndices - vCenterX;

        float_v xr = x_ * vCosa - vSinaY_;
        float_v yr = xsimd::abs(x_ * vSina + vCosaY_);

        // check if we need to apply fader on values
        float_m excludeMask = d->fadeMaker.needFade(xr, yr);
        const float_v vValue = xsimd::set_one(float_v(0), excludeMask);

        if (!xsimd::all(excludeMask)) {
            // We need to mask the extra area given for aliniation
            // the next operation should never give values above 1
            float_v preSIndex = xsimd::abs(xr) * vXCoeff;
            float_v preTIndex = xsimd::abs(yr) * vYCoeff;

            preSIndex = xsimd::select(preSIndex > vOne, vOne, preSIndex);
            preTIndex = xsimd::select(preTIndex > vOne, vOne, preTIndex);

            const auto sIndex = xsimd::nearbyint_as_int(preSIndex * vCurveResolution);
            const auto tIndex = xsimd::nearbyint_as_int(preTIndex * vCurveResolution);

            const auto sIndexInverted = xsimd::to_int(vCurveResolution - xsimd::to_float(sIndex));
            const auto tIndexInverted = xsimd::to_int(vCurveResolution - xsimd::to_float(tIndex));

            const auto vCurvedDataSIndex = float_v::gather(curveDataPointer, sIndex);
            const auto vCurvedDataTIndex = float_v::gather(curveDataPointer, tIndex);
            const auto vCurvedDataSIndexInv = float_v::gather(curveDataPointer, sIndexInverted);
            const auto vCurvedDataTIndexInv = float_v::gather(curveDataPointer, tIndexInverted);

            float_v fullFade = vValMax
                * (vOne
                   - (vCurvedDataSIndex * (vOne - vCurvedDataSIndexInv) * vCurvedDataTIndex
                      * (vOne - vCurvedDataTIndexInv)));

            // apply antialias fader
            d->fadeMaker.apply2DFader(fullFade, excludeMask, xr, yr);

            // Mask in the inner circle of the mask
            const float_m mask = fullFade < vZero;
            fullFade = xsimd::set_zero(fullFade, mask);

            // Mask the outer circle
            const float_m outerMask = fullFade > 254.974f;
            fullFade = xsimd::select(outerMask, vValMax, fullFade);

            // Mask (value - value), precision errors.
            float_v vFade = fullFade / vValMax;

            // return original vValue values before vFade transform
            vFade = xsimd::select(excludeMask, vValue, vFade);
            vFade.store_aligned(bufferPointer);
        } else {
            vValue.store_aligned(bufferPointer);
        }
        currentIndices = currentIndices + increment;

        bufferPointer += float_v::size;
    }
}

#endif /* defined HAVE_XSIMD && XSIMD_UNIVERSAL_BUILD_PASS */
