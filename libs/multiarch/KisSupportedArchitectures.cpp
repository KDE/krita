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


struct ArchToStringFactory {
    template <typename arch>
    static ArchToStringBase* create() {
        return new ArchToString<arch>();
    }
};

}

QString KisSupportedArchitectures::bestArchName()
{
    QString bestArchName = "<unavailable>";

#ifdef HAVE_XSIMD
    detail::ArchToStringBase *archDetector =
        createOptimizedClass<detail::ArchToStringFactory>();
    bestArchName = archDetector->name();
#endif

    return bestArchName;
}

template<typename S>
struct is_supported_arch {
    is_supported_arch(S &log)
        : l(log)
    {
    }

    template<typename A>
    void operator()(A arch) const
    {
#ifdef HAVE_XSIMD
        if (xsimd::available_architectures().has(arch)) {
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
