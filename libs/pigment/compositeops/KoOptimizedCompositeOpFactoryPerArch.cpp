/*
 *  Copyright (c) 2012 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "KoOptimizedCompositeOpFactoryPerArch.h"

#include <QDebug>

#include "KoOptimizedCompositeOpAlphaDarken32.h"
#include "KoOptimizedCompositeOpOver32.h"

template<>
template<>
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpAlphaDarken32>::ReturnType
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpAlphaDarken32>::create<VC_IMPL>(ParamType param)
{
    return new KoOptimizedCompositeOpAlphaDarken32<VC_IMPL>(param);
}

template<>
template<>
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpOver32>::ReturnType
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpOver32>::create<VC_IMPL>(ParamType param)
{
    return new KoOptimizedCompositeOpOver32<VC_IMPL>(param);
}

#define __stringify(_s) #_s
#define stringify(_s) __stringify(_s)

inline void printFeatureSupported(const QString &feature, Vc::Implementation impl)
{
    qDebug() << "\t" << feature << "\t---\t" << (Vc::isImplementationSupported(impl) ? "yes" : "no");
}

template<>
KoReportCurrentArch::ReturnType
KoReportCurrentArch::create<VC_IMPL>(ParamType)
{
    qDebug() << "Compiled for arch:" << stringify(VC_IMPL);
    qDebug() << "Features supported:";
    printFeatureSupported("SSE2", Vc::SSE2Impl);
    printFeatureSupported("SSSE3", Vc::SSSE3Impl);
    printFeatureSupported("SSE4.1", Vc::SSE41Impl);
    printFeatureSupported("AVX ", Vc::AVXImpl);
#if VC_VERSION_NUMBER >= VC_VERSION_CHECK(0, 8, 0)
    printFeatureSupported("AVX2 ", Vc::AVX2Impl);
#endif
}
