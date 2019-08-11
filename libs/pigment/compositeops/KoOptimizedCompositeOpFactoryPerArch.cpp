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
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpAlphaDarkenHard32>::ReturnType
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpAlphaDarkenHard32>::create<Vc::CurrentImplementation::current()>(ParamType param)
{
    return new KoOptimizedCompositeOpAlphaDarkenHard32<Vc::CurrentImplementation::current()>(param);
}


template<>
template<>
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpAlphaDarkenCreamy32>::ReturnType
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpAlphaDarkenCreamy32>::create<Vc::CurrentImplementation::current()>(ParamType param)
{
    return new KoOptimizedCompositeOpAlphaDarkenCreamy32<Vc::CurrentImplementation::current()>(param);
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
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpAlphaDarkenHard128>::ReturnType
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpAlphaDarkenHard128>::create<Vc::CurrentImplementation::current()>(ParamType param)
{
    return new KoOptimizedCompositeOpAlphaDarkenHard128<Vc::CurrentImplementation::current()>(param);
}

template<>
template<>
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpAlphaDarkenCreamy128>::ReturnType
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpAlphaDarkenCreamy128>::create<Vc::CurrentImplementation::current()>(ParamType param)
{
    return new KoOptimizedCompositeOpAlphaDarkenCreamy128<Vc::CurrentImplementation::current()>(param);
}


template<>
template<>
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpOver128>::ReturnType
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpOver128>::create<Vc::CurrentImplementation::current()>(ParamType param)
{
    return new KoOptimizedCompositeOpOver128<Vc::CurrentImplementation::current()>(param);
}
