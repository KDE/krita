/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "KoOptimizedCompositeOpFactoryPerArch.h"
#include "KoOptimizedCompositeOpAlphaDarken32.h"
#include "KoOptimizedCompositeOpAlphaDarken128.h"
#include "KoOptimizedCompositeOpOver32.h"
#include "KoOptimizedCompositeOpOver128.h"
#include "KoOptimizedCompositeOpCopy128.h"

#include <QString>
#include "DebugPigment.h"

#include <KoCompositeOpRegistry.h>

template<>
template<>
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpAlphaDarkenHard32>::ReturnType
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpAlphaDarkenHard32>::create<xsimd::current_arch>(ParamType param)
{
    return new KoOptimizedCompositeOpAlphaDarkenHard32<xsimd::current_arch>(param);
}


template<>
template<>
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpAlphaDarkenCreamy32>::ReturnType
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpAlphaDarkenCreamy32>::create<xsimd::current_arch>(ParamType param)
{
    return new KoOptimizedCompositeOpAlphaDarkenCreamy32<xsimd::current_arch>(param);
}

template<>
template<>
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpOver32>::ReturnType
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpOver32>::create<xsimd::current_arch>(ParamType param)
{
    return new KoOptimizedCompositeOpOver32<xsimd::current_arch>(param);
}

template<>
template<>
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpAlphaDarkenHard128>::ReturnType
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpAlphaDarkenHard128>::create<xsimd::current_arch>(ParamType param)
{
    return new KoOptimizedCompositeOpAlphaDarkenHard128<xsimd::current_arch>(param);
}

template<>
template<>
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpAlphaDarkenCreamy128>::ReturnType
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpAlphaDarkenCreamy128>::create<xsimd::current_arch>(ParamType param)
{
    return new KoOptimizedCompositeOpAlphaDarkenCreamy128<xsimd::current_arch>(param);
}


template<>
template<>
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpOver128>::ReturnType
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpOver128>::create<xsimd::current_arch>(ParamType param)
{
    return new KoOptimizedCompositeOpOver128<xsimd::current_arch>(param);
}

template<>
template<>
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpOverU64>::ReturnType
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpOverU64>::create<xsimd::current_arch>(ParamType param)
{
    return new KoOptimizedCompositeOpOverU64<xsimd::current_arch>(param);
}

template<>
template<>
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpCopy128>::ReturnType
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpCopy128>::create<xsimd::current_arch>(ParamType param)
{
    return new KoOptimizedCompositeOpCopy128<xsimd::current_arch>(param);
}

template<>
template<>
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpCopyU64>::ReturnType
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpCopyU64>::create<xsimd::current_arch>(ParamType param)
{
    return new KoOptimizedCompositeOpCopyU64<xsimd::current_arch>(param);
}

template<>
template<>
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpCopy32>::ReturnType
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpCopy32>::create<xsimd::current_arch>(ParamType param)
{
    return new KoOptimizedCompositeOpCopy32<xsimd::current_arch>(param);
}

template<>
template<>
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpAlphaDarkenHardU64>::ReturnType
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpAlphaDarkenHardU64>::create<xsimd::current_arch>(ParamType param)
{
    return new KoOptimizedCompositeOpAlphaDarkenHardU64<xsimd::current_arch>(param);
}

template<>
template<>
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpAlphaDarkenCreamyU64>::ReturnType
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpAlphaDarkenCreamyU64>::create<xsimd::current_arch>(ParamType param)
{
    return new KoOptimizedCompositeOpAlphaDarkenCreamyU64<xsimd::current_arch>(param);
}
