/*
 * SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef KIS_XSIMD_ARCH_HPP
#define KIS_XSIMD_ARCH_HPP

#include "./xsimd_config.hpp"

// Architecture initialization. Borrowed from Vc
// Define the following strings to a unique integer, which is the only type the
// preprocessor can compare. This allows to use -DXSIMD_IMPL=SSE3. The
// preprocessor will then consider XSIMD_IMPL and SSE3 to be equal.
// An additional define IMPL_MASK allows to detect the FMA extension.

#define Scalar 0x00100000
#define SSE2 0x00200000
#define SSE3 0x00300000
#define SSSE3 0x00400000
#define SSE4_1 0x00500000
#define SSE4_2 0x00600000
#define FMA4 0x00700000
#define AVX 0x00800000
#define AVX2 0x00900000
#define AVX512F 0x00A00000
#define AVX512BW 0x00B00000
#define AVX512CD 0x00C00000
#define AVX512DQ 0x00D00000
#define NEON 0x10100000
#define NEON64 0x10200000

#define FMA 0x00000001

#define Intel_Architecture 0x00000000
#define Arm_Architecture 0x10000000

#define IMPL_MASK 0xFFF00000
#define PLATFORM_MASK 0xF0000000

namespace xsimd
{
# if XSIMD_VERSION_MAJOR >= 14
using generic = common;
# endif
#if defined(XSIMD_IMPL) && (XSIMD_IMPL & IMPL_MASK) == Scalar
using current_arch = generic;
#elif !defined(XSIMD_IMPL)
using current_arch = default_arch;
#elif (XSIMD_IMPL & IMPL_MASK) == SSE2
using current_arch = sse2;
#elif (XSIMD_IMPL & IMPL_MASK) == SSE3
using current_arch = sse3;
#elif (XSIMD_IMPL & IMPL_MASK) == SSSE3
using current_arch = ssse3;
#elif (XSIMD_IMPL & IMPL_MASK) == SSE4_1
using current_arch = sse4_1;
#elif (XSIMD_IMPL & IMPL_MASK) == SSE4_2
#if (XSIMD_IMPL & FMA)
using current_arch = fma3<sse4_2>;
#else
using current_arch = sse4_2;
#endif
#elif (XSIMD_IMPL & IMPL_MASK) == FMA4
using current_arch = fma4;
#elif (XSIMD_IMPL & IMPL_MASK) == AVX
#if (XSIMD_IMPL & FMA)
using current_arch = fma3<avx>;
#else
using current_arch = avx;
#endif
#elif (XSIMD_IMPL & IMPL_MASK) == AVX2
#if (XSIMD_IMPL & FMA)
using current_arch = fma3<avx2>;
#else
using current_arch = avx2;
#endif
#elif (XSIMD_IMPL & IMPL_MASK) == AVX512F
using current_arch = avx512f;
#elif (XSIMD_IMPL & IMPL_MASK) == AVX512CD
using current_arch = avx512cd;
#elif (XSIMD_IMPL & IMPL_MASK) == AVX512DQ
using current_arch = avx512dq;
#elif (XSIMD_IMPL & IMPL_MASK) == AVX512BW
using current_arch = avx512bw;
#elif (XSIMD_IMPL & IMPL_MASK) == NEON
using current_arch = neon;
#elif (XSIMD_IMPL & IMPL_MASK) == NEON64
using current_arch = neon64;
#endif
}; // namespace xsimd

// xsimd extension to block AppleClang's auto-lipoization of
// compiled objects.
// If the defined instruction sets don't match what's expected
// from the build flags, zonk out the included file.

#if !defined(XSIMD_IMPL) || defined(XSIMD_IMPL) && (XSIMD_IMPL & IMPL_MASK) == Scalar
#define XSIMD_UNIVERSAL_BUILD_PASS 3
#elif XSIMD_WITH_SSE2 && (XSIMD_IMPL & PLATFORM_MASK) == Intel_Architecture
#define XSIMD_UNIVERSAL_BUILD_PASS 2
#elif (XSIMD_WITH_NEON || XSIMD_WITH_NEON64) && (XSIMD_IMPL & PLATFORM_MASK) == Arm_Architecture
#define XSIMD_UNIVERSAL_BUILD_PASS 1
#endif

#ifndef XSIMD_UNIVERSAL_BUILD_PASS
#define XSIMD_UNIVERSAL_BUILD_PASS 0
#endif

#undef Scalar
#undef SSE2
#undef SSE3
#undef SSSE3
#undef SSE4_1
#undef SSE4_2
#undef AVX
#undef AVX2
#undef AVX512F
#undef AVX512BW
#undef AVX512CD
#undef AVX512DQ
#undef NEON
#undef NEON64

#undef FMA3
#undef FMA4
#undef IMPL_MASK

#endif // KIS_XSIMD_ARCH_HPP
