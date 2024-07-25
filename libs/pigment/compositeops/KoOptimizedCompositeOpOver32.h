/*
 * SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 * SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 * SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOOPTIMIZEDCOMPOSITEOPOVER32_H_
#define KOOPTIMIZEDCOMPOSITEOPOVER32_H_

#include <math.h>

#include "KoCompositeOpBase.h"
#include "KoCompositeOpRegistry.h"
#include "KoStreamedMath.h"


template<typename channels_type, typename pixel_type, bool alphaLocked, bool allChannelsFlag>
struct OverCompositor32 {
    struct ParamsWrapper {
        ParamsWrapper(const KoCompositeOp::ParameterInfo& params)
            : channelFlags(params.channelFlags)
        {
        }
        const QBitArray &channelFlags;
    };

    // \see docs in AlphaDarkenCompositor32
    template<bool haveMask, bool src_aligned, typename _impl>
    static ALWAYS_INLINE void compositeVector(const quint8 *src, quint8 *dst, const quint8 *mask, float opacity, const ParamsWrapper &oparams)
    {
        Q_UNUSED(oparams);

        using float_v = typename KoStreamedMath<_impl>::float_v;

        float_v src_alpha = KoStreamedMath<_impl>::template fetch_alpha_32<src_aligned>(src);
        float_v dst_alpha;

        const bool haveOpacity = opacity != 1.0f;
        const float_v opacity_norm_vec(opacity);

        const float_v uint8Max(255.0f);
        const float_v uint8MaxRec1(1.0f / 255.0f);
        const float_v zeroValue(0);
        const float_v oneValue(1);

        src_alpha *= opacity_norm_vec;

        if (haveMask) {
            const float_v mask_vec = KoStreamedMath<_impl>::fetch_mask_8(mask);
            src_alpha *= mask_vec * uint8MaxRec1;
        }

        // The source cannot change the colors in the destination,
        // since its fully transparent
        if (xsimd::all(src_alpha == zeroValue)) {
            return;
        }

        dst_alpha = KoStreamedMath<_impl>::template fetch_alpha_32<true>(dst);

        float_v src_c1;
        float_v src_c2;
        float_v src_c3;

        float_v dst_c1;
        float_v dst_c2;
        float_v dst_c3;


        KoStreamedMath<_impl>::template fetch_colors_32<src_aligned>(src, src_c1, src_c2, src_c3);
        float_v src_blend;
        float_v new_alpha;

        if (xsimd::all(dst_alpha == uint8Max)) {
            new_alpha = dst_alpha;
            src_blend = src_alpha * uint8MaxRec1;
        } else if (xsimd::all(dst_alpha == zeroValue)) {
            new_alpha = src_alpha;
            src_blend = oneValue;
        } else {
            /**
             * The value of new_alpha can have *some* zero values,
             * which will result in NaN values while division. But
             * when converted to integers these NaN values will
             * be converted to zeroes, which is exactly what we need
             */
            new_alpha = dst_alpha + (uint8Max - dst_alpha) * src_alpha * uint8MaxRec1;

            // Optimized version of:
            //     src_blend = src_alpha / new_alpha;
            src_blend = OptiDiv<_impl>::divVector(src_alpha, new_alpha);

        }

        if (!xsimd::all(src_blend == oneValue)) {
            KoStreamedMath<_impl>::template fetch_colors_32<true>(dst, dst_c1, dst_c2, dst_c3);

            dst_c1 = src_blend * (src_c1 - dst_c1) + dst_c1;
            dst_c2 = src_blend * (src_c2 - dst_c2) + dst_c2;
            dst_c3 = src_blend * (src_c3 - dst_c3) + dst_c3;

        } else {
            if (!haveMask && !haveOpacity) {
                memcpy(dst, src, 4 * float_v::size);
                return;
            } else {
                // opacity has changed the alpha of the source,
                // so we can't just memcpy the bytes
                dst_c1 = src_c1;
                dst_c2 = src_c2;
                dst_c3 = src_c3;
            }
        }

        KoStreamedMath<_impl>::write_channels_32(dst, new_alpha, dst_c1, dst_c2, dst_c3);
    }

    template <bool haveMask, typename _impl>
    static ALWAYS_INLINE void compositeOnePixelScalar(const channels_type *src, channels_type *dst, const quint8 *mask, float opacity, const ParamsWrapper &oparams)
    {
        using namespace Arithmetic;
        const qint32 alpha_pos = 3;

        const float uint8Rec1 = 1.0f / 255.0f;
        const float uint8Max = 255.0f;

        float srcAlpha = src[alpha_pos];
        srcAlpha *= opacity;

        if (haveMask) {
            srcAlpha *= float(*mask) * uint8Rec1;
        }

        if (srcAlpha != 0.0f) {

            float dstAlpha = dst[alpha_pos];
            float srcBlendNorm = NAN;

            if (alphaLocked || dstAlpha == uint8Max) {
                srcBlendNorm = srcAlpha * uint8Rec1;
            } else if (dstAlpha == 0.0f) {
                dstAlpha = srcAlpha;
                srcBlendNorm = 1.0f;

                if (!allChannelsFlag) {
                    auto *d = reinterpret_cast<pixel_type*>(dst);
                    *d = 0; // dstAlpha is already null
                }
            } else {
                dstAlpha += (uint8Max - dstAlpha) * srcAlpha * uint8Rec1;
                // Optimized version of:
                //     srcBlendNorm = srcAlpha / dstAlpha);
                srcBlendNorm = OptiDiv<_impl>::divScalar(srcAlpha, dstAlpha);
            }

            if(allChannelsFlag) {
                if (srcBlendNorm == 1.0f) {
                    if (!alphaLocked) {
                        const auto *s = reinterpret_cast<const pixel_type*>(src);
                        auto *d = reinterpret_cast<pixel_type*>(dst);
                        *d = *s;
                    } else {
                        dst[0] = src[0];
                        dst[1] = src[1];
                        dst[2] = src[2];
                    }
                } else if (srcBlendNorm != 0.0f){
                    dst[0] = KoStreamedMath<_impl>::lerp_mixed_u8_float(dst[0], src[0], srcBlendNorm);
                    dst[1] = KoStreamedMath<_impl>::lerp_mixed_u8_float(dst[1], src[1], srcBlendNorm);
                    dst[2] = KoStreamedMath<_impl>::lerp_mixed_u8_float(dst[2], src[2], srcBlendNorm);
                }
            } else {
                const QBitArray &channelFlags = oparams.channelFlags;

                if (srcBlendNorm == 1.0f) {
                    if(channelFlags.at(0)) dst[0] = src[0];
                    if(channelFlags.at(1)) dst[1] = src[1];
                    if(channelFlags.at(2)) dst[2] = src[2];
                } else if (srcBlendNorm != 0.0f) {
                    if(channelFlags.at(0)) dst[0] = KoStreamedMath<_impl>::lerp_mixed_u8_float(dst[0], src[0], srcBlendNorm);
                    if(channelFlags.at(1)) dst[1] = KoStreamedMath<_impl>::lerp_mixed_u8_float(dst[1], src[1], srcBlendNorm);
                    if(channelFlags.at(2)) dst[2] = KoStreamedMath<_impl>::lerp_mixed_u8_float(dst[2], src[2], srcBlendNorm);
                }
            }

            if (!alphaLocked) {
                dst[alpha_pos] = KoStreamedMath<_impl>::round_float_to_u8(dstAlpha);
            }
        }
    }
};

/**
 * An optimized version of a composite op for the use in 4 byte
 * colorspaces with alpha channel placed at the last byte of
 * the pixel: C1_C2_C3_A.
 */
template<typename _impl>
class KoOptimizedCompositeOpOver32 : public KoCompositeOp
{
public:
    KoOptimizedCompositeOpOver32(const KoColorSpace* cs)
        : KoCompositeOp(cs, COMPOSITE_OVER, KoCompositeOp::categoryMix()) {}

    using KoCompositeOp::composite;

    void composite(const KoCompositeOp::ParameterInfo& params) const override
    {
        if(params.maskRowStart) {
            composite<true>(params);
        } else {
            composite<false>(params);
        }
    }

    template <bool haveMask>
    inline void composite(const KoCompositeOp::ParameterInfo& params) const {
        if (params.channelFlags.isEmpty() ||
            params.channelFlags == QBitArray(4, true)) {

            KoStreamedMath<_impl>::template genericComposite32<haveMask, false, OverCompositor32<quint8, quint32, false, true> >(params);
        } else {
            const bool allChannelsFlag =
                params.channelFlags.at(0) &&
                params.channelFlags.at(1) &&
                params.channelFlags.at(2);

            const bool alphaLocked =
                !params.channelFlags.at(3);

            if (allChannelsFlag && alphaLocked) {
                KoStreamedMath<_impl>::template genericComposite32_novector<haveMask, false, OverCompositor32<quint8, quint32, true, true> >(params);
            } else if (!allChannelsFlag && !alphaLocked) {
                KoStreamedMath<_impl>::template genericComposite32_novector<haveMask, false, OverCompositor32<quint8, quint32, false, false> >(params);
            } else /*if (!allChannelsFlag && alphaLocked) */{
                KoStreamedMath<_impl>::template genericComposite32_novector<haveMask, false, OverCompositor32<quint8, quint32, true, false> >(params);
            }
        }
    }
};

#endif // KOOPTIMIZEDCOMPOSITEOPOVER32_H_
