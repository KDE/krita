/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOOPTIMIZEDCOMPOSITEOPCOPY128_H_
#define KOOPTIMIZEDCOMPOSITEOPCOPY128_H_

#include "KoCompositeOpBase.h"
#include "KoCompositeOpRegistry.h"
#include "KoStreamedMath.h"

#define NATIVE_OPACITY_OPAQUE KoColorSpaceMathsTraits<channels_type>::unitValue
#define NATIVE_OPACITY_TRANSPARENT KoColorSpaceMathsTraits<channels_type>::zeroValue

template<typename channels_type, bool alphaLocked, bool allChannelsFlag>
struct CopyCompositor128 {
    struct ParamsWrapper {
        ParamsWrapper(const KoCompositeOp::ParameterInfo& params)
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

    template<bool haveMask, bool src_aligned, Vc::Implementation _impl>
    static ALWAYS_INLINE void compositeVector(const quint8 *src, quint8 *dst, const quint8 *mask, float opacity, const ParamsWrapper &oparams)
    {
        Q_UNUSED(oparams);

        Vc::float_v src_alpha;
        Vc::float_v src_c1;
        Vc::float_v src_c2;
        Vc::float_v src_c3;

        PixelWrapper<channels_type, _impl> dataWrapper;
        dataWrapper.read(const_cast<quint8*>(src), src_c1, src_c2, src_c3, src_alpha);

        Vc::float_v opacity_norm_vec(opacity);

        if (haveMask) {
            const Vc::float_v uint8MaxRec1(1.0f / 255.0f);
            Vc::float_v mask_vec = KoStreamedMath<_impl>::fetch_mask_8(mask);
            opacity_norm_vec *= mask_vec * uint8MaxRec1;
        }

        const Vc::float_v zeroValue(0.0f);
        const Vc::float_v oneValue(1.0f);
        // The source cannot change the colors in the destination,
        // since its fully transparent
        if ((opacity_norm_vec == zeroValue).isFull()) {
            // noop
        } else if ((opacity_norm_vec == oneValue).isFull()) {
            if ((src_alpha == zeroValue).isFull()) {
                dataWrapper.clearPixels(dst);
            } else {
                dataWrapper.copyPixels(src, dst);
            }

        } else {
            Vc::float_v dst_alpha;
            Vc::float_v dst_c1;
            Vc::float_v dst_c2;
            Vc::float_v dst_c3;

            dataWrapper.read(dst, dst_c1, dst_c2, dst_c3, dst_alpha);

            Vc::float_v newAlpha = dst_alpha + opacity_norm_vec * (src_alpha - dst_alpha);

            if ((newAlpha == zeroValue).isFull()) {
                dataWrapper.clearPixels(dst);
            } else {
                src_c1 *= src_alpha;
                src_c2 *= src_alpha;
                src_c3 *= src_alpha;

                dst_c1 *= dst_alpha;
                dst_c2 *= dst_alpha;
                dst_c3 *= dst_alpha;

                dst_c1 += opacity_norm_vec * (src_c1 - dst_c1);
                dst_c2 += opacity_norm_vec * (src_c2 - dst_c2);
                dst_c3 += opacity_norm_vec * (src_c3 - dst_c3);

                if (!(newAlpha == oneValue).isFull()) {
                    /// This division by newAlpha may be unsafe in case
                    /// **some** elements of newAlpha are null. We don't
                    /// care, because:
                    ///
                    /// 1) the value will be clamped by Vc::min a bit later;
                    ///
                    /// 2) even if it doesn't, the new alpha will be null,
                    ///    so the state of the color channels is undefined

                    dst_c1 /= newAlpha;
                    dst_c2 /= newAlpha;
                    dst_c3 /= newAlpha;

                    Vc::float_v unitValue(KoColorSpaceMathsTraits<channels_type>::unitValue);

                    dst_c1 = Vc::min(dst_c1, unitValue);
                    dst_c2 = Vc::min(dst_c2, unitValue);
                    dst_c3 = Vc::min(dst_c3, unitValue);
                }

                dataWrapper.write(dst, dst_c1, dst_c2, dst_c3, newAlpha);
            }
        }
    }

    template <bool haveMask, Vc::Implementation _impl>
    static ALWAYS_INLINE void compositeOnePixelScalar(const quint8 *src, quint8 *dst, const quint8 *mask, float opacity, const ParamsWrapper &oparams)
    {
        using namespace Arithmetic;
        const qint32 alpha_pos = 3;

        const channels_type *s = reinterpret_cast<const channels_type*>(src);
        channels_type *d = reinterpret_cast<channels_type*>(dst);

        const channels_type nativeOriginalSrcAlpha = s[alpha_pos];

        // we shouldn't leak undefined color value from under the locked zero alpha
        // into our destination
        if (alphaLocked &&
            nativeOriginalSrcAlpha == KoColorSpaceMathsTraits<channels_type>::zeroValue) {

            return;
        }

        float srcAlpha = nativeOriginalSrcAlpha;
        PixelWrapper<channels_type, _impl>::normalizeAlpha(srcAlpha);

        if (haveMask) {
            const float uint8Rec1 = 1.0f / 255.0f;
            opacity *= float(*mask) * uint8Rec1;
        }

        if (opacity == 0.0f) {
            // noop
        } else if (opacity == 1.0f) {
            if (allChannelsFlag && !alphaLocked) {
                if (srcAlpha == 0.0f) {
                    KoStreamedMathFunctions::clearPixel<sizeof(Pixel)>(dst);
                } else {
                    KoStreamedMathFunctions::copyPixel<sizeof(Pixel)>(src, dst);
                }
            } else {
                if (d[alpha_pos] != KoColorSpaceMathsTraits<channels_type>::zeroValue ||
                    alphaLocked) {

                    const QBitArray &channelFlags = oparams.channelFlags;
                    if (channelFlags.at(0)) d[0] = s[0];
                    if (channelFlags.at(1)) d[1] = s[1];
                    if (channelFlags.at(2)) d[2] = s[2];
                    if (!alphaLocked) d[3] = s[3];

                } else {
                    // Precondition: d[alpha_pos] == 0 && !alphaLocked

                    const QBitArray &channelFlags = oparams.channelFlags;
                    d[0] = channelFlags.at(0) ? s[0] : KoColorSpaceMathsTraits<channels_type>::zeroValue;
                    d[1] = channelFlags.at(1) ? s[1] : KoColorSpaceMathsTraits<channels_type>::zeroValue;
                    d[2] = channelFlags.at(2) ? s[2] : KoColorSpaceMathsTraits<channels_type>::zeroValue;
                    d[3] = s[3];
                }
            }
        } else {
            float dstAlpha = d[alpha_pos];
            PixelWrapper<channels_type, _impl>::normalizeAlpha(dstAlpha);

            float newAlpha = dstAlpha + opacity * (srcAlpha - dstAlpha);

            if (newAlpha == 0.0f) {
                if ((allChannelsFlag && !alphaLocked) || dstAlpha == 0.0f) {
                    KoStreamedMathFunctions::clearPixel<sizeof(Pixel)>(dst);
                } else {
                    const QBitArray &channelFlags = oparams.channelFlags;
                    if (channelFlags.at(0)) d[0] = KoColorSpaceMathsTraits<channels_type>::zeroValue;
                    if (channelFlags.at(1)) d[1] = KoColorSpaceMathsTraits<channels_type>::zeroValue;
                    if (channelFlags.at(2)) d[2] = KoColorSpaceMathsTraits<channels_type>::zeroValue;
                    if (!alphaLocked) d[3] = KoColorSpaceMathsTraits<channels_type>::zeroValue;
                }
            } else {
                float src_c1 = s[0];
                float src_c2 = s[1];
                float src_c3 = s[2];

                float dst_c1 = d[0];
                float dst_c2 = d[1];
                float dst_c3 = d[2];

                src_c1 *= srcAlpha;
                src_c2 *= srcAlpha;
                src_c3 *= srcAlpha;

                dst_c1 *= dstAlpha;
                dst_c2 *= dstAlpha;
                dst_c3 *= dstAlpha;

                dst_c1 += opacity * (src_c1 - dst_c1);
                dst_c2 += opacity * (src_c2 - dst_c2);
                dst_c3 += opacity * (src_c3 - dst_c3);

                if (newAlpha != 1.0f) {
                    dst_c1 /= newAlpha;
                    dst_c2 /= newAlpha;
                    dst_c3 /= newAlpha;

                    const float unitValue = KoColorSpaceMathsTraits<channels_type>::unitValue;

                    dst_c1 = std::min(dst_c1, unitValue);
                    dst_c2 = std::min(dst_c2, unitValue);
                    dst_c3 = std::min(dst_c3, unitValue);
                }

                PixelWrapper<channels_type, _impl>::roundFloatToUint(dst_c1);
                PixelWrapper<channels_type, _impl>::roundFloatToUint(dst_c2);
                PixelWrapper<channels_type, _impl>::roundFloatToUint(dst_c3);

                if (allChannelsFlag) {
                    d[0] = dst_c1;
                    d[1] = dst_c2;
                    d[2] = dst_c3;
                } else {
                    if (dstAlpha != 0.0f || alphaLocked) {
                        const QBitArray &channelFlags = oparams.channelFlags;
                        if (channelFlags.at(0)) d[0] = dst_c1;
                        if (channelFlags.at(1)) d[1] = dst_c2;
                        if (channelFlags.at(2)) d[2] = dst_c3;
                    } else {
                        // Precondition: dstAlpha == 0 && !alphaLocked
                        const QBitArray &channelFlags = oparams.channelFlags;
                        d[0] = channelFlags.at(0) ? dst_c1 : KoColorSpaceMathsTraits<channels_type>::zeroValue;
                        d[1] = channelFlags.at(1) ? dst_c2 : KoColorSpaceMathsTraits<channels_type>::zeroValue;
                        d[2] = channelFlags.at(2) ? dst_c3 : KoColorSpaceMathsTraits<channels_type>::zeroValue;
                    }
                }

                if (!alphaLocked) {
                    PixelWrapper<channels_type, _impl>::denormalizeAlpha(newAlpha);
                    d[alpha_pos] = PixelWrapper<channels_type, _impl>::roundFloatToUint(newAlpha);
                }
            }
        }
    }
};

/**
 * An optimized version of a composite op for the use in 16 byte
 * colorspaces with alpha channel placed at the last byte of
 * the pixel: C1_C2_C3_A.
 */
template<Vc::Implementation _impl>
class KoOptimizedCompositeOpCopy128 : public KoCompositeOp
{
public:
    KoOptimizedCompositeOpCopy128(const KoColorSpace* cs)
        : KoCompositeOp(cs, COMPOSITE_COPY, i18n("Normal"), KoCompositeOp::categoryMix()) {}

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

            KoStreamedMath<_impl>::template genericComposite128<haveMask, false, CopyCompositor128<float, false, true> >(params);
        } else {
            const bool allChannelsFlag =
                params.channelFlags.at(0) &&
                params.channelFlags.at(1) &&
                params.channelFlags.at(2);

            const bool alphaLocked =
                !params.channelFlags.at(3);

            if (allChannelsFlag && alphaLocked) {
                KoStreamedMath<_impl>::template genericComposite128_novector<haveMask, false, CopyCompositor128<float, true, true> >(params);
            } else if (!allChannelsFlag && !alphaLocked) {
                KoStreamedMath<_impl>::template genericComposite128_novector<haveMask, false, CopyCompositor128<float, false, false> >(params);
            } else /*if (!allChannelsFlag && alphaLocked) */{
                KoStreamedMath<_impl>::template genericComposite128_novector<haveMask, false, CopyCompositor128<float, true, false> >(params);
            }
        }
    }
};

template<Vc::Implementation _impl>
class KoOptimizedCompositeOpCopyU64 : public KoCompositeOp
{
public:
    KoOptimizedCompositeOpCopyU64(const KoColorSpace* cs)
        : KoCompositeOp(cs, COMPOSITE_COPY, i18n("Normal"), KoCompositeOp::categoryMix()) {}

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

            KoStreamedMath<_impl>::template genericComposite64<haveMask, false, CopyCompositor128<quint16, false, true> >(params);
        } else {
            const bool allChannelsFlag =
                params.channelFlags.at(0) &&
                params.channelFlags.at(1) &&
                params.channelFlags.at(2);

            const bool alphaLocked =
                !params.channelFlags.at(3);

            if (allChannelsFlag && alphaLocked) {
                KoStreamedMath<_impl>::template genericComposite64_novector<haveMask, false, CopyCompositor128<quint16, true, true> >(params);
            } else if (!allChannelsFlag && !alphaLocked) {
                KoStreamedMath<_impl>::template genericComposite64_novector<haveMask, false, CopyCompositor128<quint16, false, false> >(params);
            } else /*if (!allChannelsFlag && alphaLocked) */{
                KoStreamedMath<_impl>::template genericComposite64_novector<haveMask, false, CopyCompositor128<quint16, true, false> >(params);
            }
        }
    }
};


template<Vc::Implementation _impl>
class KoOptimizedCompositeOpCopy32 : public KoCompositeOp
{
public:
    KoOptimizedCompositeOpCopy32(const KoColorSpace* cs)
        : KoCompositeOp(cs, COMPOSITE_COPY, i18n("Normal"), KoCompositeOp::categoryMix()) {}

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

            KoStreamedMath<_impl>::template genericComposite32<haveMask, false, CopyCompositor128<quint8, false, true> >(params);
        } else {
            const bool allChannelsFlag =
                params.channelFlags.at(0) &&
                params.channelFlags.at(1) &&
                params.channelFlags.at(2);

            const bool alphaLocked =
                !params.channelFlags.at(3);

            if (allChannelsFlag && alphaLocked) {
                KoStreamedMath<_impl>::template genericComposite32_novector<haveMask, false, CopyCompositor128<quint8, true, true> >(params);
            } else if (!allChannelsFlag && !alphaLocked) {
                KoStreamedMath<_impl>::template genericComposite32_novector<haveMask, false, CopyCompositor128<quint8, false, false> >(params);
            } else /*if (!allChannelsFlag && alphaLocked) */{
                KoStreamedMath<_impl>::template genericComposite32_novector<haveMask, false, CopyCompositor128<quint8, true, false> >(params);
            }
        }
    }
};
#endif // KOOPTIMIZEDCOMPOSITEOPCOPY128_H_
