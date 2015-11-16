/*
 *  Copyright (c) 2012 Dmitry Kazakov <dimula73@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __KOVCMULTIARCHBUILDSUPPORT_H
#define __KOVCMULTIARCHBUILDSUPPORT_H

#include "config-vc.h"

#ifdef HAVE_VC

#if defined(__clang__)

#pragma GCC diagnostic ignored "-Wlocal-type-template-args"
#endif

#include <Vc/Vc>
#include <Vc/support.h>

#else /* HAVE_VC */

namespace Vc {
    typedef enum {ScalarImpl} Implementation;
}

#define VC_IMPL ::Vc::ScalarImpl

#ifdef DO_PACKAGERS_BUILD
#ifdef __GNUC__
#warning "Packagers build is not available without the presence of Vc library. Disabling."
#endif
#undef DO_PACKAGERS_BUILD
#endif

#endif /* HAVE_VC */


#ifdef DO_PACKAGERS_BUILD

#include <QDebug>
#include <ksharedconfig.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kconfiggroup.h>

template<class FactoryType>
typename FactoryType::ReturnType
createOptimizedClass(typename FactoryType::ParamType param)
{
    static bool isConfigInitialized = false;
    static bool useVectorization = true;

    if (!isConfigInitialized) {
        KConfigGroup cfg = KGlobal::config()->group("");
        useVectorization = !cfg.readEntry("amdDisableVectorWorkaround", false);
        isConfigInitialized = true;
    }

    if (!useVectorization) {
        qWarning() << "WARNING: vector instructions disabled by \'amdDisableVectorWorkaround\' option!";
        return FactoryType::template create<Vc::ScalarImpl>(param);
    }

    /**
     * We use SSE2, SSSE3, SSE4.1, AVX and AVX2.
     * The rest are integer and string instructions mostly.
     *
     * TODO: Add FMA3/4 when it is adopted by Vc
     */
#if VC_VERSION_NUMBER >= VC_VERSION_CHECK(0, 8, 0)
    if (Vc::isImplementationSupported(Vc::AVX2Impl)) {
        return FactoryType::template create<Vc::AVX2Impl>(param);
    } else
#endif
    if (Vc::isImplementationSupported(Vc::AVXImpl)) {
        return FactoryType::template create<Vc::AVXImpl>(param);
    } else if (Vc::isImplementationSupported(Vc::SSE41Impl)) {
        return FactoryType::template create<Vc::SSE41Impl>(param);
    } else if (Vc::isImplementationSupported(Vc::SSSE3Impl)) {
        return FactoryType::template create<Vc::SSSE3Impl>(param);
    } else if (Vc::isImplementationSupported(Vc::SSE2Impl)) {
        return FactoryType::template create<Vc::SSE2Impl>(param);
    } else {
        return FactoryType::template create<Vc::ScalarImpl>(param);
    }
}

#else /* DO_PACKAGERS_BUILD */

/**
 * When doing not a packager's build we have one architecture only,
 * so the factory methods are simplified
 */

template<class FactoryType>
typename FactoryType::ReturnType
createOptimizedClass(typename FactoryType::ParamType param)
{
    return FactoryType::template create<VC_IMPL>(param);
}

#endif /* DO_PACKAGERS_BUILD */

#endif /* __KOVCMULTIARCHBUILDSUPPORT_H */
