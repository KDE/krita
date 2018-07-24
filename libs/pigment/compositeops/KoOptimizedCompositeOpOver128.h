/*
 * Copyright (c) 2015 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef KOOPTIMIZEDCOMPOSITEOPOVER128_H_
#define KOOPTIMIZEDCOMPOSITEOPOVER128_H_

#include "KoCompositeOpBase.h"
#include "KoCompositeOpRegistry.h"
#include "KoStreamedMath.h"

#define NATIVE_OPACITY_OPAQUE KoColorSpaceMathsTraits<channels_type>::unitValue
#define NATIVE_OPACITY_TRANSPARENT KoColorSpaceMathsTraits<channels_type>::zeroValue

#define INFO_DEBUG 0

template<typename channels_type, typename pixel_type, bool alphaLocked, bool allChannelsFlag>
struct OverCompositor128 {
    struct OptionalParams {
        OptionalParams(const KoCompositeOp::ParameterInfo& params)
            : channelFlags(params.channelFlags)
        {
        }
        const QBitArray &channelFlags;
    };

    struct Pixel {
        channels_type red;
        channels_type green;
        channels_type blue;
        channels_type alpha;
    };

    // \see docs in AlphaDarkenCompositor32
    template<bool haveMask, bool src_aligned, Vc::Implementation _impl>
    static ALWAYS_INLINE void compositeVector(const quint8 *src, quint8 *dst, const quint8 *mask, float opacity, const OptionalParams &oparams)
    {
#if INFO_DEBUG
        static quint32 countTotal = 0;
        static quint32 countOne = 0;
        static quint32 countTwo = 0;
        static quint32 countThree = 0;
        static quint32 countFour = 0;

        if (++countTotal % 250000 == 0) {
            qInfo() << "count" << countOne << countTwo << countThree << countFour << countTotal << opacity;
        }
#endif
        Q_UNUSED(oparams);

        const Pixel *sp = reinterpret_cast<const Pixel*>(src);
        Pixel *dp = reinterpret_cast<Pixel*>(dst);

        Vc::float_v src_alpha;
        Vc::float_v dst_alpha;

        Vc::float_v src_c1;
        Vc::float_v src_c2;
        Vc::float_v src_c3;

        const Vc::float_v::IndexType indexes(Vc::IndexesFromZero);
        Vc::InterleavedMemoryWrapper<Pixel, Vc::float_v> data(const_cast<Pixel*>(sp));
        tie(src_c1, src_c2, src_c3, src_alpha) = data[indexes];

        //bool haveOpacity = opacity != 1.0;
        const Vc::float_v opacity_norm_vec(opacity);
        src_alpha *= opacity_norm_vec;

        if (haveMask) {
            const Vc::float_v uint8MaxRec1((float)1.0 / 255);
            Vc::float_v mask_vec = KoStreamedMath<_impl>::fetch_mask_8(mask);
            src_alpha *= mask_vec * uint8MaxRec1;
        }

        const Vc::float_v zeroValue(NATIVE_OPACITY_TRANSPARENT);
        // The source cannot change the colors in the destination,
        // since its fully transparent
        if ((src_alpha == zeroValue).isFull()) {
#if INFO_DEBUG
            countFour++;
#endif
            return;
        }

        Vc::float_v dst_c1;
        Vc::float_v dst_c2;
        Vc::float_v dst_c3;

        Vc::InterleavedMemoryWrapper<Pixel, Vc::float_v> dataDest(dp);
        tie(dst_c1, dst_c2, dst_c3, dst_alpha) = dataDest[indexes];

        Vc::float_v src_blend;
        Vc::float_v new_alpha;

        const Vc::float_v oneValue(NATIVE_OPACITY_OPAQUE);
        if ((dst_alpha == oneValue).isFull()) {
            new_alpha = dst_alpha;
            src_blend = src_alpha;
        } else if ((dst_alpha == zeroValue).isFull()) {
            new_alpha = src_alpha;
            src_blend = oneValue;
        } else {
            /**
             * The value of new_alpha can have *some* zero values,
             * which will result in NaN values while division.
             */
            new_alpha = dst_alpha + (oneValue - dst_alpha) * src_alpha;
            Vc::float_m mask = (new_alpha == zeroValue);
            src_blend = src_alpha / new_alpha;
            src_blend.setZero(mask);
        }

        if (!(src_blend == oneValue).isFull()) {
#if INFO_DEBUG
            ++countOne;
#endif

            dst_c1 = src_blend * (src_c1 - dst_c1) + dst_c1;
            dst_c2 = src_blend * (src_c2 - dst_c2) + dst_c2;
            dst_c3 = src_blend * (src_c3 - dst_c3) + dst_c3;

            dataDest[indexes] = tie(dst_c1, dst_c2, dst_c3, new_alpha);
        } else {
#if INFO_DEBUG
                ++countTwo;
#endif
                dataDest[indexes] = tie(src_c1, src_c2, src_c3, new_alpha);
        }
    }

    template <bool haveMask, Vc::Implementation _impl>
    static ALWAYS_INLINE void compositeOnePixelScalar(const quint8 *src, quint8 *dst, const quint8 *mask, float opacity, const OptionalParams &oparams)
    {
        using namespace Arithmetic;
        const qint32 alpha_pos = 3;

        const channels_type *s = reinterpret_cast<const channels_type*>(src);
        channels_type *d = reinterpret_cast<channels_type*>(dst);

        float srcAlpha = s[alpha_pos];
        srcAlpha *= opacity;

        if (haveMask) {
            const float uint8Rec1 = 1.0 / 255;
            srcAlpha *= float(*mask) * uint8Rec1;
        }

#if INFO_DEBUG
        static int xx = 0;
        bool display = xx > 45 && xx < 50;
        if (display) {
            qInfo() << "O" << s[alpha_pos] << srcAlpha << haveMask << opacity;
        }
#endif

        if (srcAlpha != NATIVE_OPACITY_TRANSPARENT) {

            float dstAlpha = d[alpha_pos];
            float srcBlendNorm;

            if (dstAlpha == NATIVE_OPACITY_OPAQUE) {
                srcBlendNorm = srcAlpha;
            } else if (dstAlpha == NATIVE_OPACITY_TRANSPARENT) {
                dstAlpha = srcAlpha;
                srcBlendNorm = NATIVE_OPACITY_OPAQUE;

                if (!allChannelsFlag) {
                    KoStreamedMathFunctions::clearPixel<16>(dst);
                }
            } else {
                dstAlpha += (NATIVE_OPACITY_OPAQUE - dstAlpha) * srcAlpha;
                srcBlendNorm = srcAlpha / dstAlpha;
            }

#if INFO_DEBUG
            if (display) {
                qInfo() << "params" << srcBlendNorm << allChannelsFlag << alphaLocked << dstAlpha << haveMask;
            }
#endif
            if(allChannelsFlag) {
                if (srcBlendNorm == NATIVE_OPACITY_OPAQUE) {
                    if (!alphaLocked) {
                        KoStreamedMathFunctions::copyPixel<16>(src, dst);
                    } else {
                        d[0] = s[0];
                        d[1] = s[1];
                        d[2] = s[2];
                    }
                } else if (srcBlendNorm != 0.0){
#if INFO_DEBUG
                    if (display) {
                        qInfo() << "calc" << s[0] << d[0] << srcBlendNorm * (s[0] - d[0]) + d[0] << s[0] - d[0] << srcBlendNorm * (s[0] - d[0]) << srcBlendNorm;
                    }
#endif
                    d[0] = srcBlendNorm * (s[0] - d[0]) + d[0];
                    d[1] = srcBlendNorm * (s[1] - d[1]) + d[1];
                    d[2] = srcBlendNorm * (s[2] - d[2]) + d[2];
                }
            } else {
                const QBitArray &channelFlags = oparams.channelFlags;

                if (srcBlendNorm == NATIVE_OPACITY_OPAQUE) {
                    if(channelFlags.at(0)) d[0] = s[0];
                    if(channelFlags.at(1)) d[1] = s[1];
                    if(channelFlags.at(2)) d[2] = s[2];
                } else if (srcBlendNorm != 0.0) {
                    if(channelFlags.at(0)) d[0] = srcBlendNorm * (s[0] - d[0]) + d[0];
                    if(channelFlags.at(1)) d[1] = srcBlendNorm * (s[1] - d[1]) + d[1];
                    if(channelFlags.at(2)) d[2] = srcBlendNorm * (s[2] - d[2]) + d[2];
                }
            }

            if (!alphaLocked) {
                d[alpha_pos] = dstAlpha;
            }
#if INFO_DEBUG
            if (display) {
                qInfo() << "result" << d[0] << d[1] << d[2] << d[3];
            }
            ++xx;
#endif
        }
    }
};

/**
 * An optimized version of a composite op for the use in 16 byte
 * colorspaces with alpha channel placed at the last byte of
 * the pixel: C1_C2_C3_A.
 */
template<Vc::Implementation _impl>
class KoOptimizedCompositeOpOver128 : public KoCompositeOp
{
public:
    KoOptimizedCompositeOpOver128(const KoColorSpace* cs)
        : KoCompositeOp(cs, COMPOSITE_OVER, i18n("Normal"), KoCompositeOp::categoryMix()) {}

    using KoCompositeOp::composite;

    virtual void composite(const KoCompositeOp::ParameterInfo& params) const
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

            KoStreamedMath<_impl>::template genericComposite128<haveMask, false, OverCompositor128<float, quint32, false, true> >(params);
        } else {
            const bool allChannelsFlag =
                params.channelFlags.at(0) &&
                params.channelFlags.at(1) &&
                params.channelFlags.at(2);

            const bool alphaLocked =
                !params.channelFlags.at(3);

            if (allChannelsFlag && alphaLocked) {
                KoStreamedMath<_impl>::template genericComposite128_novector<haveMask, false, OverCompositor128<float, quint32, true, true> >(params);
            } else if (!allChannelsFlag && !alphaLocked) {
                KoStreamedMath<_impl>::template genericComposite128_novector<haveMask, false, OverCompositor128<float, quint32, false, false> >(params);
            } else /*if (!allChannelsFlag && alphaLocked) */{
                KoStreamedMath<_impl>::template genericComposite128_novector<haveMask, false, OverCompositor128<float, quint32, true, false> >(params);
            }
        }
    }
};

#endif // KOOPTIMIZEDCOMPOSITEOPOVER128_H_
