/*
 * Copyright (c) 2006 Cyrille Berger  <cberger@cberger.net>
 * Copyright (c) 2011 Silvio Heinrich <plassy@web.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KOOPTIMIZEDCOMPOSITEOPALPHADARKEN32_H_
#define KOOPTIMIZEDCOMPOSITEOPALPHADARKEN32_H_

#include "KoCompositeOpBase.h"

#include "KoStreamedMath.h"

template<typename channels_type, typename pixel_type>
struct AlphaDarkenCompositor32 {
    /**
     * This is a vector equivalent of compositeOnePixelScalar(). It is considered
     * to process Vc::float_v::Size pixels in a single pass.
     *
     * o the \p haveMask parameter points whether the real (non-null) mask
     *   pointer is passed to the function.
     * o the \p src pointer may be aligned to vector boundary or may be
     *   not. In case not, it must be pointed with a special parameter
     *   \p src_aligned.
     * o the \p dst pointer must always(!) be aligned to the boundary
     *   of a streaming vector. Unaligned writes are really expensive.
     * o This function is *never* used if HAVE_VC is not present
     */


    template<bool haveMask, bool src_aligned, Vc::Implementation _impl>
    static ALWAYS_INLINE void compositeVector(const quint8 *src, quint8 *dst, const quint8 *mask, float opacity, float flow)
    {
        Vc::float_v src_alpha;
        Vc::float_v dst_alpha;

        Vc::float_v opacity_vec(255.0 * opacity * flow);
        Vc::float_v flow_norm_vec(flow);


        Vc::float_v uint8MaxRec2((float)1.0 / (255.0 * 255.0));
        Vc::float_v uint8MaxRec1((float)1.0 / 255.0);
        Vc::float_v uint8Max((float)255.0);
        Vc::float_v zeroValue(Vc::Zero);


        Vc::float_v msk_norm_alpha;
        src_alpha = KoStreamedMath<_impl>::template fetch_alpha_32<src_aligned>(src);

        if (haveMask) {
            Vc::float_v mask_vec = KoStreamedMath<_impl>::fetch_mask_8(mask);
            msk_norm_alpha = src_alpha * mask_vec * uint8MaxRec2;
        } else {
            msk_norm_alpha = src_alpha * uint8MaxRec1;
        }

        dst_alpha = KoStreamedMath<_impl>::template fetch_alpha_32<true>(dst);
        src_alpha = msk_norm_alpha * opacity_vec;

        Vc::float_m empty_dst_pixels_mask = dst_alpha == zeroValue;

        Vc::float_v src_c1;
        Vc::float_v src_c2;
        Vc::float_v src_c3;

        Vc::float_v dst_c1;
        Vc::float_v dst_c2;
        Vc::float_v dst_c3;

        KoStreamedMath<_impl>::template fetch_colors_32<src_aligned>(src, src_c1, src_c2, src_c3);

        bool srcAlphaIsZero = (src_alpha == zeroValue).isFull();
        if (srcAlphaIsZero) return;

        bool dstAlphaIsZero = empty_dst_pixels_mask.isFull();

        Vc::float_v dst_blend = src_alpha * uint8MaxRec1;

        bool srcAlphaIsUnit = (src_alpha == uint8Max).isFull();

        if (dstAlphaIsZero) {
            dst_c1 = src_c1;
            dst_c2 = src_c2;
            dst_c3 = src_c3;
        } else if (srcAlphaIsUnit) {
            bool dstAlphaIsUnit = (dst_alpha == uint8Max).isFull();
            if (dstAlphaIsUnit) {
                memcpy(dst, src, 4 * Vc::float_v::Size);
                return;
            } else {
                dst_c1 = src_c1;
                dst_c2 = src_c2;
                dst_c3 = src_c3;
            }
        } else if (empty_dst_pixels_mask.isEmpty()) {
            KoStreamedMath<_impl>::template fetch_colors_32<true>(dst, dst_c1, dst_c2, dst_c3);
            dst_c1 = dst_blend * (src_c1 - dst_c1) + dst_c1;
            dst_c2 = dst_blend * (src_c2 - dst_c2) + dst_c2;
            dst_c3 = dst_blend * (src_c3 - dst_c3) + dst_c3;
        } else {
            KoStreamedMath<_impl>::template fetch_colors_32<true>(dst, dst_c1, dst_c2, dst_c3);
            dst_c1(empty_dst_pixels_mask) = src_c1;
            dst_c2(empty_dst_pixels_mask) = src_c2;
            dst_c3(empty_dst_pixels_mask) = src_c3;

            Vc::float_m not_empty_dst_pixels_mask = !empty_dst_pixels_mask;

            dst_c1(not_empty_dst_pixels_mask) = dst_blend * (src_c1 - dst_c1) + dst_c1;
            dst_c2(not_empty_dst_pixels_mask) = dst_blend * (src_c2 - dst_c2) + dst_c2;
            dst_c3(not_empty_dst_pixels_mask) = dst_blend * (src_c3 - dst_c3) + dst_c3;
        }

        Vc::float_v alpha1 = src_alpha + dst_alpha -
            dst_blend * dst_alpha;

        Vc::float_m alpha2_mask = opacity_vec > dst_alpha;
        Vc::float_v opt1 = (opacity_vec - dst_alpha) * msk_norm_alpha + dst_alpha;
        Vc::float_v alpha2;
        alpha2(!alpha2_mask) = dst_alpha;
        alpha2(alpha2_mask) = opt1;
        dst_alpha = (alpha2 - alpha1) * flow_norm_vec + alpha1;

        KoStreamedMath<_impl>::write_channels_32(dst, dst_alpha, dst_c1, dst_c2, dst_c3);
    }

    /**
     * Composes one pixel of the source into the destination
     */
    template <bool haveMask, Vc::Implementation _impl>
    static ALWAYS_INLINE void compositeOnePixelScalar(const channels_type *src, channels_type *dst, const quint8 *mask, float opacity, float flow, const QBitArray &channelFlags)
    {
        Q_UNUSED(channelFlags);

        using namespace Arithmetic;
        const qint32 alpha_pos = 3;

        const float uint8Rec1 = 1.0 / 255.0;
        const float uint8Rec2 = 1.0 / (255.0 * 255.0);
        const float uint8Max = 255.0;

        quint8 dstAlphaInt = dst[alpha_pos];
        float dstAlphaNorm = dstAlphaInt ? dstAlphaInt * uint8Rec1 : 0.0;
        float srcAlphaNorm;
        float mskAlphaNorm;

        /**
         * FIXME: precalculate this value on a higher level for
         * not doing it on every cycle
         */
        opacity *= flow;

        if (haveMask) {
            mskAlphaNorm = float(*mask) * uint8Rec2 * src[alpha_pos];
            srcAlphaNorm = mskAlphaNorm * opacity;
        } else {
            mskAlphaNorm = src[alpha_pos] * uint8Rec1;
            srcAlphaNorm = mskAlphaNorm * opacity;
        }

        if (dstAlphaInt != 0) {
            dst[0] = KoStreamedMath<_impl>::lerp_mixed_u8_float(dst[0], src[0], srcAlphaNorm);
            dst[1] = KoStreamedMath<_impl>::lerp_mixed_u8_float(dst[1], src[1], srcAlphaNorm);
            dst[2] = KoStreamedMath<_impl>::lerp_mixed_u8_float(dst[2], src[2], srcAlphaNorm);
        } else {
            const pixel_type *s = reinterpret_cast<const pixel_type*>(src);
            pixel_type *d = reinterpret_cast<pixel_type*>(dst);
            *d = *s;
        }

        float alpha1 = unionShapeOpacity(srcAlphaNorm, dstAlphaNorm);                               // alpha with 0% flow
        float alpha2 = (opacity > dstAlphaNorm) ? lerp(dstAlphaNorm, opacity, mskAlphaNorm) : dstAlphaNorm; // alpha with 100% flow
        dst[alpha_pos] = quint8(lerp(alpha1, alpha2, flow) * uint8Max);
    }
};

/**
 * An optimized version of a composite op for the use in 4 byte
 * colorspaces with alpha channel placed at the last byte of
 * the pixel: C1_C2_C3_A.
 */
template<Vc::Implementation _impl>
class KoOptimizedCompositeOpAlphaDarken32 : public KoCompositeOp
{
public:
    KoOptimizedCompositeOpAlphaDarken32(const KoColorSpace* cs)
        : KoCompositeOp(cs, COMPOSITE_ALPHA_DARKEN, i18n("Alpha darken"), KoCompositeOp::categoryMix()) {}

    using KoCompositeOp::composite;

    virtual void composite(const KoCompositeOp::ParameterInfo& params) const
    {
        if(params.maskRowStart) {
            KoStreamedMath<_impl>::template genericComposite32<true, true, AlphaDarkenCompositor32<quint8, quint32> >(params);
        } else {
            KoStreamedMath<_impl>::template genericComposite32<false, true, AlphaDarkenCompositor32<quint8, quint32> >(params);
        }
    }
};

#endif // KOOPTIMIZEDCOMPOSITEOPALPHADARKEN32_H_
