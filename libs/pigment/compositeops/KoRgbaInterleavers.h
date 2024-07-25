/*
 * SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef KO_RGBA_INTERLEAVERS
#define KO_RGBA_INTERLEAVERS

#include <xsimd_extensions/xsimd.hpp>

#if XSIMD_VERSION_MAJOR >= 10
#error "The interleavers use per-lane zipping semantics, which are not compatible with xsimd 10"
#endif

using namespace xsimd;

template<typename T, size_t S>
using enable_sized_t = typename std::enable_if<sizeof(T) == S, int>::type;

template<typename T, size_t S>
using enable_sized_integral_t =
    typename std::enable_if<std::is_integral<T>::value && sizeof(T) == S,
                            int>::type;

template<typename T, typename A, size_t S>
using enable_sized_vector_t = typename std::enable_if<batch<T, A>::size == S, int>::type;

#if XSIMD_WITH_AVX2
template<typename A>
inline batch<float, A> exchange_mid_halves(batch<float, A> const &a, kernel::requires_arch<avx2>) noexcept
{
    return _mm256_castpd_ps(_mm256_permute4x64_pd(_mm256_castps_pd(a.data), 0xD8));
}

template<typename T, typename A, enable_sized_integral_t<T, 4> = 0>
inline batch<T, A> exchange_mid_halves(batch<T, A> const &a, kernel::requires_arch<avx2>) noexcept
{
    return _mm256_permute4x64_epi64(a.data, 0xD8);
}
#endif

#if XSIMD_WITH_AVX
template<typename A>
inline batch<float, A> merge_low(batch<float, A> const &a, batch<float, A> const &b, kernel::requires_arch<avx>) noexcept
{
    return _mm256_insertf128_ps(a, _mm256_castps256_ps128(b), 1);
}

template<typename T, typename A, enable_sized_integral_t<T, 4> = 0>
inline batch<T, A> merge_low(batch<T, A> const &a, batch<T, A> const &b, kernel::requires_arch<avx>) noexcept
{
    return _mm256_insertf128_si256(a, _mm256_castsi256_si128(b), 1);
}

template<typename A>
inline batch<float, A> merge_high(batch<float, A> const &a, batch<float, A> const &b, kernel::requires_arch<avx>) noexcept
{
    return _mm256_permute2f128_ps(a, b, 0x31);
}

template<typename T, typename A, enable_sized_integral_t<T, 4> = 0>
inline batch<T, A> merge_high(batch<T, A> const &a, batch<T, A> const &b, kernel::requires_arch<avx>) noexcept
{
    return _mm256_permute2f128_si256(a, b, 0x31);
}

template<typename A>
inline batch<float, A> duplicate_low_halves(batch<float, A> const &a, batch<float, A> const &b, kernel::requires_arch<avx>) noexcept
{
    return _mm256_shuffle_ps(a, b, _MM_SHUFFLE(2, 0, 2, 0));
}

template<typename T, typename A, enable_sized_integral_t<T, 4> = 0>
inline batch<T, A> duplicate_low_halves(batch<T, A> const &a, batch<T, A> const &b, kernel::requires_arch<avx>) noexcept
{
    return _mm256_castps_si256(_mm256_shuffle_ps(_mm256_castsi256_ps(a), _mm256_castsi256_ps(b), _MM_SHUFFLE(2, 0, 2, 0)));
}

template<typename A>
inline batch<float, A> duplicate_high_halves(batch<float, A> const &a, batch<float, A> const &b, kernel::requires_arch<avx>) noexcept
{
    return _mm256_shuffle_ps(a, b, _MM_SHUFFLE(3, 1, 3, 1));
}

template<typename T, typename A, enable_sized_integral_t<T, 4> = 0>
inline batch<T, A> duplicate_high_halves(batch<T, A> const &a, batch<T, A> const &b, kernel::requires_arch<avx>) noexcept
{
    return _mm256_castps_si256(_mm256_shuffle_ps(_mm256_castsi256_ps(a), _mm256_castsi256_ps(b), _MM_SHUFFLE(3, 1, 3, 1)));
}
#endif

template<size_t N>
struct KoRgbaInterleavers;

template<>
struct KoRgbaInterleavers<16> {
    template<bool aligned, typename T, typename A, enable_sized_integral_t<T, 4> = 0, enable_sized_vector_t<T, A, 4> = 0>
    static inline void interleave(void *dst, batch<T, A> const &a, batch<T, A> const &b, kernel::requires_arch<generic>)
    {
        auto *dstPtr = static_cast<T *>(dst);
        using U = std::conditional_t<aligned, aligned_mode, unaligned_mode>;
        const auto t1 = zip_lo(a, b);
        const auto t2 = zip_hi(a, b);
        t1.store(dstPtr, U{});
        t2.store(dstPtr + batch<T, A>::size, U{});
    }

    // The AVX versions are handmade ports of the ones generated
    // by Clang 14.0.0: https://godbolt.org/z/Ts8MWosW3
    // Except for interleave(avx) which comes from GCC 11.2

#if XSIMD_WITH_AVX
    template<bool aligned, typename T, typename A, enable_sized_t<T, 4> = 0>
    static inline void interleave(void *dst, batch<T, A> const &a, batch<T, A> const &b, kernel::requires_arch<avx>)
    {
        auto *dstPtr = static_cast<T *>(dst);
        using U = std::conditional_t<aligned, aligned_mode, unaligned_mode>;
        const auto t1 = zip_lo(a, b);
        const auto t2 = zip_hi(a, b);
        const auto src1 = merge_low(t1, t2, A{});
        const auto src2 = merge_high(t1, t2, A{});
        src1.store(dstPtr, U{});
        src2.store(dstPtr + batch<T, A>::size, U{});
    }
#endif

    template<typename T, typename A, bool aligned = false>
    static inline void interleave(void *dst, batch<T, A> const &a, batch<T, A> const &b)
    {
        return interleave<aligned>(dst, a, b, A{});
    }

    template<bool aligned, typename T, typename A, enable_sized_integral_t<T, 4> = 0, enable_sized_vector_t<T, A, 4> = 0>
    static inline void deinterleave(const void *src, batch<T, A> &dst1, batch<T, A> &dst2, kernel::requires_arch<generic>)
    {
        const auto *srcPtr = static_cast<const T *>(src);
        using U = std::conditional_t<aligned, aligned_mode, unaligned_mode>;

        const auto a = batch<T, A>::load(srcPtr, U{});
        const auto b = batch<T, A>::load(srcPtr + batch<T, A>::size, U{});
        const auto t1 = zip_lo(a, b);
        const auto t2 = zip_hi(a, b);
        dst1 = zip_lo(t1, t2);
        dst2 = zip_hi(t1, t2);
    }

#if XSIMD_WITH_AVX2
    template<bool aligned, typename T, typename A, enable_sized_t<T, 4> = 0>
    static inline void deinterleave(const void *src, batch<T, A> &a, batch<T, A> &b, kernel::requires_arch<avx2>)
    {
        const auto *srcPtr = static_cast<const T *>(src);
        using U = std::conditional_t<aligned, aligned_mode, unaligned_mode>;
        const auto src1 = batch<T, A>::load(srcPtr, U{});
        const auto src2 = batch<T, A>::load(srcPtr + batch<T, A>::size, U{});
        const auto t1 = duplicate_low_halves(src1, src2, A{});
        a = exchange_mid_halves(t1, A{});
        const auto t2 = duplicate_high_halves(src1, src2, A{});
        b = exchange_mid_halves(t2, A{});
    }
#endif
#if XSIMD_WITH_AVX
    template<bool aligned, typename T, typename A, enable_sized_t<T, 4> = 0>
    static inline void deinterleave(const void *src, batch<T, A> &a, batch<T, A> &b, kernel::requires_arch<avx>)
    {
        const auto *srcPtr = static_cast<const T *>(src);
        using U = std::conditional_t<aligned, aligned_mode, unaligned_mode>;
        const auto src1 = batch<T, A>::load(srcPtr, U{});
        const auto src2 =
            batch<T, A>::load(srcPtr + batch<T, A>::size, U{});
        const auto t1 = merge_high(src1, src2, A{});
        const auto t2 = merge_low(src1, src2, A{});
        a.data = duplicate_low_halves(t2, t1, A{});
        b.data = duplicate_high_halves(t2, t1, A{});
    }
#endif

    template<typename T, typename A, bool aligned = false>
    static inline void deinterleave(const void *src, batch<T, A> &a, batch<T, A> &b)
    {
        return deinterleave<aligned>(src, a, b, A{});
    }
};

template<>
struct KoRgbaInterleavers<32> {
    template<typename T, typename A, bool aligned = false, enable_sized_t<T, 4> = 0, enable_sized_vector_t<T, A, 4> = 0>
    static inline void
    interleave(void *dst, batch<T, A> const &a, batch<T, A> const &b, batch<T, A> const &c, batch<T, A> const &d, kernel::requires_arch<generic>)
    {
        auto *dstPtr = static_cast<T *>(dst);
        using U = std::conditional_t<aligned, aligned_mode, unaligned_mode>;

        const auto t1 = zip_lo(a, c);
        const auto t2 = zip_hi(a, c);
        const auto t3 = zip_lo(b, d);
        const auto t4 = zip_hi(b, d);
        const auto src1 = zip_lo(t1, t3);
        const auto src2 = zip_hi(t1, t3);
        const auto src3 = zip_lo(t2, t4);
        const auto src4 = zip_hi(t2, t4);
        src1.store(dstPtr, U{});
        src2.store(dstPtr + batch<T, A>::size, U{});
        src3.store(dstPtr + batch<T, A>::size * 2, U{});
        src4.store(dstPtr + batch<T, A>::size * 3, U{});
    }

#if XSIMD_WITH_AVX
    template<typename T, typename A, bool aligned = false, enable_sized_t<T, 4> = 0>
    static inline void
    interleave(void *dst, batch<T, A> const &a, batch<T, A> const &b, batch<T, A> const &c, batch<T, A> const &d, kernel::requires_arch<avx>)
    {
        auto *dstPtr = static_cast<T *>(dst);
        using U = std::conditional_t<aligned, aligned_mode, unaligned_mode>;

        const auto t1 = zip_lo(a, c);
        const auto t2 = zip_lo(b, d);
        const auto t3 = zip_hi(a, c);
        const auto t4 = zip_hi(b, d);
        const auto t5 = zip_lo(t1, t2);
        const auto t6 = zip_hi(t1, t2);
        const auto t7 = zip_lo(t3, t4);
        const auto t8 = zip_hi(t3, t4);
        const auto src1 = merge_low(t5, t6, A{});
        const auto src2 = merge_low(t7, t8, A{});
        const auto src3 = merge_high(t5, t6, A{});
        const auto src4 = merge_high(t7, t8, A{});
        src1.store(dstPtr, U{});
        src2.store(dstPtr + batch<T, A>::size, U{});
        src3.store(dstPtr + batch<T, A>::size * 2, U{});
        src4.store(dstPtr + batch<T, A>::size * 3, U{});
    }
#endif

    template<typename T, typename A, bool aligned = false>
    static inline void interleave(void *dst, batch<T, A> const &a, batch<T, A> const &b, batch<T, A> const &c, batch<T, A> const &d)
    {
        return interleave<T, A, aligned>(dst, a, b, c, d, A{});
    }

    template<typename T, typename A, bool aligned = false, enable_sized_t<T, 4> = 0, enable_sized_vector_t<T, A, 4> = 0>
    static inline void deinterleave(const void *src, batch<T, A> &a, batch<T, A> &b, batch<T, A> &c, batch<T, A> &d, kernel::requires_arch<generic>)
    {
        const auto *srcPtr = static_cast<const T *>(src);
        using U = std::conditional_t<aligned, aligned_mode, unaligned_mode>;

        const auto t1 = batch<T, A>::load(srcPtr, U{});
        const auto t2 = batch<T, A>::load(srcPtr + batch<T, A>::size, U{});
        const auto t3 = batch<T, A>::load(srcPtr + batch<T, A>::size * 2, U{});
        const auto t4 = batch<T, A>::load(srcPtr + batch<T, A>::size * 3, U{});
        const auto src1 = zip_lo(t1, t3);
        const auto src2 = zip_hi(t1, t3);
        const auto src3 = zip_lo(t2, t4);
        const auto src4 = zip_hi(t2, t4);
        a = zip_lo(src1, src3);
        b = zip_hi(src1, src3);
        c = zip_lo(src2, src4);
        d = zip_hi(src2, src4);
    }

#if XSIMD_WITH_AVX
    template<typename T, typename A, bool aligned = false, enable_sized_t<T, 4> = 0>
    static inline void deinterleave(const void *src, batch<T, A> &a, batch<T, A> &b, batch<T, A> &c, batch<T, A> &d, kernel::requires_arch<avx>)
    {
        const auto *srcPtr = static_cast<const T *>(src);
        using U = std::conditional_t<aligned, aligned_mode, unaligned_mode>;

        const auto a0b0c0d0_a1b1c1d1 = batch<T, A>::load(srcPtr, U{});
        const auto a2b2c2d2_a3b3c3d3 =
            batch<T, A>::load(srcPtr + batch<T, A>::size, U{});
        const auto a4b4c4d4_a5b5c5d5 =
            batch<T, A>::load(srcPtr + batch<T, A>::size * 2, U{});
        const auto a6b6c6d6_a7b7c7d7 =
            batch<T, A>::load(srcPtr + batch<T, A>::size * 3, U{});

        const auto a0a2b0b2_a1a3b1b3 =
            zip_lo(a0b0c0d0_a1b1c1d1, a2b2c2d2_a3b3c3d3);
        const auto c0c2d0d2_c1c3d1d3 =
            zip_hi(a0b0c0d0_a1b1c1d1, a2b2c2d2_a3b3c3d3);
        const auto a0a2b0b2_c0c2d0d2 =
            merge_low(a0a2b0b2_a1a3b1b3, c0c2d0d2_c1c3d1d3, A{});
        const auto a1a3b1b3_c1c3d1d3 =
            merge_high(a0a2b0b2_a1a3b1b3, c0c2d0d2_c1c3d1d3, A{});
        const auto a0a1a2a3_c0c1c2c3 =
            zip_lo(a0a2b0b2_c0c2d0d2, a1a3b1b3_c1c3d1d3);
        const auto b0b1b2b3_d0d1d2d3 =
            zip_hi(a0a2b0b2_c0c2d0d2, a1a3b1b3_c1c3d1d3);

        const auto a4a6b4b6_a5a7b5b7 =
            zip_lo(a4b4c4d4_a5b5c5d5, a6b6c6d6_a7b7c7d7);
        const auto c4c6d4d6_c5c7d5d7 =
            zip_hi(a4b4c4d4_a5b5c5d5, a6b6c6d6_a7b7c7d7);
        const auto a4a6b4b6_c4c6d4d6 =
            merge_low(a4a6b4b6_a5a7b5b7, c4c6d4d6_c5c7d5d7, A{});
        const auto a5a7b5b7_c5c7d5d7 =
            merge_high(a4a6b4b6_a5a7b5b7, c4c6d4d6_c5c7d5d7, A{});
        const auto a4a5a6a7_c4c5c6c7 =
            zip_lo(a4a6b4b6_c4c6d4d6, a5a7b5b7_c5c7d5d7);
        const auto b4b5b6b7_d4d5d6d7 =
            zip_hi(a4a6b4b6_c4c6d4d6, a5a7b5b7_c5c7d5d7);

        a = merge_low(a0a1a2a3_c0c1c2c3, a4a5a6a7_c4c5c6c7, A{});
        b = merge_low(b0b1b2b3_d0d1d2d3, b4b5b6b7_d4d5d6d7, A{});
        c = merge_high(a0a1a2a3_c0c1c2c3, a4a5a6a7_c4c5c6c7, A{});
        d = merge_high(b0b1b2b3_d0d1d2d3, b4b5b6b7_d4d5d6d7, A{});
    }
#endif

    template<typename T, typename A, bool aligned = false>
    static inline void deinterleave(const void *src, batch<T, A> &a, batch<T, A> &b, batch<T, A> &c, batch<T, A> &d)
    {
        return deinterleave<T, A, aligned>(src, a, b, c, d, A{});
    }
};

#endif // KO_RGBA_INTERLEAVERS
