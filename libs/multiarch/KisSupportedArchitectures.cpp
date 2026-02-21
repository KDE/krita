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

#include <KoMultiArchBuildSupport.h>

QString KisSupportedArchitectures::baseArchName()
{
    return xsimd::current_arch::name();
}

namespace detail
{
struct ArchToStringBase
{
    virtual ~ArchToStringBase() = default;
    virtual QString name() const = 0;
};

template <typename arch>
struct ArchToString : ArchToStringBase {
    QString name() const override {
        return arch::name();
    }
};

#if XSIMD_VERSION_MAJOR < 13
template <>
struct ArchToString<xsimd::generic> : ArchToStringBase {
    QString name() const override {
        return "generic";
    }
};
#endif

struct ArchToStringFactory {
    template <typename arch>
    static ArchToStringBase* create() {
        return new ArchToString<arch>();
    }
};

}

QString KisSupportedArchitectures::bestArchName()
{
    detail::ArchToStringBase *archDetector = createOptimizedClass<detail::ArchToStringFactory>();
    return archDetector->name();
}

#if (XSIMD_VERSION_MAJOR >= 14) \
    || (XSIMD_VERSION_MAJOR == 13 && XSIMD_VERSION_MINOR >=1) \
    || defined(XSIMD_HAS_ARCH_LIST_FIX_PR1032)
#define XSIMD_SUPPORTS_NEW_ARCH_DETECTION
#elif XSIMD_VERSION_MAJOR < 13
#define XSIMD_SUPPORTS_OLD_ARCH_DETECTION
#endif

template<typename S>
struct is_supported_arch {
    is_supported_arch(S &log)
        : l(log)
    {
    }

    template<typename A>
    void operator()(A arch) const
    {
#ifdef XSIMD_SUPPORTS_NEW_ARCH_DETECTION
        if (xsimd::available_architectures().has(arch)) {
            l.append(A::name()).append(" ");
        }
#else
        Q_UNUSED(arch)
#endif
    }

    S &l;
};

QString KisSupportedArchitectures::supportedInstructionSets()
{
    static const QString archs = []() {
        QString archs;
#ifdef XSIMD_SUPPORTS_NEW_ARCH_DETECTION
        xsimd::all_architectures::for_each(is_supported_arch<QString>{archs});
#elif defined XSIMD_SUPPORTS_OLD_ARCH_DETECTION
        QStringList archsList;
        auto available = xsimd::available_architectures();
#define CHECK_ARCH(arch) if (available.arch) archsList << #arch
        CHECK_ARCH(sse2);
        CHECK_ARCH(sse3);
        CHECK_ARCH(ssse3);
        CHECK_ARCH(sse4_1);
        CHECK_ARCH(sse4_2);
        CHECK_ARCH(sse4a);
        CHECK_ARCH(fma3_sse);
        CHECK_ARCH(fma4);
        CHECK_ARCH(xop);
        CHECK_ARCH(avx);
        CHECK_ARCH(fma3_avx);
        CHECK_ARCH(avx2);
        CHECK_ARCH(fma3_avx2);
        CHECK_ARCH(avx512f);
        CHECK_ARCH(avx512cd);
        CHECK_ARCH(avx512dq);
        CHECK_ARCH(avx512bw);
        CHECK_ARCH(neon);
        CHECK_ARCH(neon64);
#undef CHECK_ARCH
        archs = archsList.join(' ');
#endif
        return archs;
    }();
    return archs;
}
