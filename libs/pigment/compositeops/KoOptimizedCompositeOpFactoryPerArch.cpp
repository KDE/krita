/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "KoOptimizedCompositeOpFactoryPerArch.h"

#if XSIMD_UNIVERSAL_BUILD_PASS
#include "KoOptimizedCompositeOpAlphaDarken32.h"
#include "KoOptimizedCompositeOpAlphaDarken128.h"
#include "KoOptimizedCompositeOpOver32.h"
#include "KoOptimizedCompositeOpOver128.h"
#include "KoOptimizedCompositeOpCopy128.h"

#include <KoCompositeOpRegistry.h>

template<>
template<>
KoCompositeOp *
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpAlphaDarkenHard32>::
    create<xsimd::current_arch>(const KoColorSpace *param)
{
    return new KoOptimizedCompositeOpAlphaDarkenHard32<xsimd::current_arch>(param);
}

template<>
template<>
KoCompositeOp *KoOptimizedCompositeOpFactoryPerArch<
    KoOptimizedCompositeOpAlphaDarkenCreamy32>::
    create<xsimd::current_arch>(const KoColorSpace *param)
{
    return new KoOptimizedCompositeOpAlphaDarkenCreamy32<xsimd::current_arch>(param);
}

template<>
template<>
KoCompositeOp *
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpOver32>::create<
    xsimd::current_arch>(const KoColorSpace *param)
{
    return new KoOptimizedCompositeOpOver32<xsimd::current_arch>(param);
}

template<>
template<>
KoCompositeOp *
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpAlphaDarkenHard128>::
    create<xsimd::current_arch>(const KoColorSpace *param)
{
    return new KoOptimizedCompositeOpAlphaDarkenHard128<xsimd::current_arch>(param);
}

template<>
template<>
KoCompositeOp *KoOptimizedCompositeOpFactoryPerArch<
    KoOptimizedCompositeOpAlphaDarkenCreamy128>::
    create<xsimd::current_arch>(const KoColorSpace *param)
{
    return new KoOptimizedCompositeOpAlphaDarkenCreamy128<xsimd::current_arch>(param);
}

template<>
template<>
KoCompositeOp *
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpOver128>::create<
    xsimd::current_arch>(const KoColorSpace *param)
{
    return new KoOptimizedCompositeOpOver128<xsimd::current_arch>(param);
}

template<>
template<>
KoCompositeOp *
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpOverU64>::create<
    xsimd::current_arch>(const KoColorSpace *param)
{
    return new KoOptimizedCompositeOpOverU64<xsimd::current_arch>(param);
}

template<>
template<>
KoCompositeOp *
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpCopy128>::create<
    xsimd::current_arch>(const KoColorSpace *param)
{
    return new KoOptimizedCompositeOpCopy128<xsimd::current_arch>(param);
}

template<>
template<>
KoCompositeOp *
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpCopyU64>::create<
    xsimd::current_arch>(const KoColorSpace *param)
{
    return new KoOptimizedCompositeOpCopyU64<xsimd::current_arch>(param);
}

template<>
template<>
KoCompositeOp *
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpCopy32>::create<
    xsimd::current_arch>(const KoColorSpace *param)
{
    return new KoOptimizedCompositeOpCopy32<xsimd::current_arch>(param);
}

template<>
template<>
KoCompositeOp *
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpAlphaDarkenHardU64>::
    create<xsimd::current_arch>(const KoColorSpace *param)
{
    return new KoOptimizedCompositeOpAlphaDarkenHardU64<xsimd::current_arch>(param);
}

template<>
template<>
KoCompositeOp *KoOptimizedCompositeOpFactoryPerArch<
    KoOptimizedCompositeOpAlphaDarkenCreamyU64>::
    create<xsimd::current_arch>(const KoColorSpace *param)
{
    return new KoOptimizedCompositeOpAlphaDarkenCreamyU64<xsimd::current_arch>(param);
}

#endif // XSIMD_UNIVERSAL_BUILD_PASS
