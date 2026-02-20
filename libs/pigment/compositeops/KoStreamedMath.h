/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2020 Mathias Wein <lynx.mw+kde@gmail.com>
 *  SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef __KOSTREAMED_MATH_H
#define __KOSTREAMED_MATH_H

#include <cstdint>
#include <cstring>
#include <iostream>
#include <type_traits>
#include <xsimd_extensions/xsimd.hpp>

#if XSIMD_VERSION_MAJOR < 10
#include <KoRgbaInterleavers.h>
#endif

#include <KoAlwaysInline.h>
#include <KoCompositeOp.h>
#include <KoColorSpaceMaths.h>

#define BLOCKDEBUG 0

template<typename _impl, typename result_type>
struct OptiRound {
    ALWAYS_INLINE static result_type roundScalar(const float value)
    {
#ifdef __SSE__
        // SSE/AVX instructions use round-to-even rounding rule so we
        // should reuse it when possible
        return _mm_cvtss_si32(_mm_set_ss(value));
#elif XSIMD_WITH_NEON64
        return vgetq_lane_s32(vcvtnq_s32_f32(vrndiq_f32(vdupq_n_f32(value))),
                              0);
#elif XSIMD_WITH_NEON
        /* origin:
         * https://github.com/DLTcollab/sse2neon/blob/cad518a93b326f0f644b7972d488d04eaa2b0475/sse2neon.h#L4028-L4047
         */
        //  Contributors to this work are:
        //   John W. Ratcliff <jratcliffscarab@gmail.com>
        //   Brandon Rowlett <browlett@nvidia.com>
        //   Ken Fast <kfast@gdeb.com>
        //   Eric van Beurden <evanbeurden@nvidia.com>
        //   Alexander Potylitsin <apotylitsin@nvidia.com>
        //   Hasindu Gamaarachchi <hasindu2008@gmail.com>
        //   Jim Huang <jserv@biilabs.io>
        //   Mark Cheng <marktwtn@biilabs.io>
        //   Malcolm James MacLeod <malcolm@gulden.com>
        //   Devin Hussey (easyaspi314) <husseydevin@gmail.com>
        //   Sebastian Pop <spop@amazon.com>
        //   Developer Ecosystem Engineering
        //   <DeveloperEcosystemEngineering@apple.com> Danila Kutenin
        //   <danilak@google.com> François Turban (JishinMaster)
        //   <francois.turban@gmail.com> Pei-Hsuan Hung <afcidk@gmail.com>
        //   Yang-Hao Yuan <yanghau@biilabs.io>
        //   Syoyo Fujita <syoyo@lighttransport.com>
        //   Brecht Van Lommel <brecht@blender.org>

        /*
         * sse2neon is freely redistributable under the MIT License.
         *
         * Permission is hereby granted, free of charge, to any person obtaining
         * a copy of this software and associated documentation files (the
         * "Software"), to deal in the Software without restriction, including
         * without limitation the rights to use, copy, modify, merge, publish,
         * distribute, sublicense, and/or sell copies of the Software, and to
         * permit persons to whom the Software is furnished to do so, subject to
         * the following conditions:
         *
         * The above copyright notice and this permission notice shall be
         * included in all copies or substantial portions of the Software.
         *
         * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
         * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
         * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
         * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
         * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
         * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
         * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
         * SOFTWARE.
         */
        const auto nearbyint_as_int = [](const float v) {
            const auto a = vdupq_n_f32(v);
            const auto signmask = vdupq_n_u32(0x80000000);
            const auto half =
                vbslq_f32(signmask, a, vdupq_n_f32(0.5f)); /* +/- 0.5 */
            const auto r_normal = vcvtq_s32_f32(
                vaddq_f32(a, half)); /* round to integer: [a + 0.5]*/
            const auto r_trunc =
                vcvtq_s32_f32(a); /* truncate to integer: [a] */
            const auto plusone = vreinterpretq_s32_u32(
                vshrq_n_u32(vreinterpretq_u32_s32(vnegq_s32(r_trunc)),
                            31)); /* 1 or 0 */
            const auto r_even =
                vbicq_s32(vaddq_s32(r_trunc, plusone),
                          vdupq_n_s32(1)); /* ([a] + {0,1}) & ~1 */
            const auto delta = vsubq_f32(
                a,
                vcvtq_f32_s32(r_trunc)); /* compute delta: delta = (a - [a]) */
            const auto is_delta_half =
                vceqq_f32(delta, half); /* delta == +/- 0.5 */
            return vbslq_s32(is_delta_half, r_even, r_normal);
        };
        return vgetq_lane_s32(nearbyint_as_int(value), 0);
#else
        return std::lroundf(value);
#endif
    }
};

#if !defined(XSIMD_NO_SUPPORTED_ARCHITECTURE)

template<typename _impl>
struct OptiDiv {
    using float_v = xsimd::batch<float, _impl>;

    ALWAYS_INLINE static float divScalar(const float &divident, const float &divisor)
    {
#ifdef __SSE__
        float result = NAN;

        __m128 x = _mm_set_ss(divisor);
        __m128 y = _mm_set_ss(divident);
        x = _mm_rcp_ss(x);
        x = _mm_mul_ss(x, y);

        _mm_store_ss(&result, x);
        return result;
#elif defined __ARM_NEON
        auto x = vdupq_n_f32(divisor);
        auto y = vdupq_n_f32(divident);
        x = vrecpeq_f32(x);
        x = vmulq_f32(x, y);

        return vgetq_lane_f32(x, 0);
#else
        return (1.f / divisor) * divident;
#endif
    }

    ALWAYS_INLINE static float_v divVector(const float_v &divident, const float_v &divisor)
    {
        return divident * xsimd::reciprocal(divisor);
    }
};

template<typename _impl>
struct KoStreamedMath {
    using int_v = xsimd::batch<int, _impl>;
    using uint_v = xsimd::batch<unsigned int, _impl>;
    using float_v = xsimd::batch<float, _impl>;

    static_assert(int_v::size == uint_v::size, "the selected architecture does not guarantee vector size equality!");
    static_assert(uint_v::size == float_v::size, "the selected architecture does not guarantee vector size equality!");

    /**
     * Composes src into dst without using vector instructions
     */
    template<bool useMask, bool useFlow, class Compositor, int pixelSize>
    static void genericComposite_novector(const KoCompositeOp::ParameterInfo &params)
    {
        const qint32 linearInc = pixelSize;
        qint32 srcLinearInc = params.srcRowStride ? pixelSize : 0;

        quint8 *dstRowStart = params.dstRowStart;
        const quint8 *maskRowStart = params.maskRowStart;
        const quint8 *srcRowStart = params.srcRowStart;
        typename Compositor::ParamsWrapper paramsWrapper(params);

        for (qint32 r = params.rows; r > 0; --r) {
            const quint8 *mask = maskRowStart;
            const quint8 *src = srcRowStart;
            quint8 *dst = dstRowStart;

            int blockRest = params.cols;

            for (int i = 0; i < blockRest; i++) {
                Compositor::template compositeOnePixelScalar<useMask, _impl>(src,
                                                                             dst,
                                                                             mask,
                                                                             params.opacity,
                                                                             paramsWrapper);
                src += srcLinearInc;
                dst += linearInc;

                if (useMask) {
                    mask++;
                }
            }

            srcRowStart += params.srcRowStride;
            dstRowStart += params.dstRowStride;

            if (useMask) {
                maskRowStart += params.maskRowStride;
            }
        }
    }

    template<bool useMask, bool useFlow, class Compositor>
    static void genericComposite32_novector(const KoCompositeOp::ParameterInfo &params)
    {
        genericComposite_novector<useMask, useFlow, Compositor, 4>(params);
    }

    template<bool useMask, bool useFlow, class Compositor>
    static void genericComposite128_novector(const KoCompositeOp::ParameterInfo &params)
    {
        genericComposite_novector<useMask, useFlow, Compositor, 16>(params);
    }

    template<bool useMask, bool useFlow, class Compositor>
    static void genericComposite64_novector(const KoCompositeOp::ParameterInfo &params)
    {
        genericComposite_novector<useMask, useFlow, Compositor, 8>(params);
    }

    static inline quint8 round_float_to_u8(float x)
    {
        return OptiRound<_impl, quint8>::roundScalar(x);
    }

    static inline quint8 lerp_mixed_u8_float(quint8 a, quint8 b, float alpha)
    {
        return round_float_to_u8(float(b - a) * alpha + float(a));
    }

    /**
     * Get a vector containing first float_v::size values of mask.
     * Each source mask element is considered to be a 8-bit integer
     */
    static inline float_v fetch_mask_8(const quint8 *data)
    {
        return xsimd::batch_cast<float>(xsimd::load_and_extend<int_v>(data));
    }

    /**
     * Get an alpha values from float_v::size pixels 32-bit each
     * (4 channels, 8 bit per channel).  The alpha value is considered
     * to be stored in the most significant byte of the pixel
     *
     * \p aligned controls whether the \p data is fetched using aligned
     *            instruction or not.
     *            1) Fetching aligned data with unaligned instruction
     *               degrades performance.
     *            2) Fetching unaligned data with aligned instruction
     *               causes \#GP (General Protection Exception)
     */
    template<bool aligned>
    static inline float_v fetch_alpha_32(const void *data)
    {
        using U = typename std::conditional<aligned, xsimd::aligned_mode, xsimd::unaligned_mode>::type;
        const auto data_i = uint_v::load(static_cast<const typename uint_v::value_type *>(data), U{});
        return xsimd::to_float(xsimd::bitwise_cast_compat<int>(data_i >> 24));
    }

    /**
     * Get color values from float_v::size pixels 32-bit each
     * (4 channels, 8 bit per channel).  The color data is considered
     * to be stored in the 3 least significant bytes of the pixel.
     *
     * \p aligned controls whether the \p data is fetched using aligned
     *            instruction or not.
     *            1) Fetching aligned data with unaligned instruction
     *               degrades performance.
     *            2) Fetching unaligned data with aligned instruction
     *               causes \#GP (General Protection Exception)
     */
    template<bool aligned>
    static inline void fetch_colors_32(const void *data, float_v &c1, float_v &c2, float_v &c3)
    {
        using U = typename std::conditional<aligned, xsimd::aligned_mode, xsimd::unaligned_mode>::type;

        const auto data_i = uint_v::load(static_cast<const typename uint_v::value_type *>(data), U{});

        const uint_v mask(0xFF);

        c1 = xsimd::to_float(xsimd::bitwise_cast_compat<int>((data_i >> 16) & mask));
        c2 = xsimd::to_float(xsimd::bitwise_cast_compat<int>((data_i >> 8) & mask));
        c3 = xsimd::to_float(xsimd::bitwise_cast_compat<int>((data_i) & mask));
    }

    /**
     * Pack color and alpha values to float_v::size pixels 32-bit each
     * (4 channels, 8 bit per channel).  The color data is considered
     * to be stored in the 3 least significant bytes of the pixel, alpha -
     * in the most significant byte
     *
     * NOTE: \p data must be aligned pointer!
     */
    static inline void
    write_channels_32(void *data, const float_v alpha, const float_v c1, const float_v c2, const float_v c3)
    {
        const int_v mask(0xFF);

        const auto v1 = (xsimd::nearbyint_as_int(alpha)) << 24;
        const auto v2 = (xsimd::nearbyint_as_int(c1) & mask) << 16;
        const auto v3 = (xsimd::nearbyint_as_int(c2) & mask) << 8;
        const auto v4 = (xsimd::nearbyint_as_int(c3) & mask);
        xsimd::store_aligned(static_cast<typename int_v::value_type *>(data), (v1 | v2) | (v3 | v4));
    }

    static inline void
    write_channels_32_unaligned(void *data, const float_v alpha, const float_v c1, const float_v c2, const float_v c3)
    {
        const int_v mask(0xFF);

        const auto v1 = (xsimd::nearbyint_as_int(alpha)) << 24;
        const auto v2 = (xsimd::nearbyint_as_int(c1) & mask) << 16;
        const auto v3 = (xsimd::nearbyint_as_int(c2) & mask) << 8;
        const auto v4 = (xsimd::nearbyint_as_int(c3) & mask);
        xsimd::store_unaligned(static_cast<typename int_v::value_type *>(data), (v1 | v2) | (v3 | v4));
    }

    /**
     * Composes src pixels into dst pixels. Is optimized for 32-bit-per-pixel
     * colorspaces. Uses \p Compositor strategy parameter for doing actual
     * math of the composition
     */
    template<bool useMask, bool useFlow, class Compositor, int pixelSize>
    static void genericComposite(const KoCompositeOp::ParameterInfo &params)
    {
        const int vectorSize = static_cast<int>(float_v::size);
        const qint32 vectorInc = pixelSize * vectorSize;
        const qint32 linearInc = pixelSize;
        qint32 srcVectorInc = vectorInc;
        qint32 srcLinearInc = pixelSize;

        quint8 *dstRowStart = params.dstRowStart;
        const quint8 *maskRowStart = params.maskRowStart;
        const quint8 *srcRowStart = params.srcRowStart;
        typename Compositor::ParamsWrapper paramsWrapper(params);

        if (!params.srcRowStride) {
            if (pixelSize == 4) {
                auto *buf = reinterpret_cast<uint_v *>(xsimd::vector_aligned_malloc<typename uint_v::value_type>(vectorSize));
                *buf = uint_v(*(reinterpret_cast<const quint32 *>(srcRowStart)));
                srcRowStart = reinterpret_cast<quint8 *>(buf);
                srcLinearInc = 0;
                srcVectorInc = 0;
            } else {
                auto *buf = xsimd::vector_aligned_malloc<quint8>(vectorInc);
                quint8 *ptr = buf;

                for (size_t i = 0; i < vectorSize; i++) {
                    memcpy(ptr, params.srcRowStart, pixelSize);
                    ptr += pixelSize;
                }

                srcRowStart = buf;
                srcLinearInc = 0;
                srcVectorInc = 0;
            }
        }
#if BLOCKDEBUG
        int totalBlockAlign = 0;
        int totalBlockAlignedVector = 0;
        int totalBlockUnalignedVector = 0;
        int totalBlockRest = 0;
#endif

        for (qint32 r = params.rows; r > 0; --r) {
            // Hint: Mask is allowed to be unaligned
            const quint8 *mask = maskRowStart;

            const quint8 *src = srcRowStart;
            quint8 *dst = dstRowStart;

            const int pixelsAlignmentMask = vectorSize * sizeof(float) - 1;
            auto srcPtrValue = reinterpret_cast<uintptr_t>(src);
            auto dstPtrValue = reinterpret_cast<uintptr_t>(dst);
            uintptr_t srcAlignment = srcPtrValue & pixelsAlignmentMask;
            uintptr_t dstAlignment = dstPtrValue & pixelsAlignmentMask;

            // Uncomment if facing problems with alignment:
            // Q_ASSERT_X(!(dstAlignment & 3), "Compositing",
            //            "Pixel data must be aligned on pixels borders!");

            int blockAlign = params.cols;
            int blockAlignedVector = 0;
            int blockUnalignedVector = 0;
            int blockRest = 0;

            int *vectorBlock =
                srcAlignment == dstAlignment || !srcVectorInc ? &blockAlignedVector : &blockUnalignedVector;

            if (!dstAlignment) {
                blockAlign = 0;
                *vectorBlock = params.cols / vectorSize;
                blockRest = params.cols % vectorSize;
            } else if (params.cols > 2 * vectorSize) {
                blockAlign = (vectorInc - dstAlignment) / pixelSize;
                const int restCols = params.cols - blockAlign;
                if (restCols > 0) {
                    *vectorBlock = restCols / vectorSize;
                    blockRest = restCols % vectorSize;
                } else {
                    blockAlign = params.cols;
                    *vectorBlock = 0;
                    blockRest = 0;
                }
            }
#if BLOCKDEBUG
            totalBlockAlign += blockAlign;
            totalBlockAlignedVector += blockAlignedVector;
            totalBlockUnalignedVector += blockUnalignedVector;
            totalBlockRest += blockRest;
#endif

            for (int i = 0; i < blockAlign; i++) {
                Compositor::template compositeOnePixelScalar<useMask, _impl>(src,
                                                                             dst,
                                                                             mask,
                                                                             params.opacity,
                                                                             paramsWrapper);
                src += srcLinearInc;
                dst += linearInc;

                if (useMask) {
                    mask++;
                }
            }

            for (int i = 0; i < blockAlignedVector; i++) {
                Compositor::template compositeVector<useMask, true, _impl>(src,
                                                                           dst,
                                                                           mask,
                                                                           params.opacity,
                                                                           paramsWrapper);
                src += srcVectorInc;
                dst += vectorInc;

                if (useMask) {
                    mask += vectorSize;
                }
            }

            for (int i = 0; i < blockUnalignedVector; i++) {
                Compositor::template compositeVector<useMask, false, _impl>(src,
                                                                            dst,
                                                                            mask,
                                                                            params.opacity,
                                                                            paramsWrapper);
                src += srcVectorInc;
                dst += vectorInc;

                if (useMask) {
                    mask += vectorSize;
                }
            }

            for (int i = 0; i < blockRest; i++) {
                Compositor::template compositeOnePixelScalar<useMask, _impl>(src,
                                                                             dst,
                                                                             mask,
                                                                             params.opacity,
                                                                             paramsWrapper);
                src += srcLinearInc;
                dst += linearInc;

                if (useMask) {
                    mask++;
                }
            }

            srcRowStart += params.srcRowStride;
            dstRowStart += params.dstRowStride;

            if (useMask) {
                maskRowStart += params.maskRowStride;
            }
        }

#if BLOCKDEBUG
        dbgPigment << "I"
                   << "rows:" << params.rows << "\tpad(S):" << totalBlockAlign << "\tbav(V):" << totalBlockAlignedVector
                   << "\tbuv(V):" << totalBlockUnalignedVector << "\tres(S)"
                   << totalBlockRest; // << srcAlignment << dstAlignment;
#endif

        if (!params.srcRowStride) {
            xsimd::vector_aligned_free(srcRowStart);
        }
    }

    template<bool useMask, bool useFlow, class Compositor>
    static void genericComposite32(const KoCompositeOp::ParameterInfo &params)
    {
        genericComposite<useMask, useFlow, Compositor, 4>(params);
    }

    template<bool useMask, bool useFlow, class Compositor>
    static void genericComposite128(const KoCompositeOp::ParameterInfo &params)
    {
        genericComposite<useMask, useFlow, Compositor, 16>(params);
    }

    template<bool useMask, bool useFlow, class Compositor>
    static void genericComposite64(const KoCompositeOp::ParameterInfo &params)
    {
        genericComposite<useMask, useFlow, Compositor, 8>(params);
    }
};

template<typename channels_type, class _impl>
struct PixelStateRecoverHelper {
    using float_v = xsimd::batch<float, _impl>;
    using float_m = typename float_v::batch_bool_type;

    ALWAYS_INLINE
    PixelStateRecoverHelper(const float_v &c1, const float_v &c2, const float_v &c3)
    {
        Q_UNUSED(c1);
        Q_UNUSED(c2);
        Q_UNUSED(c3);
    }

    ALWAYS_INLINE
    void recoverPixels(const float_m &mask, float_v &c1, float_v &c2, float_v &c3) const {
        Q_UNUSED(mask);
        Q_UNUSED(c1);
        Q_UNUSED(c2);
        Q_UNUSED(c3);
    }
};

template<class _impl>
struct PixelStateRecoverHelper<float, _impl> {
    using float_v = xsimd::batch<float, _impl>;
    using float_m = typename float_v::batch_bool_type;

    ALWAYS_INLINE
    PixelStateRecoverHelper(const float_v &c1, const float_v &c2, const float_v &c3)
        : m_orig_c1(c1),
          m_orig_c2(c2),
          m_orig_c3(c3)
    {
    }

    ALWAYS_INLINE
    void recoverPixels(const float_m &mask, float_v &c1, float_v &c2, float_v &c3) const {
        if (xsimd::any(mask)) {
            c1 = xsimd::select(mask, m_orig_c1, c1);
            c2 = xsimd::select(mask, m_orig_c2, c2);
            c3 = xsimd::select(mask, m_orig_c3, c3);
        }
    }

private:
    const float_v m_orig_c1;
    const float_v m_orig_c2;
    const float_v m_orig_c3;
};

template<typename channels_type, class _impl>
struct PixelWrapper
{
};

template<class _impl>
struct PixelWrapper<quint16, _impl> {
    using int_v = xsimd::batch<int, _impl>;
    using uint_v = xsimd::batch<unsigned int, _impl>;
    using float_v = xsimd::batch<float, _impl>;

    static_assert(int_v::size == uint_v::size, "the selected architecture does not guarantee vector size equality!");
    static_assert(uint_v::size == float_v::size, "the selected architecture does not guarantee vector size equality!");

    ALWAYS_INLINE
    static quint16 lerpMixedUintFloat(quint16 a, quint16 b, float alpha)
    {
        return OptiRound<_impl, quint16>::roundScalar((float(b) - a) * alpha + float(a));
    }

    ALWAYS_INLINE
    static quint16 roundFloatToUint(float x)
    {
        return OptiRound<_impl, quint16>::roundScalar(x);
    }

    ALWAYS_INLINE
    static void normalizeAlpha(float &alpha)
    {
        const float uint16Rec1 = 1.0f / 65535.0f;
        alpha *= uint16Rec1;
    }

    ALWAYS_INLINE
    static void denormalizeAlpha(float &alpha)
    {
        const float uint16Max = 65535.0f;
        alpha *= uint16Max;
    }

    PixelWrapper()
        : mask(0xFFFF)
        , uint16Max(65535.0f)
        , uint16Rec1(1.0f / 65535.0f)
    {
    }

    ALWAYS_INLINE void read(const void *src, float_v &dst_c1, float_v &dst_c2, float_v &dst_c3, float_v &dst_alpha)
    {
        // struct PackedPixel {
        //    float rrgg;
        //    float bbaa;
        // }
#if XSIMD_VERSION_MAJOR < 10
        uint_v pixelsC1C2;
        uint_v pixelsC3Alpha;
        KoRgbaInterleavers<16>::deinterleave(src, pixelsC1C2, pixelsC3Alpha);
#else
        const auto *srcPtr = static_cast<const typename uint_v::value_type *>(src);
        const auto idx1 = xsimd::detail::make_sequence_as_batch<int_v>() * 2; // stride == 2
        const auto idx2 = idx1 + 1; // offset 1 == 2nd members

        const auto pixelsC1C2 = uint_v::gather(srcPtr, idx1);
        const auto pixelsC3Alpha = uint_v::gather(srcPtr, idx2);
#endif

        dst_c1 = xsimd::to_float(xsimd::bitwise_cast_compat<int>(pixelsC1C2 & mask)); // r
        dst_c2 = xsimd::to_float(xsimd::bitwise_cast_compat<int>((pixelsC1C2 >> 16) & mask)); // g
        dst_c3 = xsimd::to_float(xsimd::bitwise_cast_compat<int>((pixelsC3Alpha & mask))); // b
        dst_alpha = xsimd::to_float(xsimd::bitwise_cast_compat<int>((pixelsC3Alpha >> 16) & mask)); // a

        dst_alpha *= uint16Rec1;
    }

    ALWAYS_INLINE void write(void *dst, const float_v &c1, const float_v &c2, const float_v &c3, const float_v &a)
    {
        const auto alpha = a * uint16Max;

        const auto v1 = xsimd::bitwise_cast_compat<unsigned int>(xsimd::nearbyint_as_int(c1));
        const auto v2 = xsimd::bitwise_cast_compat<unsigned int>(xsimd::nearbyint_as_int(c2));
        const auto v3 = xsimd::bitwise_cast_compat<unsigned int>(xsimd::nearbyint_as_int(c3));
        const auto v4 = xsimd::bitwise_cast_compat<unsigned int>(xsimd::nearbyint_as_int(alpha));

        const auto c1c2 = ((v2 & mask) << 16) | (v1 & mask);
        const auto c3ca = ((v4 & mask) << 16) | (v3 & mask);

#if XSIMD_VERSION_MAJOR < 10
        KoRgbaInterleavers<16>::interleave(dst, c1c2, c3ca);
#else
        auto dstPtr = reinterpret_cast<typename int_v::value_type *>(dst);

        const auto idx1 = xsimd::detail::make_sequence_as_batch<int_v>() * 2;
        const auto idx2 = idx1 + 1;

        c1c2.scatter(dstPtr, idx1);
        c3ca.scatter(dstPtr, idx2);
#endif
    }

    ALWAYS_INLINE
    void clearPixels(quint8 *dataDst)
    {
        memset(dataDst, 0, float_v::size * sizeof(quint16) * 4);
    }

    ALWAYS_INLINE
    void copyPixels(const quint8 *dataSrc, quint8 *dataDst)
    {
        memcpy(dataDst, dataSrc, float_v::size * sizeof(quint16) * 4);
    }

    const uint_v mask;
    const float_v uint16Max;
    const float_v uint16Rec1;
};

template<typename _impl>
struct PixelWrapper<quint8, _impl> {
    using int_v = xsimd::batch<int, _impl>;
    using uint_v = xsimd::batch<unsigned int, _impl>;
    using float_v = xsimd::batch<float, _impl>;

    static_assert(int_v::size == uint_v::size, "the selected architecture does not guarantee vector size equality!");
    static_assert(uint_v::size == float_v::size, "the selected architecture does not guarantee vector size equality!");

    ALWAYS_INLINE
    static quint8 lerpMixedUintFloat(quint8 a, quint8 b, float alpha)
    {
        return KoStreamedMath<_impl>::lerp_mixed_u8_float(a, b, alpha);
    }

    ALWAYS_INLINE
    static quint8 roundFloatToUint(float x)
    {
        return KoStreamedMath<_impl>::round_float_to_u8(x);
    }

    ALWAYS_INLINE
    static void normalizeAlpha(float &alpha)
    {
        const float uint8Rec1 = 1.0f / 255.0f;
        alpha *= uint8Rec1;
    }

    ALWAYS_INLINE
    static void denormalizeAlpha(float &alpha)
    {
        const float uint8Max = 255.0f;
        alpha *= uint8Max;
    }

    PixelWrapper()
        : mask(quint32(0xFF))
        , uint8Max(255.0f)
        , uint8Rec1(1.0f / 255.0f)
    {
    }

    ALWAYS_INLINE void read(const void *src, float_v &dst_c1, float_v &dst_c2, float_v &dst_c3, float_v &dst_alpha)
    {
        dst_alpha = KoStreamedMath<_impl>::template fetch_alpha_32<false>(src);
        KoStreamedMath<_impl>::template fetch_colors_32<false>(src, dst_c1, dst_c2, dst_c3);

        dst_alpha *= uint8Rec1;
    }

    ALWAYS_INLINE
    void write(quint8 *dataDst, const float_v &c1, const float_v &c2, const float_v &c3, const float_v &a)
    {
        const auto alpha = a * uint8Max;

        KoStreamedMath<_impl>::write_channels_32_unaligned(dataDst, alpha, c1, c2, c3);
    }

    ALWAYS_INLINE
    void clearPixels(quint8 *dataDst)
    {
        memset(dataDst, 0, float_v::size * sizeof(quint8) * 4);
    }

    ALWAYS_INLINE
    void copyPixels(const quint8 *dataSrc, quint8 *dataDst)
    {
        memcpy(dataDst, dataSrc, float_v::size * sizeof(quint8) * 4);
    }

    const uint_v mask;
    const float_v uint8Max;
    const float_v uint8Rec1;
};

template<typename _impl>
struct PixelWrapper<float, _impl> {
    using int_v = xsimd::batch<int, _impl>;
    using uint_v = xsimd::batch<unsigned int, _impl>;
    using float_v = xsimd::batch<float, _impl>;

    static_assert(int_v::size == uint_v::size, "the selected architecture does not guarantee vector size equality!");
    static_assert(uint_v::size == float_v::size, "the selected architecture does not guarantee vector size equality!");

    struct Pixel {
        float red;
        float green;
        float blue;
        float alpha;
    };

    ALWAYS_INLINE
    static float lerpMixedUintFloat(float a, float b, float alpha)
    {
        return Arithmetic::lerp(a,b,alpha);
    }

    ALWAYS_INLINE
    static float roundFloatToUint(float x)
    {
        return x;
    }

    ALWAYS_INLINE
    static void normalizeAlpha(float &alpha)
    {
        Q_UNUSED(alpha);
    }

    ALWAYS_INLINE
    static void denormalizeAlpha(float &alpha)
    {
        Q_UNUSED(alpha);
    }

    PixelWrapper() = default;

    ALWAYS_INLINE void read(const void *src, float_v &dst_c1, float_v &dst_c2, float_v &dst_c3, float_v &dst_alpha)
    {
#if XSIMD_VERSION_MAJOR < 10
        KoRgbaInterleavers<32>::deinterleave(src, dst_c1, dst_c2, dst_c3, dst_alpha);
#else
        const auto srcPtr = reinterpret_cast<const typename float_v::value_type *>(src);
        const auto idx1 = xsimd::detail::make_sequence_as_batch<int_v>() * 4; // stride == 4
        const auto idx2 = idx1 + 1;
        const auto idx3 = idx1 + 2;
        const auto idx4 = idx1 + 3;

        dst_c1 = float_v::gather(srcPtr, idx1);
        dst_c2 = float_v::gather(srcPtr, idx2);
        dst_c3 = float_v::gather(srcPtr, idx3);
        dst_alpha = float_v::gather(srcPtr, idx4);
#endif
    }

    ALWAYS_INLINE void
    write(void *dst, const float_v &src_c1, const float_v &src_c2, const float_v &src_c3, const float_v &src_alpha)
    {
#if XSIMD_VERSION_MAJOR < 10
        KoRgbaInterleavers<32>::interleave(dst, src_c1, src_c2, src_c3, src_alpha);
#else
        auto dstPtr = reinterpret_cast<typename float_v::value_type *>(dst);

        const auto idx1 = xsimd::detail::make_sequence_as_batch<int_v>() * 4; // stride == 4
        const auto idx2 = idx1 + 1;
        const auto idx3 = idx1 + 2;
        const auto idx4 = idx1 + 3;

        src_c1.scatter(dstPtr, idx1);
        src_c2.scatter(dstPtr, idx2);
        src_c3.scatter(dstPtr, idx3);
        src_alpha.scatter(dstPtr, idx4);
#endif
    }

    ALWAYS_INLINE
    void clearPixels(quint8 *dataDst)
    {
        memset(dataDst, 0, float_v::size * sizeof(float) * 4);
    }

    ALWAYS_INLINE
    void copyPixels(const quint8 *dataSrc, quint8 *dataDst)
    {
        memcpy(dataDst, dataSrc, float_v::size * sizeof(float) * 4);
    }
};

#endif /* !defined(XSIMD_NO_SUPPORTED_ARCHITECTURE) */

namespace KoStreamedMathFunctions
{
template<int pixelSize>
ALWAYS_INLINE void clearPixel(quint8 *dst)
{
    std::memset(dst, 0, pixelSize);
}

template<int pixelSize>
ALWAYS_INLINE void copyPixel(const quint8 *src, quint8 *dst)
{
    std::memcpy(dst, src, pixelSize);
}
} // namespace KoStreamedMathFunctions

#endif /* __KOSTREAMED_MATH_H */
