/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KoOptimizedRgbPixelDataScalerU8ToU16_H
#define KoOptimizedRgbPixelDataScalerU8ToU16_H

#include "KoOptimizedRgbPixelDataScalerU8ToU16Base.h"

#include "KoVcMultiArchBuildSupport.h"
#include "kis_debug.h"

#if defined(__i386__) || defined(__x86_64__)
#include <immintrin.h>
#endif


template<Vc::Implementation _impl>
class KoOptimizedRgbPixelDataScalerU8ToU16 : public KoOptimizedRgbPixelDataScalerU8ToU16Base
{
    void convertU8ToU16(const quint8 *src, int srcRowStride,
                        quint8 *dst, int dstRowStride,
                        int numRows, int numColumns) const override
    {
        const int numColorChannels = 4 * numColumns;

#if defined __AVX2__
        const int channelsPerAvx2Block = 16;
        const int channelsPerSse2Block = 8;
        const int avx2Block = numColorChannels / channelsPerAvx2Block;
        const int rest = numColorChannels % channelsPerAvx2Block;
        const int sse2Block = rest / channelsPerSse2Block;
        const int scalarBlock = rest % channelsPerSse2Block;
#elif defined __SSE4_1__
        const int channelsPerSse2Block = 8;
        const int avx2Block = 0;
        const int sse2Block = numColorChannels / channelsPerSse2Block;
        const int scalarBlock = numColorChannels % channelsPerSse2Block;
#else
        const int avx2Block = 0;
        const int sse2Block = 0;
        const int scalarBlock = numColorChannels;
#endif

        //qWarning() << ppVar(avx2Block) << ppVar(sse2Block);

        for (int row = 0; row < numRows; row++) {

            const quint8 *srcPtr = src;
            quint16 *dstPtr = reinterpret_cast<quint16*>(dst);

#ifdef __AVX2__
            for (int i = 0; i < avx2Block; i++) {
                __m128i x = _mm_loadu_si128(reinterpret_cast<const __m128i*>(srcPtr));

                __m256i y = _mm256_cvtepu8_epi16(x);
                __m256i y_shifted = _mm256_slli_epi16(y, 8);
                y = _mm256_or_si256(y, y_shifted);

                _mm256_storeu_si256(reinterpret_cast<__m256i*>(dstPtr), y);

                srcPtr += channelsPerAvx2Block;
                dstPtr += channelsPerAvx2Block;
            }
#else
            Q_UNUSED(avx2Block);
#endif

#ifdef __SSE4_1__
            for (int i = 0; i < sse2Block; i++) {
                __m128i x = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(srcPtr));

                __m128i y = _mm_cvtepu8_epi16(x);
                __m128i y_shifted = _mm_slli_epi16(y, 8);
                y = _mm_or_si128(y, y_shifted);

                _mm_storeu_si128(reinterpret_cast<__m128i*>(dstPtr), y);

                srcPtr += channelsPerSse2Block;
                dstPtr += channelsPerSse2Block;
            }
#else
            Q_UNUSED(sse2Block);
#endif

            for (int i = 0; i < scalarBlock; i++) {
                const quint16 value = *srcPtr;

                *dstPtr = value | (value << 8);

                srcPtr++;
                dstPtr++;
            }


            src += srcRowStride;
            dst += dstRowStride;
        }
    }

    void convertU16ToU8(const quint8 *src, int srcRowStride,
                        quint8 *dst, int dstRowStride,
                        int numRows, int numColumns) const override
    {
        const int numColorChannels = 4 * numColumns;

#if defined __AVX2__
        const int channelsPerAvx2Block = 32;
        const int channelsPerSse2Block = 16;
        const int avx2Block = numColorChannels / channelsPerAvx2Block;
        const int rest = numColorChannels % channelsPerAvx2Block;
        const int sse2Block = rest / channelsPerSse2Block;
        const int scalarBlock = rest % channelsPerSse2Block;

        __m256i offset1 = _mm256_set1_epi16(128);
        __m128i offset2 = _mm_set1_epi16(128);

#elif defined __SSE2__
        const int channelsPerSse2Block = 16;
        const int avx2Block = 0;
        const int sse2Block = numColorChannels / channelsPerSse2Block;
        const int scalarBlock = numColorChannels % channelsPerSse2Block;

        __m128i offset2 = _mm_set1_epi16(128);
#else
        const int avx2Block = 0;
        const int sse2Block = 0;
        const int scalarBlock = numColorChannels;
#endif

        //qWarning() << ppVar(avx2Block) << ppVar(sse2Block);

        for (int row = 0; row < numRows; row++) {

            const quint16 *srcPtr = reinterpret_cast<const quint16*>(src);
            quint8 *dstPtr = dst;

#ifdef __AVX2__
            for (int i = 0; i < avx2Block; i++) {

                __m256i x1 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(srcPtr));
                __m256i x2 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(srcPtr + 16));

                __m256i x1_shifted = _mm256_srli_epi16(x1, 8);
                __m256i x2_shifted = _mm256_srli_epi16(x2, 8);

                x1 = _mm256_sub_epi16(x1, x1_shifted);
                x1 = _mm256_add_epi16(x1, offset1);
                x1 = _mm256_srli_epi16(x1, 8);

                x2 = _mm256_sub_epi16(x2, x2_shifted);
                x2 = _mm256_add_epi16(x2, offset1);
                x2 = _mm256_srli_epi16(x2, 8);

                x1 = _mm256_packus_epi16(x1, x2);

                // Packing in AVX2 does a bit different thing, not
                // what you expect that after seeing a SSE2 version :)
                // Therefore we need to permute the result...
                x1 = _mm256_permute4x64_epi64(x1, 0xd8);

                _mm256_storeu_si256(reinterpret_cast<__m256i*>(dstPtr), x1);

                srcPtr += channelsPerAvx2Block;
                dstPtr += channelsPerAvx2Block;
            }
#else
            Q_UNUSED(avx2Block);
#endif

#ifdef __SSE2__
            for (int i = 0; i < sse2Block; i++) {
                __m128i x1 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(srcPtr));
                __m128i x2 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(srcPtr + 8));

                __m128i x1_shifted = _mm_srli_epi16(x1, 8);
                __m128i x2_shifted = _mm_srli_epi16(x2, 8);

                x1 = _mm_sub_epi16(x1, x1_shifted);
                x1 = _mm_add_epi16(x1, offset2);
                x1 = _mm_srli_epi16(x1, 8);

                x2 = _mm_sub_epi16(x2, x2_shifted);
                x2 = _mm_add_epi16(x2, offset2);
                x2 = _mm_srli_epi16(x2, 8);

                x1 = _mm_packus_epi16(x1, x2);

                _mm_storeu_si128(reinterpret_cast<__m128i*>(dstPtr), x1);

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

#endif // KoOptimizedRgbPixelDataScalerU8ToU16_H
