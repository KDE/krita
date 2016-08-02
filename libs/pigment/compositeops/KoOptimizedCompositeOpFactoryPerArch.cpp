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

#if !defined _MSC_VER
#pragma GCC diagnostic ignored "-Wundef"
#endif

#include "KoOptimizedCompositeOpFactoryPerArch.h"
#include "KoOptimizedCompositeOpAlphaDarken32.h"
#include "KoOptimizedCompositeOpAlphaDarken128.h"
#include "KoOptimizedCompositeOpOver32.h"
#include "KoOptimizedCompositeOpOver128.h"

#include <QString>
#include "DebugPigment.h"

#include <KoCompositeOpRegistry.h>

#if defined(__clang__)
#pragma GCC diagnostic ignored "-Wlocal-type-template-args"
#endif

template<>
template<>
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpAlphaDarken32>::ReturnType
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpAlphaDarken32>::create<Vc::CurrentImplementation::current()>(ParamType param)
{
    return new KoOptimizedCompositeOpAlphaDarken32<Vc::CurrentImplementation::current()>(param);
}

template<>
template<>
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpOver32>::ReturnType
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpOver32>::create<Vc::CurrentImplementation::current()>(ParamType param)
{
    return new KoOptimizedCompositeOpOver32<Vc::CurrentImplementation::current()>(param);
}

template<>
template<>
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpAlphaDarken128>::ReturnType
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpAlphaDarken128>::create<Vc::CurrentImplementation::current()>(ParamType param)
{
    return new KoOptimizedCompositeOpAlphaDarken128<Vc::CurrentImplementation::current()>(param);
}

template<>
template<>
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpOver128>::ReturnType
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpOver128>::create<Vc::CurrentImplementation::current()>(ParamType param)
{
    return new KoOptimizedCompositeOpOver128<Vc::CurrentImplementation::current()>(param);
}

#define __stringify(_s) #_s
#define stringify(_s) __stringify(_s)

inline void printFeatureSupported(const QString &feature, Vc::Implementation impl)
{
  dbgPigment << "\t" << feature << "\t---\t" << (Vc::isImplementationSupported(impl) ? "yes" : "no");
}

template<>
KoReportCurrentArch::ReturnType
KoReportCurrentArch::create<Vc::CurrentImplementation::current()>(ParamType)
{
    dbgPigment << "Compiled for arch:" << Vc::CurrentImplementation::current();
    dbgPigment << "Features supported:";
    printFeatureSupported("SSE2", Vc::SSE2Impl);
    printFeatureSupported("SSSE3", Vc::SSSE3Impl);
    printFeatureSupported("SSE4.1", Vc::SSE41Impl);
    printFeatureSupported("AVX ", Vc::AVXImpl);
    printFeatureSupported("AVX2 ", Vc::AVX2Impl);
}
