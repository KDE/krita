/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef KO_MULTI_ARCH_BUILD_SUPPORT_H
#define KO_MULTI_ARCH_BUILD_SUPPORT_H

#include <utility>
#include <xsimd_extensions/xsimd.hpp>

#include "KisSupportedArchitectures.h"

template<class FactoryType, class... Args>
auto createOptimizedClass(Args &&...param)
{
#ifdef HAVE_XSIMD
    const unsigned int best_arch = KisSupportedArchitectures::bestArch();

#ifdef Q_PROCESSOR_X86
    if (xsimd::fma3<xsimd::avx2>::version() <= best_arch) {
        return FactoryType::template create<xsimd::fma3<xsimd::avx2>>(
            std::forward<Args>(param)...);
    } else if (xsimd::avx::version() <= best_arch) {
        return FactoryType::template create<xsimd::avx>(
            std::forward<Args>(param)...);
    } else if (xsimd::sse4_1::version() <= best_arch) {
        return FactoryType::template create<xsimd::sse4_1>(
            std::forward<Args>(param)...);
    } else if (xsimd::ssse3::version() <= best_arch) {
        return FactoryType::template create<xsimd::ssse3>(
            std::forward<Args>(param)...);
    } else if (xsimd::sse2::version() <= best_arch) {
        return FactoryType::template create<xsimd::sse2>(
            std::forward<Args>(param)...);
    }
#elif XSIMD_WITH_NEON64
    if (xsimd::neon64::version() <= best_arch) {
        return FactoryType::template create<xsimd::neon64>(
            std::forward<Args>(param)...);
    }
#elif XSIMD_WITH_NEON
    if (xsimd::neon::version() <= best_arch) {
        return FactoryType::template create<xsimd::neon>(
            std::forward<Args>(param)...);
    }
#endif // XSIMD_WITH_SSE2
#endif // HAVE_XSIMD

    return FactoryType::template create<xsimd::generic>(
        std::forward<Args>(param)...);
}

template<class FactoryType, class... Args>
auto createScalarClass(Args &&...params)
{
    return FactoryType::template create<xsimd::generic>(
        std::forward<Args>(params)...);
}

#endif /* KO_MULTI_ARCH_BUILD_SUPPORT_H */
