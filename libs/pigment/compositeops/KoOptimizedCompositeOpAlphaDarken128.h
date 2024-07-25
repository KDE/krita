/*
 * SPDX-FileCopyrightText: 2016 Thorsten Zachmann <zachmann@kde.org>
 * SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOOPTIMIZEDCOMPOSITEOPALPHADARKEN128_H
#define KOOPTIMIZEDCOMPOSITEOPALPHADARKEN128_H

#include "KoCompositeOpBase.h"
#include "KoCompositeOpRegistry.h"
#include "KoStreamedMath.h"
#include "KoAlphaDarkenParamsWrapper.h"

template<typename channels_type, typename ParamsWrapperT>
struct AlphaDarkenCompositor128 {
    using ParamsWrapper = ParamsWrapperT;

    struct Pixel {
        channels_type red;
        channels_type green;
        channels_type blue;
        channels_type alpha;
    };

    /**
     * This is a vector equivalent of compositeOnePixelScalar(). It is considered
     * to process float_v::size pixels in a single pass.
     *
     * o the \p haveMask parameter points whether the real (non-null) mask
     *   pointer is passed to the function.
     * o the \p src pointer may be aligned to vector boundary or may be
     *   not. In case not, it must be pointed with a special parameter
     *   \p src_aligned.
     * o the \p dst pointer must always(!) be aligned to the boundary
     *   of a streaming vector. Unaligned writes are really expensive.
     * o This function is *never* used if HAVE_XSIMD is not present
     */
    template<bool haveMask, bool src_aligned, typename _impl>
    static ALWAYS_INLINE void compositeVector(const quint8 *src, quint8 *dst, const quint8 *mask, float opacity, const ParamsWrapper &oparams)
    {
        using float_v = typename KoStreamedMath<_impl>::float_v;
        using float_m = typename float_v::batch_bool_type;

        float_v src_c1;
        float_v src_c2;
        float_v src_c3;
        float_v src_alpha;

        PixelWrapper<channels_type, _impl> dataWrapper;
        dataWrapper.read(src, src_c1, src_c2, src_c3, src_alpha);

        const float_v msk_norm_alpha = [&](){;
            if (haveMask) {
                const float_v uint8Rec1(1.0f / 255.0f);
                const float_v mask_vec = KoStreamedMath<_impl>::fetch_mask_8(mask);
                return mask_vec * uint8Rec1 * src_alpha;
            }
            else {
                return src_alpha;
            }
        }();

        // we don't use directly passed value
        Q_UNUSED(opacity);

        // instead we use value calculated by ParamsWrapper
        opacity = oparams.opacity;
        const float_v opacity_vec(opacity);

        src_alpha = msk_norm_alpha * opacity_vec;

        const float_v zeroValue(static_cast<float>(KoColorSpaceMathsTraits<channels_type>::zeroValue));

        float_v dst_c1;
        float_v dst_c2;
        float_v dst_c3;
        float_v dst_alpha;

        dataWrapper.read(dst, dst_c1, dst_c2, dst_c3, dst_alpha);

        const float_m empty_dst_pixels_mask = dst_alpha == zeroValue;

        if (!xsimd::all(empty_dst_pixels_mask)) {
            if (xsimd::none(empty_dst_pixels_mask)) {
                dst_c1 = (src_c1 - dst_c1) * src_alpha + dst_c1;
                dst_c2 = (src_c2 - dst_c2) * src_alpha + dst_c2;
                dst_c3 = (src_c3 - dst_c3) * src_alpha + dst_c3;
            }
            else {
                dst_c1 = xsimd::select(empty_dst_pixels_mask, src_c1, (src_c1 - dst_c1) * src_alpha + dst_c1);
                dst_c2 = xsimd::select(empty_dst_pixels_mask, src_c2, (src_c2 - dst_c2) * src_alpha + dst_c2);
                dst_c3 = xsimd::select(empty_dst_pixels_mask, src_c3, (src_c3 - dst_c3) * src_alpha + dst_c3);
            }
        }
        else {
            dst_c1 = src_c1;
            dst_c2 = src_c2;
            dst_c3 = src_c3;
        }

        const float_v fullFlowAlpha = [&]() {
            if (oparams.averageOpacity > opacity) {
                const float_v average_opacity_vec(oparams.averageOpacity);
                const float_m fullFlowAlpha_mask = average_opacity_vec > dst_alpha;
                return xsimd::select(fullFlowAlpha_mask,
                                (average_opacity_vec - src_alpha)
                                        * (dst_alpha / average_opacity_vec)
                                    + src_alpha,
                                dst_alpha);
            } else {
                const float_m fullFlowAlpha_mask = opacity_vec > dst_alpha;
                return xsimd::select(
                    fullFlowAlpha_mask,
                    (opacity_vec - dst_alpha) * msk_norm_alpha + dst_alpha,
                    dst_alpha);
            }
        }();

        dst_alpha = [&]() {
            if (oparams.flow == 1.0) {
                return fullFlowAlpha;
            }
            else {
                const float_v zeroFlowAlpha = ParamsWrapper::calculateZeroFlowAlpha(src_alpha, dst_alpha);
                const float_v flow_norm_vec(oparams.flow);
                return (fullFlowAlpha - zeroFlowAlpha) * flow_norm_vec + zeroFlowAlpha;
            }
        }();

        dataWrapper.write(dst, dst_c1, dst_c2, dst_c3, dst_alpha);
    }

    /**
     * Composes one pixel of the source into the destination
     */
    template <bool haveMask, typename _impl>
    static ALWAYS_INLINE void compositeOnePixelScalar(const quint8 *s, quint8 *d, const quint8 *mask, float opacity, const ParamsWrapper &oparams)
    {
        using namespace Arithmetic;
        const qint32 alpha_pos = 3;

        const auto *src = reinterpret_cast<const channels_type*>(s);
        auto *dst = reinterpret_cast<channels_type*>(d);

        float dstAlphaNorm = dst[alpha_pos];
        PixelWrapper<channels_type, _impl>::normalizeAlpha(dstAlphaNorm);

        const float uint8Rec1 = 1.0f / 255.0f;
        float mskAlphaNorm = haveMask ? float(*mask) * uint8Rec1 * src[alpha_pos] : src[alpha_pos];
        PixelWrapper<channels_type, _impl>::normalizeAlpha(mskAlphaNorm);

        Q_UNUSED(opacity);
        opacity = oparams.opacity;

        const float srcAlphaNorm = mskAlphaNorm * opacity;

        if (dstAlphaNorm != 0) {
            dst[0] = PixelWrapper<channels_type, _impl>::lerpMixedUintFloat(dst[0], src[0], srcAlphaNorm);
            dst[1] = PixelWrapper<channels_type, _impl>::lerpMixedUintFloat(dst[1], src[1], srcAlphaNorm);
            dst[2] = PixelWrapper<channels_type, _impl>::lerpMixedUintFloat(dst[2], src[2], srcAlphaNorm);
        } else {
            const auto *s = reinterpret_cast<const Pixel*>(src);
            auto *d = reinterpret_cast<Pixel*>(dst);
            *d = *s;
        }

        const float flow = oparams.flow;
        const float averageOpacity = oparams.averageOpacity;

        const float fullFlowAlpha = [&]() {
            if (averageOpacity > opacity) {
                return averageOpacity > dstAlphaNorm ? lerp(srcAlphaNorm, averageOpacity, dstAlphaNorm / averageOpacity) : dstAlphaNorm;
            } else {
                return opacity > dstAlphaNorm ? lerp(dstAlphaNorm, opacity, mskAlphaNorm) : dstAlphaNorm;
            }
        }();

        dstAlphaNorm = [&]() {
            if (flow == 1.0f) {
                return fullFlowAlpha;
            } else {
                const float zeroFlowAlpha = ParamsWrapper::calculateZeroFlowAlpha(srcAlphaNorm, dstAlphaNorm);
                return lerp(zeroFlowAlpha, fullFlowAlpha, flow);
            }
        }();

        PixelWrapper<channels_type, _impl>::denormalizeAlpha(dstAlphaNorm);
        dst[alpha_pos] = PixelWrapper<channels_type, _impl>::roundFloatToUint(dstAlphaNorm);
    }
};

/**
 * An optimized version of a composite op for the use in 16 byte
 * colorspaces with alpha channel placed at the last byte of
 * the pixel: C1_C2_C3_A.
 */
template<typename _impl, typename ParamsWrapper>
class KoOptimizedCompositeOpAlphaDarken128Impl : public KoCompositeOp
{
public:
    KoOptimizedCompositeOpAlphaDarken128Impl(const KoColorSpace* cs)
        : KoCompositeOp(cs, COMPOSITE_ALPHA_DARKEN, KoCompositeOp::categoryMix()) {}

    using KoCompositeOp::composite;

    void composite(const KoCompositeOp::ParameterInfo& params) const override
    {
        if(params.maskRowStart) {
            KoStreamedMath<_impl>::template genericComposite128<true, true, AlphaDarkenCompositor128<float, ParamsWrapper> >(params);
        } else {
            KoStreamedMath<_impl>::template genericComposite128<false, true, AlphaDarkenCompositor128<float, ParamsWrapper> >(params);
        }
    }
};

template<typename _impl>
class KoOptimizedCompositeOpAlphaDarkenHard128
    : public KoOptimizedCompositeOpAlphaDarken128Impl<_impl, KoAlphaDarkenParamsWrapperHard>
{
public:
    KoOptimizedCompositeOpAlphaDarkenHard128(const KoColorSpace* cs)
        : KoOptimizedCompositeOpAlphaDarken128Impl<_impl, KoAlphaDarkenParamsWrapperHard>(cs) {}
};

template<typename _impl>
class KoOptimizedCompositeOpAlphaDarkenCreamy128
    : public KoOptimizedCompositeOpAlphaDarken128Impl<_impl, KoAlphaDarkenParamsWrapperCreamy>
{
public:
    KoOptimizedCompositeOpAlphaDarkenCreamy128(const KoColorSpace* cs)
        : KoOptimizedCompositeOpAlphaDarken128Impl<_impl, KoAlphaDarkenParamsWrapperCreamy>(cs) {}
};

template<typename _impl, typename ParamsWrapper>
class KoOptimizedCompositeOpAlphaDarkenU64Impl : public KoCompositeOp
{
public:
    KoOptimizedCompositeOpAlphaDarkenU64Impl(const KoColorSpace* cs)
        : KoCompositeOp(cs, COMPOSITE_ALPHA_DARKEN, KoCompositeOp::categoryMix()) {}

    using KoCompositeOp::composite;

    void composite(const KoCompositeOp::ParameterInfo& params) const override
    {
        if(params.maskRowStart) {
            KoStreamedMath<_impl>::template genericComposite64<true, true, AlphaDarkenCompositor128<quint16, ParamsWrapper> >(params);
        } else {
            KoStreamedMath<_impl>::template genericComposite64<false, true, AlphaDarkenCompositor128<quint16, ParamsWrapper> >(params);
        }
    }
};

template<typename _impl>
class KoOptimizedCompositeOpAlphaDarkenHardU64
    : public KoOptimizedCompositeOpAlphaDarkenU64Impl<_impl, KoAlphaDarkenParamsWrapperHard>
{
public:
    KoOptimizedCompositeOpAlphaDarkenHardU64(const KoColorSpace* cs)
        : KoOptimizedCompositeOpAlphaDarkenU64Impl<_impl, KoAlphaDarkenParamsWrapperHard>(cs) {}
};

template<typename _impl>
class KoOptimizedCompositeOpAlphaDarkenCreamyU64
    : public KoOptimizedCompositeOpAlphaDarkenU64Impl<_impl, KoAlphaDarkenParamsWrapperCreamy>
{
public:
    KoOptimizedCompositeOpAlphaDarkenCreamyU64(const KoColorSpace* cs)
        : KoOptimizedCompositeOpAlphaDarkenU64Impl<_impl, KoAlphaDarkenParamsWrapperCreamy>(cs) {}
};


#endif // KOOPTIMIZEDCOMPOSITEOPALPHADARKEN128_H
