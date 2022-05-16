/*
 * SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 * SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 *  SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOOPTIMIZEDCOMPOSITEOPALPHADARKEN32_H_
#define KOOPTIMIZEDCOMPOSITEOPALPHADARKEN32_H_

#include "KoCompositeOpBase.h"
#include "KoCompositeOpRegistry.h"
#include "KoStreamedMath.h"
#include "KoAlphaDarkenParamsWrapper.h"

template<typename channels_type, typename pixel_type, typename ParamsWrapperT>
struct AlphaDarkenCompositor32 {
    using ParamsWrapper = ParamsWrapperT;

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

        // we don't use directly passed value
        Q_UNUSED(opacity);

        // instead we use value calculated by ParamsWrapper
        opacity = oparams.opacity;
        const float_v opacity_vec(255.0f * opacity);

        const float_v average_opacity_vec(255.0 * oparams.averageOpacity);
        const float_v flow_norm_vec(oparams.flow);

        const float_v uint8MaxRec2(1.0f / (255.0f * 255.0f));
        const float_v uint8MaxRec1(1.0f / 255.0f);
        const float_v uint8Max(255.0f);
        const float_v zeroValue(0);

        float_v src_alpha = KoStreamedMath<_impl>::template fetch_alpha_32<src_aligned>(src);

        const float_v msk_norm_alpha = [&]() {
            if (haveMask) {
                const float_v mask_vec = KoStreamedMath<_impl>::fetch_mask_8(mask);
                return src_alpha * mask_vec * uint8MaxRec2;
            } else {
                return src_alpha * uint8MaxRec1;
            }
        }();

        float_v dst_alpha = KoStreamedMath<_impl>::template fetch_alpha_32<true>(dst);
        src_alpha = msk_norm_alpha * opacity_vec;

        const float_m empty_dst_pixels_mask = dst_alpha == zeroValue;

        float_v src_c1;
        float_v src_c2;
        float_v src_c3;

        float_v dst_c1;
        float_v dst_c2;
        float_v dst_c3;

        KoStreamedMath<_impl>::template fetch_colors_32<src_aligned>(src, src_c1, src_c2, src_c3);

        const bool srcAlphaIsZero = xsimd::all(src_alpha == zeroValue);
        if (srcAlphaIsZero) return;

        const bool dstAlphaIsZero = xsimd::all(empty_dst_pixels_mask);

        const float_v dst_blend = src_alpha * uint8MaxRec1;

        const bool srcAlphaIsUnit = xsimd::all(src_alpha == uint8Max);

        if (dstAlphaIsZero) {
            dst_c1 = src_c1;
            dst_c2 = src_c2;
            dst_c3 = src_c3;
        } else if (srcAlphaIsUnit) {
            const bool dstAlphaIsUnit = xsimd::all(dst_alpha == uint8Max);
            if (dstAlphaIsUnit) {
                memcpy(dst, src, 4 * float_v::size);
                return;
            } else {
                dst_c1 = src_c1;
                dst_c2 = src_c2;
                dst_c3 = src_c3;
            }
        } else if (xsimd::none(empty_dst_pixels_mask)) {
            KoStreamedMath<_impl>::template fetch_colors_32<true>(dst, dst_c1, dst_c2, dst_c3);
            dst_c1 = dst_blend * (src_c1 - dst_c1) + dst_c1;
            dst_c2 = dst_blend * (src_c2 - dst_c2) + dst_c2;
            dst_c3 = dst_blend * (src_c3 - dst_c3) + dst_c3;
        } else {
            KoStreamedMath<_impl>::template fetch_colors_32<true>(dst, dst_c1, dst_c2, dst_c3);
            dst_c1 = xsimd::select(empty_dst_pixels_mask, src_c1, dst_blend * (src_c1 - dst_c1) + dst_c1);
            dst_c2 = xsimd::select(empty_dst_pixels_mask, src_c2, dst_blend * (src_c2 - dst_c2) + dst_c2);
            dst_c3 = xsimd::select(empty_dst_pixels_mask, src_c3, dst_blend * (src_c3 - dst_c3) + dst_c3);
        }

        const float_v fullFlowAlpha = [&]() {
            if (oparams.averageOpacity > opacity) {
                const float_m fullFlowAlpha_mask = average_opacity_vec > dst_alpha;

                if (xsimd::none(fullFlowAlpha_mask)) {
                    return dst_alpha;
                } else {
                    const float_v reverse_blend = dst_alpha / average_opacity_vec;
                    const float_v opt1 = (average_opacity_vec - src_alpha) * reverse_blend + src_alpha;
                    return xsimd::select(fullFlowAlpha_mask, opt1, dst_alpha);
                }
            } else {
                const float_m fullFlowAlpha_mask = opacity_vec > dst_alpha;

                if (xsimd::none(fullFlowAlpha_mask)) {
                    return dst_alpha;
                } else {
                    const float_v opt1 = (opacity_vec - dst_alpha) * msk_norm_alpha + dst_alpha;
                    return xsimd::select(fullFlowAlpha_mask, opt1, dst_alpha);
                }
            }
        }();

        dst_alpha = [&]() {
            if (oparams.flow == 1.0) {
                return fullFlowAlpha;
            } else {
                const float_v zeroFlowAlpha = ParamsWrapper::calculateZeroFlowAlpha(src_alpha, dst_alpha, uint8MaxRec1);
                return (fullFlowAlpha - zeroFlowAlpha) * flow_norm_vec + zeroFlowAlpha;
            }
        }();

        KoStreamedMath<_impl>::write_channels_32(dst, dst_alpha, dst_c1, dst_c2, dst_c3);
    }

    /**
     * Composes one pixel of the source into the destination
     */
    template <bool haveMask, typename _impl>
    static ALWAYS_INLINE void compositeOnePixelScalar(const channels_type *src, channels_type *dst, const quint8 *mask, float opacity, const ParamsWrapper &oparams)
    {
        using namespace Arithmetic;
        const qint32 alpha_pos = 3;

        const float uint8Rec1 = 1.0f / 255.0f;
        const float uint8Rec2 = 1.0f / (255.0f * 255.0f);
        const float uint8Max = 255.0;

        // we don't use directly passed value
        Q_UNUSED(opacity);

        // instead we use value calculated by ParamsWrapper
        opacity = oparams.opacity;

        const quint8 dstAlphaInt = dst[alpha_pos];
        const float dstAlphaNorm = dstAlphaInt ? dstAlphaInt * uint8Rec1 : 0.0;
        const float mskAlphaNorm = [&]() {
            if (haveMask) {
                return float(*mask) * uint8Rec2 * src[alpha_pos];
            } else {
                return src[alpha_pos] * uint8Rec1;
            }
        }();
        const float srcAlphaNorm = mskAlphaNorm * opacity;

        if (dstAlphaInt != 0) {
            dst[0] = KoStreamedMath<_impl>::lerp_mixed_u8_float(dst[0], src[0], srcAlphaNorm);
            dst[1] = KoStreamedMath<_impl>::lerp_mixed_u8_float(dst[1], src[1], srcAlphaNorm);
            dst[2] = KoStreamedMath<_impl>::lerp_mixed_u8_float(dst[2], src[2], srcAlphaNorm);
        } else {
            const auto *s = reinterpret_cast<const pixel_type*>(src);
            auto *d = reinterpret_cast<pixel_type*>(dst);
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

        const float dstAlpha = [&](){
            if (flow == 1.0f) {
                return fullFlowAlpha * uint8Max;
            } else {
                const float zeroFlowAlpha = ParamsWrapper::calculateZeroFlowAlpha(srcAlphaNorm, dstAlphaNorm);
                return lerp(zeroFlowAlpha, fullFlowAlpha, flow) * uint8Max;
            }
        }();

        dst[alpha_pos] = KoStreamedMath<_impl>::round_float_to_u8(dstAlpha);
    }
};

/**
 * An optimized version of a composite op for the use in 4 byte
 * colorspaces with alpha channel placed at the last byte of
 * the pixel: C1_C2_C3_A.
 */
template<typename _impl, class ParamsWrapper>
class KoOptimizedCompositeOpAlphaDarken32Impl : public KoCompositeOp
{
public:
    KoOptimizedCompositeOpAlphaDarken32Impl(const KoColorSpace* cs)
        : KoCompositeOp(cs, COMPOSITE_ALPHA_DARKEN, KoCompositeOp::categoryMix()) {}

    using KoCompositeOp::composite;

    void composite(const KoCompositeOp::ParameterInfo& params) const override
    {
        if(params.maskRowStart) {
            KoStreamedMath<_impl>::template genericComposite32<true, true, AlphaDarkenCompositor32<quint8, quint32, ParamsWrapper> >(params);
        } else {
            KoStreamedMath<_impl>::template genericComposite32<false, true, AlphaDarkenCompositor32<quint8, quint32, ParamsWrapper> >(params);
        }
    }
};

template<typename _impl = xsimd::current_arch>
class KoOptimizedCompositeOpAlphaDarkenHard32 :
        public KoOptimizedCompositeOpAlphaDarken32Impl<_impl, KoAlphaDarkenParamsWrapperHard>
{
public:
    KoOptimizedCompositeOpAlphaDarkenHard32(const KoColorSpace *cs)
        : KoOptimizedCompositeOpAlphaDarken32Impl<_impl, KoAlphaDarkenParamsWrapperHard>(cs) {
    }
};

template<typename _impl = xsimd::current_arch>
class KoOptimizedCompositeOpAlphaDarkenCreamy32 :
        public KoOptimizedCompositeOpAlphaDarken32Impl<_impl, KoAlphaDarkenParamsWrapperCreamy>
{
public:
    KoOptimizedCompositeOpAlphaDarkenCreamy32(const KoColorSpace *cs)
        : KoOptimizedCompositeOpAlphaDarken32Impl<_impl, KoAlphaDarkenParamsWrapperCreamy>(cs) {
    }
};


#endif // KOOPTIMIZEDCOMPOSITEOPALPHADARKEN32_H_
