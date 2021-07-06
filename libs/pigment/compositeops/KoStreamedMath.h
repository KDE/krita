/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2020 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef __KOSTREAMED_MATH_H
#define __KOSTREAMED_MATH_H

#if defined _MSC_VER
// Lets shut up the "possible loss of data" and "forcing value to bool 'true' or 'false'
#pragma warning ( push )
#pragma warning ( disable : 4146 ) // avx/detail.h
#pragma warning ( disable : 4244 )
#pragma warning ( disable : 4267 ) // interleavedmemory
#pragma warning ( disable : 4800 )
#endif
#include <Vc/Vc>
#include <Vc/IO>
#include <immintrin.h>
#if defined _MSC_VER
#pragma warning ( pop )
#endif

#include <cstdint>
#include <iostream>
#include <type_traits>
#include <KoAlwaysInline.h>
#include <KoCompositeOp.h>
#include <KoColorSpaceMaths.h>

#define BLOCKDEBUG 0

#if !defined _MSC_VER
#pragma GCC diagnostic ignored "-Wcast-align"
#endif


template<Vc::Implementation _impl>
struct OptiRound {
    ALWAYS_INLINE
    static float roundScalar(const float& value) {
#ifdef __SSE4_1__
        // SSE/AVX instructions use round-to-even rounding rule so we
        // should reuse it when possible

        float result;

        __m128 x = _mm_set_ss(value);
        __m128 y{};
        y = _mm_round_ss(y, x, 0x00);
        _mm_store_ss(&result, y);

        return result;
#else
        return value + 0.5f;
#endif

    }
};

template<Vc::Implementation _impl>
struct KoStreamedMath {

using int_v = Vc::SimdArray<int, Vc::float_v::size()>;
using uint_v = Vc::SimdArray<unsigned int, Vc::float_v::size()>;


/**
 * Composes src into dst without using vector instructions
 */
template<bool useMask, bool useFlow, class Compositor, int pixelSize>
    static void genericComposite_novector(const KoCompositeOp::ParameterInfo& params)
{
    using namespace Arithmetic;

    const qint32 linearInc = pixelSize;
    qint32 srcLinearInc = params.srcRowStride ? pixelSize : 0;

    quint8*       dstRowStart  = params.dstRowStart;
    const quint8* maskRowStart = params.maskRowStart;
    const quint8* srcRowStart  = params.srcRowStart;
    typename Compositor::ParamsWrapper paramsWrapper(params);

    for(qint32 r = params.rows; r > 0; --r) {
        const quint8 *mask = maskRowStart;
        const quint8 *src  = srcRowStart;
        quint8       *dst  = dstRowStart;

        int blockRest = params.cols;

        for(int i = 0; i < blockRest; i++) {
            Compositor::template compositeOnePixelScalar<useMask, _impl>(src, dst, mask, params.opacity, paramsWrapper);
            src += srcLinearInc;
            dst += linearInc;

            if (useMask) {
                mask++;
            }
        }

        srcRowStart  += params.srcRowStride;
        dstRowStart  += params.dstRowStride;

        if (useMask) {
            maskRowStart += params.maskRowStride;
        }
    }
}

template<bool useMask, bool useFlow, class Compositor>
    static void genericComposite32_novector(const KoCompositeOp::ParameterInfo& params)
{
    genericComposite_novector<useMask, useFlow, Compositor, 4>(params);
}

template<bool useMask, bool useFlow, class Compositor>
    static void genericComposite128_novector(const KoCompositeOp::ParameterInfo& params)
{
    genericComposite_novector<useMask, useFlow, Compositor, 16>(params);
}

template<bool useMask, bool useFlow, class Compositor>
    static void genericComposite64_novector(const KoCompositeOp::ParameterInfo& params)
{
    genericComposite_novector<useMask, useFlow, Compositor, 8>(params);
}

static inline quint8 round_float_to_u8(float x) {
    return OptiRound<_impl>::roundScalar(x);
}

static inline quint8 lerp_mixed_u8_float(quint8 a, quint8 b, float alpha) {
    return round_float_to_u8(qint16(b - a) * alpha + a);
}

/**
 * Round a vector of floats to the next corresponding integers.
 */
static inline int_v iRound(Vc::float_v::AsArg a)
{
#if defined(Vc_IMPL_AVX2)
    return Vc::simd_cast<int_v>(Vc::int_v(_mm256_cvtps_epi32(a.data())));
#elif defined(Vc_IMPL_AVX)
    /**
     * WARNING: Vc, on AVX, supplies 256-bit floating point vectors but stays
     * in SSE land for integers. It is not possible to cast between Vc::int_v
     * and uint_v without a custom SIMD type, because otherwise we lose the
     * XMM1 register. By using such a type:
     *    using avx_int_v = Vc::Vector<Vc::int_v::EntryType, Vc::float_v::abi>;
     *    static_assert(int_v::size() == avx_int_v::size(),
     *                  "uint_v must match the AVX placeholder");
     * and copying the entries manually, a smart compiler can do a single move
     * to memory (Clang 12):
     *    mov     rax, rdi
     *    vcvtps2dq       ymm0, ymm0
     *    vmovapd ymmword ptr [rdi], ymm0
     *    vzeroupper // useless since it's already stored in [rdi]
     *    ret
     * GCC 5.5 and 7.3, as well as MSVC, do not optimize such manual copying;
     * but by handling the internal registers themselves (as done below),
     * we achieve the following while still preserving Clang 12's optimization:
     *    mov     rax, rdi
     *    vcvtps2dq       ymm0, ymm0
     *    vextractf128    XMMWORD PTR [rdi+16], ymm0, 0x1
     *    vmovaps XMMWORD PTR [rdi], xmm0 // vmovdqu with MSVC
     *    vzeroupper // same as above
     *    ret
     */

    __m256i temp(_mm256_cvtps_epi32(a.data()));
    int_v res;

    internal_data(internal_data1(res)) = Vc_1::AVX::hi128(temp);
    internal_data(internal_data0(res)) = Vc_1::AVX::lo128(temp);

    return res;
#elif defined(Vc_IMPL_SSE2)
    return Vc::simd_cast<int_v>(Vc::int_v(_mm_cvtps_epi32(a.data())));
#else
    return Vc::simd_cast<int_v>(Vc::round(a));
#endif
}

/**
 * Get a vector containing first Vc::float_v::size() values of mask.
 * Each source mask element is considered to be a 8-bit integer
 */
static inline Vc::float_v fetch_mask_8(const quint8 *data) {
    uint_v data_i(data);
    return Vc::simd_cast<Vc::float_v>(int_v(data_i));
}

/**
 * Get an alpha values from Vc::float_v::size() pixels 32-bit each
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
template <bool aligned>
static inline Vc::float_v fetch_alpha_32(const void *data) {
    uint_v data_i;
    if (aligned) {
        data_i.load(static_cast<const quint32*>(data), Vc::Aligned);
    } else {
        data_i.load(static_cast<const quint32 *>(data), Vc::Unaligned);
    }

    return Vc::simd_cast<Vc::float_v>(int_v(data_i >> 24));
}

/**
 * Get color values from Vc::float_v::size() pixels 32-bit each
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
template <bool aligned>
static inline void fetch_colors_32(const void *data,
                            Vc::float_v &c1,
                            Vc::float_v &c2,
                            Vc::float_v &c3) {
    int_v data_i;
    if (aligned) {
        data_i.load(static_cast<const quint32*>(data), Vc::Aligned);
    } else {
        data_i.load(static_cast<const quint32*>(data), Vc::Unaligned);
    }

    const quint32 lowByteMask = 0xFF;
    uint_v mask(lowByteMask);

    c1 = Vc::simd_cast<Vc::float_v>(int_v((data_i >> 16) & mask));
    c2 = Vc::simd_cast<Vc::float_v>(int_v((data_i >> 8)  & mask));
    c3 = Vc::simd_cast<Vc::float_v>(int_v( data_i        & mask));
}

/**
 * Pack color and alpha values to Vc::float_v::size() pixels 32-bit each
 * (4 channels, 8 bit per channel).  The color data is considered
 * to be stored in the 3 least significant bytes of the pixel, alpha -
 * in the most significant byte
 *
 * NOTE: \p data must be aligned pointer!
 */
static inline void write_channels_32(void *data,
                                     Vc::float_v::AsArg alpha,
                                     Vc::float_v::AsArg c1,
                                     Vc::float_v::AsArg c2,
                                     Vc::float_v::AsArg c3) {
    const quint32 lowByteMask = 0xFF;

    uint_v mask(lowByteMask);
    uint_v v1 = uint_v(iRound(alpha)) << 24;
    uint_v v2 = (uint_v(iRound(c1)) & mask) << 16;
    uint_v v3 = (uint_v(iRound(c2)) & mask) <<  8;
    uint_v v4 = uint_v(iRound(c3)) & mask;
    v1 = v1 | v2;
    v3 = v3 | v4;
    (v1 | v3).store(static_cast<quint32*>(data), Vc::Aligned);
}

static inline void write_channels_32_unaligned(void *data,
                                               Vc::float_v::AsArg alpha,
                                               Vc::float_v::AsArg c1,
                                               Vc::float_v::AsArg c2,
                                               Vc::float_v::AsArg c3) {
    const quint32 lowByteMask = 0xFF;

    uint_v mask(lowByteMask);
    uint_v v1 = uint_v(iRound(alpha)) << 24;
    uint_v v2 = (uint_v(iRound(c1)) & mask) << 16;
    uint_v v3 = (uint_v(iRound(c2)) & mask) << 8;
    uint_v v4 = uint_v(iRound(c3)) & mask;
    v1 = v1 | v2;
    v3 = v3 | v4;
    (v1 | v3).store(static_cast<quint32*>(data), Vc::Unaligned);
}

/**
 * Composes src pixels into dst pixles. Is optimized for 32-bit-per-pixel
 * colorspaces. Uses \p Compositor strategy parameter for doing actual
 * math of the composition
 */
template<bool useMask, bool useFlow, class Compositor, int pixelSize>
    static void genericComposite(const KoCompositeOp::ParameterInfo& params)
{
    using namespace Arithmetic;

    const int vectorSize = static_cast<int>(Vc::float_v::size());
    const qint32 vectorInc = pixelSize * vectorSize;
    const qint32 linearInc = pixelSize;
    qint32 srcVectorInc = vectorInc;
    qint32 srcLinearInc = pixelSize;

    quint8 *dstRowStart = params.dstRowStart;
    const quint8 *maskRowStart = params.maskRowStart;
    const quint8* srcRowStart  = params.srcRowStart;
    typename Compositor::ParamsWrapper paramsWrapper(params);

    if (!params.srcRowStride) {
        if (pixelSize == 4) {
            KoStreamedMath::uint_v *buf = reinterpret_cast<KoStreamedMath::uint_v*>(Vc::malloc<quint32, Vc::AlignOnVector>(vectorSize));
            *buf = uint_v(*(reinterpret_cast<const quint32 *>(srcRowStart)));
            srcRowStart = reinterpret_cast<quint8*>(buf);
            srcLinearInc = 0;
            srcVectorInc = 0;
        } else {
            quint8 *buf = Vc::malloc<quint8, Vc::AlignOnVector>(vectorInc);
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
        uintptr_t srcPtrValue = reinterpret_cast<uintptr_t>(src);
        uintptr_t dstPtrValue = reinterpret_cast<uintptr_t>(dst);
        uintptr_t srcAlignment = srcPtrValue & pixelsAlignmentMask;
        uintptr_t dstAlignment = dstPtrValue & pixelsAlignmentMask;

        // Uncomment if facing problems with alignment:
        // Q_ASSERT_X(!(dstAlignment & 3), "Compositioning",
        //            "Pixel data must be aligned on pixels borders!");

        int blockAlign = params.cols;
        int blockAlignedVector = 0;
        int blockUnalignedVector = 0;
        int blockRest = 0;

        int *vectorBlock =
            srcAlignment == dstAlignment || !srcVectorInc ?
            &blockAlignedVector : &blockUnalignedVector;

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
            }
            else {
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

        for(int i = 0; i < blockAlign; i++) {
            Compositor::template compositeOnePixelScalar<useMask, _impl>(src, dst, mask, params.opacity, paramsWrapper);
            src += srcLinearInc;
            dst += linearInc;

            if(useMask) {
                mask++;
            }
        }

        for (int i = 0; i < blockAlignedVector; i++) {
            Compositor::template compositeVector<useMask, true, _impl>(src, dst, mask, params.opacity, paramsWrapper);
            src += srcVectorInc;
            dst += vectorInc;

            if (useMask) {
                mask += vectorSize;
            }
        }

        for (int i = 0; i < blockUnalignedVector; i++) {
            Compositor::template compositeVector<useMask, false, _impl>(src, dst, mask, params.opacity, paramsWrapper);
            src += srcVectorInc;
            dst += vectorInc;

            if (useMask) {
                mask += vectorSize;
            }
        }


        for(int i = 0; i < blockRest; i++) {
            Compositor::template compositeOnePixelScalar<useMask, _impl>(src, dst, mask, params.opacity, paramsWrapper);
            src += srcLinearInc;
            dst += linearInc;

            if (useMask) {
                mask++;
            }
        }

        srcRowStart  += params.srcRowStride;
        dstRowStart  += params.dstRowStride;

        if (useMask) {
            maskRowStart += params.maskRowStride;
        }
    }

#if BLOCKDEBUG
    dbgPigment << "I" << "rows:" << params.rows
             << "\tpad(S):" << totalBlockAlign
             << "\tbav(V):" << totalBlockAlignedVector
             << "\tbuv(V):" << totalBlockUnalignedVector
             << "\tres(S)" << totalBlockRest; // << srcAlignment << dstAlignment;
#endif

    if (!params.srcRowStride) {
        Vc::free<float>(reinterpret_cast<float*>(const_cast<quint8*>(srcRowStart)));
    }
}

template<bool useMask, bool useFlow, class Compositor>
    static void genericComposite32(const KoCompositeOp::ParameterInfo& params)
{
    genericComposite<useMask, useFlow, Compositor, 4>(params);
}

template<bool useMask, bool useFlow, class Compositor>
    static void genericComposite128(const KoCompositeOp::ParameterInfo& params)
{
    genericComposite<useMask, useFlow, Compositor, 16>(params);
}

template<bool useMask, bool useFlow, class Compositor>
    static void genericComposite64(const KoCompositeOp::ParameterInfo& params)
{
    genericComposite<useMask, useFlow, Compositor, 8>(params);
}

};

template<typename channels_type, Vc::Implementation _impl>
struct PixelWrapper
{
};

template<Vc::Implementation _impl>
struct PixelWrapper<quint16, _impl>
{
    using int_v = Vc::SimdArray<int, Vc::float_v::size()>;
    using uint_v = Vc::SimdArray<unsigned int, Vc::float_v::size()>;

    ALWAYS_INLINE
    static quint16 lerpMixedUintFloat(quint16 a, quint16 b, float alpha) {
        return OptiRound<_impl>::roundScalar(qint32(b - a) * alpha + a);
    }

    ALWAYS_INLINE
    static quint16 roundFloatToUint(float x) {
        return OptiRound<_impl>::roundScalar(x);
    }

    ALWAYS_INLINE
    static void normalizeAlpha(float &alpha) {
        const float uint16Rec1 = 1.0f / 65535.0f;
        alpha *= uint16Rec1;
    }

    ALWAYS_INLINE
    static void denormalizeAlpha(float &alpha) {
        const float uint16Max = 65535.0f;
        alpha *= uint16Max;
    }

    PixelWrapper()
        : mask(quint32(0xFFFF)),
          uint16Max(65535.0f),
          uint16Rec1(1.0f / 65535.0f)
    {
    }

    ALWAYS_INLINE
    void read(quint8 *dataDst, Vc::float_v &dst_c1, Vc::float_v &dst_c2, Vc::float_v &dst_c3, Vc::float_v &dst_alpha)
    {
        struct PackedPixel {
            float rrgg;
            float bbaa;
        };

        Vc::InterleavedMemoryWrapper<PackedPixel, Vc::float_v> dataWrapper((PackedPixel*)(dataDst));
        Vc::float_v v1, v2;
        Vc::tie(v1, v2) = dataWrapper[size_t(0)];
        uint_v pixelsC1C2 = uint_v(Vc::reinterpret_components_cast<int_v>(v1));
        uint_v pixelsC3Alpha = uint_v(Vc::reinterpret_components_cast<int_v>(v2));

        dst_c1 = Vc::simd_cast<Vc::float_v>(pixelsC1C2 & mask);
        dst_c2 = Vc::simd_cast<Vc::float_v>((pixelsC1C2 >> 16) & mask);
        dst_c3 = Vc::simd_cast<Vc::float_v>(pixelsC3Alpha & mask);
        dst_alpha = Vc::simd_cast<Vc::float_v>((pixelsC3Alpha >> 16) & mask);

        dst_alpha *= uint16Rec1;
    }

    ALWAYS_INLINE
    void write(quint8 *dataDst, Vc::float_v::AsArg c1, Vc::float_v::AsArg c2, Vc::float_v::AsArg c3, Vc::float_v &alpha)
    {
        alpha *= uint16Max;

        uint_v v1 = uint_v(int_v(Vc::round(c1)));
        uint_v v2 = uint_v(int_v(Vc::round(c2)));
        uint_v v3 = uint_v(int_v(Vc::round(c3)));
        uint_v v4 = uint_v(int_v(Vc::round(alpha)));
        uint_v c1c2 = ((v2 & mask) << 16) | (v1 & mask);
        uint_v c3ca = ((v4 & mask) << 16) | (v3 & mask);
        std::pair<int_v, int_v> out = Vc::interleave(c1c2, c3ca);
        out.first.store(reinterpret_cast<Vc::uint32_t*>(dataDst), Vc::Aligned);
        out.second.store(reinterpret_cast<Vc::uint32_t*>(dataDst) + out.first.size(), Vc::Aligned);
    }

    ALWAYS_INLINE
    void clearPixels(quint8 *dataDst) {
        memset(dataDst, 0, Vc::float_v::size() * sizeof(quint16) * 4);
    }

    ALWAYS_INLINE
    void copyPixels(const quint8 *dataSrc, quint8 *dataDst) {
        memcpy(dataDst, dataSrc, Vc::float_v::size() * sizeof(quint16) * 4);
    }


    const uint_v mask;
    const Vc::float_v uint16Max;
    const Vc::float_v uint16Rec1;
};

template<Vc::Implementation _impl>
struct PixelWrapper<quint8, _impl>
{
    using int_v = Vc::SimdArray<int, Vc::float_v::size()>;
    using uint_v = Vc::SimdArray<unsigned int, Vc::float_v::size()>;

    ALWAYS_INLINE
    static quint8 lerpMixedUintFloat(quint8 a, quint8 b, float alpha) {
        return KoStreamedMath<_impl>::lerp_mixed_u8_float(a, b, alpha);
    }

    ALWAYS_INLINE
    static quint8 roundFloatToUint(float x) {
        return KoStreamedMath<_impl>::round_float_to_u8(x);
    }

    ALWAYS_INLINE
    static void normalizeAlpha(float &alpha) {
        const float uint8Rec1 = 1.0f / 255.0f;
        alpha *= uint8Rec1;
    }

    ALWAYS_INLINE
    static void denormalizeAlpha(float &alpha) {
        const float uint8Max = 255.0f;
        alpha *= uint8Max;
    }

    PixelWrapper()
        : mask(quint32(0xFF)),
          uint8Max(255.0f),
          uint8Rec1(1.0f / 255.0f)
    {
    }

    ALWAYS_INLINE
    void read(quint8 *dataDst, Vc::float_v &dst_c1, Vc::float_v &dst_c2, Vc::float_v &dst_c3, Vc::float_v &dst_alpha)
    {
        dst_alpha = KoStreamedMath<_impl>::template fetch_alpha_32<false>(dataDst);
        KoStreamedMath<_impl>::template fetch_colors_32<false>(dataDst, dst_c1, dst_c2, dst_c3);

        dst_alpha *= uint8Rec1;
    }

    ALWAYS_INLINE
    void write(quint8 *dataDst, Vc::float_v::AsArg c1, Vc::float_v::AsArg c2, Vc::float_v::AsArg c3, Vc::float_v &alpha)
    {
        alpha *= uint8Max;

        KoStreamedMath<_impl>::write_channels_32_unaligned(dataDst, alpha, c1, c2, c3);
    }

    ALWAYS_INLINE
    void clearPixels(quint8 *dataDst) {
        memset(dataDst, 0, Vc::float_v::size() * sizeof(quint8) * 4);
    }

    ALWAYS_INLINE
    void copyPixels(const quint8 *dataSrc, quint8 *dataDst) {
        memcpy(dataDst, dataSrc, Vc::float_v::size() * sizeof(quint8) * 4);
    }


    const uint_v mask;
    const Vc::float_v uint8Max;
    const Vc::float_v uint8Rec1;
};

template<Vc::Implementation _impl>
struct PixelWrapper<float, _impl>
{
    struct Pixel {
        float red;
        float green;
        float blue;
        float alpha;
    };

    ALWAYS_INLINE
    static float lerpMixedUintFloat(float a, float b, float alpha) {
        return Arithmetic::lerp(a,b,alpha);
    }

    ALWAYS_INLINE
    static float roundFloatToUint(float x) {
        return x;
    }

    ALWAYS_INLINE
    static void normalizeAlpha(float &alpha) {
        Q_UNUSED(alpha);
    }

    ALWAYS_INLINE
    static void denormalizeAlpha(float &alpha) {
        Q_UNUSED(alpha);
    }

    PixelWrapper()
        : indexes(Vc::IndexesFromZero)
    {
    }

    ALWAYS_INLINE
    void read(quint8 *dstPtr, Vc::float_v &dst_c1, Vc::float_v &dst_c2, Vc::float_v &dst_c3, Vc::float_v &dst_alpha)
    {
        Vc::InterleavedMemoryWrapper<Pixel, Vc::float_v> dataDst(reinterpret_cast<Pixel*>(dstPtr));
        tie(dst_c1, dst_c2, dst_c3, dst_alpha) = dataDst[indexes];

    }

    ALWAYS_INLINE
    void write(quint8 *dstPtr, Vc::float_v &dst_c1, Vc::float_v &dst_c2, Vc::float_v &dst_c3, Vc::float_v &dst_alpha)
    {
        Vc::InterleavedMemoryWrapper<Pixel, Vc::float_v> dataDst(reinterpret_cast<Pixel*>(dstPtr));
        dataDst[indexes] = tie(dst_c1, dst_c2, dst_c3, dst_alpha);
    }

    ALWAYS_INLINE
    void clearPixels(quint8 *dataDst) {
        memset(dataDst, 0, Vc::float_v::size() * sizeof(float) * 4);
    }

    ALWAYS_INLINE
    void copyPixels(const quint8 *dataSrc, quint8 *dataDst) {
        memcpy(dataDst, dataSrc, Vc::float_v::size() * sizeof(float) * 4);
    }

    const Vc::float_v::IndexType indexes;
};

namespace KoStreamedMathFunctions {

template<int pixelSize>
ALWAYS_INLINE void clearPixel(quint8* dst);

template<>
ALWAYS_INLINE void clearPixel<4>(quint8* dst)
{
    quint32 *d = reinterpret_cast<quint32*>(dst);
    *d = 0;
}

template<>
ALWAYS_INLINE void clearPixel<8>(quint8* dst)
{
    quint64 *d = reinterpret_cast<quint64*>(dst);
    d[0] = 0;
}

template<>
ALWAYS_INLINE void clearPixel<16>(quint8* dst)
{
    quint64 *d = reinterpret_cast<quint64*>(dst);
    d[0] = 0;
    d[1] = 0;
}

template<int pixelSize>
ALWAYS_INLINE void copyPixel(const quint8 *src, quint8* dst);

template<>
ALWAYS_INLINE void copyPixel<4>(const quint8 *src, quint8* dst)
{
    const quint32 *s = reinterpret_cast<const quint32*>(src);
    quint32 *d = reinterpret_cast<quint32*>(dst);
    *d = *s;
}

template<>
ALWAYS_INLINE void copyPixel<8>(const quint8 *src, quint8* dst)
{
    const quint64 *s = reinterpret_cast<const quint64*>(src);
    quint64 *d = reinterpret_cast<quint64*>(dst);
    d[0] = s[0];
}

template<>
ALWAYS_INLINE void copyPixel<16>(const quint8 *src, quint8* dst)
{
    const quint64 *s = reinterpret_cast<const quint64*>(src);
    quint64 *d = reinterpret_cast<quint64*>(dst);
    d[0] = s[0];
    d[1] = s[1];
}
}

#endif /* __KOSTREAMED_MATH_H */
