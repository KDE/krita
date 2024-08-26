/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KoOptimizedPixelDataScalerU8ToU16_H
#define KoOptimizedPixelDataScalerU8ToU16_H

#include "KoOptimizedPixelDataScalerU8ToU16Base.h"

#include "KoMultiArchBuildSupport.h"
#include "kis_debug.h"

#include <xsimd_extensions/xsimd.hpp>

template<typename _impl = xsimd::current_arch>
class KoOptimizedPixelDataScalerU8ToU16 : public KoOptimizedPixelDataScalerU8ToU16Base
{
public:
    KoOptimizedPixelDataScalerU8ToU16(int channelsPerPixel)
        : KoOptimizedPixelDataScalerU8ToU16Base(channelsPerPixel)
    {
    }

    void convertU8ToU16(const quint8 *src, int srcRowStride, quint8 *dst, int dstRowStride, int numRows, int numColumns) const override
    {
        const int numColorChannels = m_channelsPerPixel * numColumns;

#if defined(HAVE_XSIMD) && XSIMD_WITH_AVX2
        using uint16_avx_v = xsimd::batch<uint16_t, xsimd::avx2>;
        using uint16_v = xsimd::batch<uint16_t, xsimd::sse4_1>;
        using uint8_v = xsimd::batch<uint8_t, xsimd::sse4_1>;

        const int channelsPerAvx2Block = 16;
        const int channelsPerSse2Block = 8;
        const int avx2Block = numColorChannels / channelsPerAvx2Block;
        const int rest = numColorChannels % channelsPerAvx2Block;
        const int sse2Block = rest / channelsPerSse2Block;
        const int scalarBlock = rest % channelsPerSse2Block;
#elif defined(HAVE_XSIMD) && (XSIMD_WITH_SSE4_1 || XSIMD_WITH_NEON || XSIMD_WITH_NEON64)
#if XSIMD_WITH_SSE4_1
        using uint16_v = xsimd::batch<uint16_t, xsimd::sse4_1>;
        using uint8_v = xsimd::batch<uint8_t, xsimd::sse4_1>;
#elif XSIMD_WITH_NEON64
        using uint16_v = xsimd::batch<uint16_t, xsimd::neon64>;
        using uint8_v = xsimd::batch<uint8_t, xsimd::neon64>;
#else
        using uint16_v = xsimd::batch<uint16_t, xsimd::neon>;
        using uint8_v = xsimd::batch<uint8_t, xsimd::neon>;
#endif

        const int channelsPerSse2Block = 8;
        const int avx2Block = 0;
        const int sse2Block = numColorChannels / channelsPerSse2Block;
        const int scalarBlock = numColorChannels % channelsPerSse2Block;
#else
        const int avx2Block = 0;
        const int sse2Block = 0;
        const int scalarBlock = numColorChannels;
#endif

        // qWarning() << ppVar(avx2Block) << ppVar(sse2Block);

        for (int row = 0; row < numRows; row++) {
            const quint8 *srcPtr = src;
            auto *dstPtr = reinterpret_cast<quint16 *>(dst);

#if defined(HAVE_XSIMD) && XSIMD_WITH_AVX2
            for (int i = 0; i < avx2Block; i++) {
                const auto x = uint8_v::load_unaligned(srcPtr);

                uint16_avx_v y(_mm256_cvtepu8_epi16(x));
                const auto y_shifted = y << 8;
                y |= y_shifted;

                y.store_unaligned(
                    reinterpret_cast<typename uint16_avx_v::value_type *>(dstPtr));

                srcPtr += channelsPerAvx2Block;
                dstPtr += channelsPerAvx2Block;
            }
#else
            Q_UNUSED(avx2Block);
#endif

#if defined(HAVE_XSIMD) && (XSIMD_WITH_SSE4_1 || XSIMD_WITH_NEON || XSIMD_WITH_NEON64)
            for (int i = 0; i < sse2Block; i++) {
#if XSIMD_WITH_SSE4_1
                const uint8_v x(_mm_loadl_epi64(reinterpret_cast<const __m128i *>(srcPtr)));
#else
                const uint8_v x(vreinterpretq_u8_u32(vcombine_u32(
                    vld1_u32(reinterpret_cast<const uint32_t *>(srcPtr)),
                    vcreate_u32(0))));
#endif
#if XSIMD_WITH_SSE4_1
                uint16_v y(_mm_cvtepu8_epi16(x.data));
#else
                uint16_v y(vmovl_u8(vget_low_u8(x.data)));
#endif
                const auto y_shifted = y << 8;
                y |= y_shifted;

                y.store_unaligned(reinterpret_cast<typename uint16_v::value_type *>(dstPtr));

                srcPtr += channelsPerSse2Block;
                dstPtr += channelsPerSse2Block;
            }
#else
            Q_UNUSED(sse2Block);
#endif

            for (int i = 0; i < scalarBlock; i++) {
                const quint16 value = *srcPtr;

                *dstPtr = static_cast<quint16>(value | (value << 8));

                srcPtr++;
                dstPtr++;
            }

            src += srcRowStride;
            dst += dstRowStride;
        }
    }

    void convertU16ToU8(const quint8 *src, int srcRowStride, quint8 *dst, int dstRowStride, int numRows, int numColumns) const override
    {
        const int numColorChannels = m_channelsPerPixel * numColumns;

#if defined(HAVE_XSIMD) && XSIMD_WITH_AVX2
        using uint16_avx_v = xsimd::batch<uint16_t, xsimd::avx2>;
        using uint16_v = xsimd::batch<uint16_t, xsimd::sse4_1>;

        const int channelsPerAvx2Block = 32;
        const int channelsPerSse2Block = 16;
        const int avx2Block = numColorChannels / channelsPerAvx2Block;
        const int rest = numColorChannels % channelsPerAvx2Block;
        const int sse2Block = rest / channelsPerSse2Block;
        const int scalarBlock = rest % channelsPerSse2Block;

        const auto offset1 = uint16_avx_v(128);
        const auto offset2 = uint16_v(128);

#elif defined(HAVE_XSIMD) && XSIMD_WITH_SSE2 || XSIMD_WITH_NEON || XSIMD_WITH_NEON64
        // SSE2, unlike the previous function, is a perfectly valid option
        // while under generic.
#if XSIMD_WITH_SSE2
        using uint16_v = xsimd::batch<uint16_t, xsimd::sse2>;
#elif XSIMD_WITH_NEON64
        using uint16_v = xsimd::batch<uint16_t, xsimd::neon64>;
#else
        using uint16_v = xsimd::batch<uint16_t, xsimd::neon>;
#endif

        const int channelsPerSse2Block = 16;
        const int avx2Block = 0;
        const int sse2Block = numColorChannels / channelsPerSse2Block;
        const int scalarBlock = numColorChannels % channelsPerSse2Block;

        const auto offset2 = uint16_v(128);
#else
        const int avx2Block = 0;
        const int sse2Block = 0;
        const int scalarBlock = numColorChannels;
#endif

        // qWarning() << ppVar(avx2Block) << ppVar(sse2Block);

        for (int row = 0; row < numRows; row++) {
            const quint16 *srcPtr = reinterpret_cast<const quint16 *>(src);
            quint8 *dstPtr = dst;

#if defined(HAVE_XSIMD) && XSIMD_WITH_AVX2
            for (int i = 0; i < avx2Block; i++) {
                auto x1 = uint16_avx_v::load_unaligned(srcPtr);
                auto x2 = uint16_avx_v::load_unaligned(srcPtr + uint16_avx_v::size);

                const auto x1_shifted = x1 >> 8;
                const auto x2_shifted = x2 >> 8;

                x1 -= x1_shifted;
                x1 += offset1;
                x1 >>= 8;

                x2 -= x2_shifted;
                x2 += offset1;
                x2 >>= 8;

                x1.data = _mm256_packus_epi16(x1, x2);

                // Packing in AVX2 does a bit different thing, not
                // what you expect that after seeing a SSE2 version :)
                // Therefore we need to permute the result...
                x1.data = _mm256_permute4x64_epi64(x1, 0xd8);

                x1.store_unaligned(reinterpret_cast<typename uint16_v::value_type *>(dstPtr));

                srcPtr += channelsPerAvx2Block;
                dstPtr += channelsPerAvx2Block;
            }
#else
        Q_UNUSED(avx2Block);
#endif

#if defined(HAVE_XSIMD) && (XSIMD_WITH_SSE2 || XSIMD_WITH_NEON || XSIMD_WITH_NEON64)
            for (int i = 0; i < sse2Block; i++) {
                auto x1 = uint16_v::load_unaligned(srcPtr);
                auto x2 = uint16_v::load_unaligned(srcPtr + uint16_v::size);

                const uint16_v x1_shifted = x1 >> 8;
                const uint16_v x2_shifted = x2 >> 8;

                x1 -= x1_shifted;
                x1 += offset2;
                x1 >>= 8;

                x2 -= x2_shifted;
                x2 += offset2;
                x2 >>= 8;
#if XSIMD_WITH_SSE2
                x1.data = _mm_packus_epi16(x1, x2);
#else
                x1.data = vreinterpretq_u16_u8(vcombine_u8(vqmovun_s16(vreinterpretq_s16_u16(x1)), vqmovun_s16(vreinterpretq_s16_u16(x2))));
#endif
                x1.store_unaligned(reinterpret_cast<typename uint16_v::value_type *>(dstPtr));
                srcPtr += channelsPerSse2Block;
                dstPtr += channelsPerSse2Block;
            }
#else
        Q_UNUSED(sse2Block);
#endif

            for (int i = 0; i < scalarBlock; i++) {
                const quint16 value = *srcPtr;

                *dstPtr = (value - (value >> 8) + 128) >> 8;

                srcPtr++;
                dstPtr++;
            }

            src += srcRowStride;
            dst += dstRowStride;
        }
    }
};

#endif // KoOptimizedPixelDataScalerU8ToU16_H
