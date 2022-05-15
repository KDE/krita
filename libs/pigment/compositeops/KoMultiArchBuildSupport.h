/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef KO_MULTI_ARCH_BUILD_SUPPORT_H
#define KO_MULTI_ARCH_BUILD_SUPPORT_H


#include <QDebug>
#include <ksharedconfig.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <xsimd_extensions/xsimd.hpp>

template<class FactoryType>
typename FactoryType::ReturnType
createOptimizedClass(typename FactoryType::ParamType param)
{
#ifdef HAVE_XSIMD
    const auto best_arch = xsimd::available_architectures().best;

#ifdef Q_PROCESSOR_X86
    static bool isConfigInitialized = false;
    static bool useVectorization = true;
    static bool disableAVXOptimizations = false;

    if (!isConfigInitialized) {
        KConfigGroup cfg = KSharedConfig::openConfig()->group("");
        // use the old key name for compatibility
        useVectorization = !cfg.readEntry("amdDisableVectorWorkaround", false);
        disableAVXOptimizations = cfg.readEntry("disableAVXOptimizations", false);
    }

    if (!useVectorization) {
        qWarning() << "WARNING: vector instructions disabled by the \'amdDisableVectorWorkaround\' option!";
        return FactoryType::template create<xsimd::generic>(param);
    }

    if (disableAVXOptimizations
        && (xsimd::avx::version() <= best_arch || xsimd::avx2::version() <= best_arch)) {
        qWarning() << "WARNING: AVX and AVX2 optimizations are disabled by the \'disableAVXOptimizations\' option!";
    }

    /**
    * We use SSE2, SSSE3, SSE4.1, AVX and AVX2+FMA.
    * The rest are integer and string instructions mostly.
    */
    if (!disableAVXOptimizations && xsimd::fma3<xsimd::avx2>::version() <= best_arch) {
        return FactoryType::template create<xsimd::fma3<xsimd::avx2>>(param);
    } else if (!disableAVXOptimizations && xsimd::avx::version() <= best_arch) {
        return FactoryType::template create<xsimd::avx>(param);
    } else if (xsimd::sse4_1::version() <= best_arch) {
        return FactoryType::template create<xsimd::sse4_1>(param);
    } else if (xsimd::ssse3::version() <= best_arch) {
        return FactoryType::template create<xsimd::ssse3>(param);
    } else if (xsimd::sse2::version() <= best_arch) {
        return FactoryType::template create<xsimd::sse2>(param);
    }
#elif XSIMD_WITH_NEON64
    if (xsimd::neon64::version() <= best_arch) {
        return FactoryType::template create<xsimd::neon64>(param);
    }
#elif XSIMD_WITH_NEON
    if (xsimd::neon::version() <= best_arch) {
        return FactoryType::template create<xsimd::neon>(param);
    }
#endif // XSIMD_WITH_SSE2
#endif // HAVE_XSIMD

    return FactoryType::template create<xsimd::generic>(param);
}

template<class FactoryType>
typename FactoryType::ReturnType
createOptimizedClass(typename FactoryType::ParamType param, bool forceScalarImplemetation)
{
    if(forceScalarImplemetation){
        return FactoryType::template create<xsimd::generic>(param);
    }
    return createOptimizedClass<FactoryType>(param);
}

#endif /* KO_MULTI_ARCH_BUILD_SUPPORT_H */
