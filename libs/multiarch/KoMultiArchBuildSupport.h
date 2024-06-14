/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef KO_MULTI_ARCH_BUILD_SUPPORT_H
#define KO_MULTI_ARCH_BUILD_SUPPORT_H

#include "kritamultiarch_export.h"

#include <utility>
#include <xsimd_extensions/xsimd.hpp>

#include <QDebug>

KRITAMULTIARCH_EXPORT
std::tuple<bool, bool> vectorizationConfiguration();

template<class FactoryType, class... Args>
auto createOptimizedClass(Args &&...param)
{
#ifdef HAVE_XSIMD
    bool useVectorization = true;
    bool disableAVXOptimizations = false;

    std::tie(useVectorization, disableAVXOptimizations) =
        vectorizationConfiguration();

    if (!useVectorization) {
        qWarning() << "WARNING: vector instructions disabled by the "
                      "\'amdDisableVectorWorkaround\' option!";
        return FactoryType::template create<xsimd::generic>(
            std::forward<Args>(param)...);
    }

    if (disableAVXOptimizations
        && (xsimd::available_architectures().fma3_avx2
            || xsimd::available_architectures().avx)) {
        qWarning() << "WARNING: AVX and AVX2 optimizations are disabled by the "
                      "\'disableAVXOptimizations\' option!";
    }

#ifdef Q_PROCESSOR_X86

    if (!disableAVXOptimizations &&
        xsimd::available_architectures().fma3_avx2) {

        return FactoryType::template create<xsimd::fma3<xsimd::avx2>>(
            std::forward<Args>(param)...);

    } else if (!disableAVXOptimizations &&
               xsimd::available_architectures().avx) {

        return FactoryType::template create<xsimd::avx>(
            std::forward<Args>(param)...);

    } else if (xsimd::available_architectures().sse4_1) {
        return FactoryType::template create<xsimd::sse4_1>(
            std::forward<Args>(param)...);
    } else if (xsimd::available_architectures().ssse3) {
        return FactoryType::template create<xsimd::ssse3>(
            std::forward<Args>(param)...);
    } else if (xsimd::available_architectures().sse2) {
        return FactoryType::template create<xsimd::sse2>(
            std::forward<Args>(param)...);
    }
#elif XSIMD_WITH_NEON64
    if (xsimd::available_architectures().neon64) {
        return FactoryType::template create<xsimd::neon64>(
            std::forward<Args>(param)...);
    }
#elif XSIMD_WITH_NEON
    if (xsimd::available_architectures().neon) {
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
