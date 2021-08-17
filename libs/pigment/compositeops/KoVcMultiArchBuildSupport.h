/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef __KOVCMULTIARCHBUILDSUPPORT_H
#define __KOVCMULTIARCHBUILDSUPPORT_H

#include "config-vc.h"

#ifdef HAVE_VC

#if defined(__clang__)

#pragma GCC diagnostic ignored "-Wlocal-type-template-args"
#endif

#if defined _MSC_VER
// Lets shut up the "possible loss of data" and "forcing value to bool 'true' or 'false'
#pragma warning ( push )
#pragma warning ( disable : 4146 ) // avx/detail.h
#pragma warning ( disable : 4244 )
#pragma warning ( disable : 4267 ) // interleavedmemory
#pragma warning ( disable : 4800 )
#endif
#include <Vc/global.h>
#include <Vc/Vc>
#include <Vc/support.h>
#if defined _MSC_VER
#pragma warning ( pop )
#endif

#else /* HAVE_VC */

namespace Vc {
    enum Implementation /*: std::uint_least32_t*/ {
            ScalarImpl,
    };
    class CurrentImplementation {
        public:
            static constexpr Implementation current()
            {
                return static_cast<Implementation>(ScalarImpl);
            }
    };
}


#endif /* HAVE_VC */


#include <QDebug>
#include <ksharedconfig.h>
#include <kconfig.h>
#include <kconfiggroup.h>

template<class FactoryType>
typename FactoryType::ReturnType
createOptimizedClass(typename FactoryType::ParamType param)
{
    static bool isConfigInitialized = false;
    static bool useVectorization = true;
    static bool disableAVXOptimizations = false;

    if (!isConfigInitialized) {
        KConfigGroup cfg = KSharedConfig::openConfig()->group("");
        // use the old key name for compatibility
        useVectorization = !cfg.readEntry("amdDisableVectorWorkaround", false);
        disableAVXOptimizations = cfg.readEntry("disableAVXOptimizations", false);
        isConfigInitialized = true;
    }

    if (!useVectorization) {
        qWarning() << "WARNING: vector instructions disabled by the \'amdDisableVectorWorkaround\' option!";
        return FactoryType::template create<Vc::ScalarImpl>(param);
    }

#ifdef HAVE_VC
    if (disableAVXOptimizations &&
        (Vc::isImplementationSupported(Vc::AVXImpl) ||
         Vc::isImplementationSupported(Vc::AVX2Impl))) {
        qWarning() << "WARNING: AVX and AVX2 optimizations are disabled by the \'disableAVXOptimizations\' option!";
    }

    /**
     * We use SSE2, SSSE3, SSE4.1, AVX and AVX2.
     * The rest are integer and string instructions mostly.
     *
     * TODO: Add FMA3/4 when it is adopted by Vc
     */
    if (!disableAVXOptimizations && Vc::isImplementationSupported(Vc::AVX2Impl)) {
        return FactoryType::template create<Vc::AVX2Impl>(param);
    } else if (!disableAVXOptimizations && Vc::isImplementationSupported(Vc::AVXImpl)) {
        return FactoryType::template create<Vc::AVXImpl>(param);
    } else if (Vc::isImplementationSupported(Vc::SSE41Impl)) {
        return FactoryType::template create<Vc::SSE41Impl>(param);
    } else if (Vc::isImplementationSupported(Vc::SSSE3Impl)) {
        return FactoryType::template create<Vc::SSSE3Impl>(param);
    } else if (Vc::isImplementationSupported(Vc::SSE2Impl)) {
        return FactoryType::template create<Vc::SSE2Impl>(param);
    } else {
#endif
        (void)disableAVXOptimizations;
        return FactoryType::template create<Vc::ScalarImpl>(param);
#ifdef HAVE_VC
    }
#endif

}

template<class FactoryType>
typename FactoryType::ReturnType
createOptimizedClass(typename FactoryType::ParamType param, bool forceScalarImplemetation)
{
    if(forceScalarImplemetation){
        return FactoryType::template create<Vc::ScalarImpl>(param);
    }
    return createOptimizedClass<FactoryType>(param);
}

#endif /* __KOVCMULTIARCHBUILDSUPPORT_H */
