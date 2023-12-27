/*
 * SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisSupportedArchitectures.h"

#include <KConfigGroup>
#include <KSharedConfig>
#include <kis_debug.h>

#include "xsimd_extensions/xsimd.hpp"

std::tuple<bool, bool> vectorizationConfiguration()
{
    static const std::tuple<bool, bool> vectorization = [&]() {
        KConfigGroup cfg = KSharedConfig::openConfig()->group("");
        // use the old key name for compatibility
        const bool useVectorization =
            !cfg.readEntry("amdDisableVectorWorkaround", false);
        const bool disableAVXOptimizations =
            cfg.readEntry("disableAVXOptimizations", false);

        return std::make_tuple(useVectorization, disableAVXOptimizations);
    }();

    return vectorization;
}

QString KisSupportedArchitectures::baseArchName()
{
    return xsimd::current_arch::name();
}

QString KisSupportedArchitectures::bestArchName()
{
#ifdef HAVE_XSIMD
    const unsigned int best_arch = xsimd::available_architectures().best;

#ifdef Q_PROCESSOR_X86
    bool useVectorization = true;
    bool disableAVXOptimizations = false;

    std::tie(useVectorization, disableAVXOptimizations) =
        vectorizationConfiguration();

    if (!useVectorization) {
        qWarning() << "WARNING: vector instructions disabled by the "
                      "\'amdDisableVectorWorkaround\' option!";
        return "Scalar";
    }

    if (disableAVXOptimizations
        && (xsimd::avx::version() <= best_arch
            || xsimd::avx2::version() <= best_arch)) {
        qWarning() << "WARNING: AVX and AVX2 optimizations are disabled by the "
                      "\'disableAVXOptimizations\' option!";
    }

    /**
     * We use SSE2, SSSE3, SSE4.1, AVX and AVX2+FMA.
     * The rest are integer and string instructions mostly.
     */
    if (!disableAVXOptimizations
        && xsimd::fma3<xsimd::avx2>::version() <= best_arch) {
        return xsimd::fma3<xsimd::avx2>::name();
    } else if (!disableAVXOptimizations && xsimd::avx::version() <= best_arch) {
        return xsimd::avx::name();
    } else if (xsimd::sse4_1::version() <= best_arch) {
        return xsimd::sse4_1::name();
    } else if (xsimd::ssse3::version() <= best_arch) {
        return xsimd::ssse3::name();
    } else if (xsimd::sse2::version() <= best_arch) {
        return xsimd::sse2::name();
    }
#elif XSIMD_WITH_NEON64
    if (xsimd::neon64::version() <= best_arch) {
        return xsimd::neon64::name();
    }
#elif XSIMD_WITH_NEON
    if (xsimd::neon::version() <= best_arch) {
        return xsimd::neon::name();
    }
#endif // XSIMD_WITH_SSE2
#endif // HAVE_XSIMD
    return xsimd::current_arch::name();
}

unsigned int KisSupportedArchitectures::bestArch()
{
#ifdef HAVE_XSIMD
    const unsigned int best_arch = xsimd::available_architectures().best;

#ifdef Q_PROCESSOR_X86
    bool useVectorization = true;
    bool disableAVXOptimizations = false;

    std::tie(useVectorization, disableAVXOptimizations) =
        vectorizationConfiguration();

    if (!useVectorization) {
        qWarning() << "WARNING: vector instructions disabled by the "
                      "\'amdDisableVectorWorkaround\' option!";
        return 0;
    }

    if (disableAVXOptimizations
        && (xsimd::avx::version() <= best_arch
            || xsimd::avx2::version() <= best_arch)) {
        qWarning() << "WARNING: AVX and AVX2 optimizations are disabled by the "
                      "\'disableAVXOptimizations\' option!";
    }

    /**
     * We use SSE2, SSSE3, SSE4.1, AVX and AVX2+FMA.
     * The rest are integer and string instructions mostly.
     */
    if (!disableAVXOptimizations
        && xsimd::fma3<xsimd::avx2>::version() <= best_arch) {
        return xsimd::fma3<xsimd::avx2>::version();
    } else if (!disableAVXOptimizations && xsimd::avx::version() <= best_arch) {
        return xsimd::avx::version();
    } else if (xsimd::sse4_1::version() <= best_arch) {
        return xsimd::sse4_1::version();
    } else if (xsimd::ssse3::version() <= best_arch) {
        return xsimd::ssse3::version();
    } else if (xsimd::sse2::version() <= best_arch) {
        return xsimd::sse2::version();
    }
#elif XSIMD_WITH_NEON64
    if (xsimd::neon64::version() <= best_arch) {
        return xsimd::neon64::version();
    }
#elif XSIMD_WITH_NEON
    if (xsimd::neon::version() <= best_arch) {
        return xsimd::neon::version();
    }
#endif // XSIMD_WITH_SSE2
#endif // HAVE_XSIMD

    return 0;
}

template<typename S>
struct is_supported_arch {
    is_supported_arch(S &log)
        : l(log)
    {
    }

    template<typename A>
    void operator()(A) const
    {
#ifdef HAVE_XSIMD
        if (A::version() <= xsimd::available_architectures().best) {
            l.append(A::name()).append(" ");
        }
#endif
    }

    S &l;
};

QString KisSupportedArchitectures::supportedInstructionSets()
{
    static const QString archs = []() {
        QString archs;
#ifdef HAVE_XSIMD
        xsimd::all_architectures::for_each(is_supported_arch<QString>{archs});
#endif
        return archs;
    }();
    return archs;
}
