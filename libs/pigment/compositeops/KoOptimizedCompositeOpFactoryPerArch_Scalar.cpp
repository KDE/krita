/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "KoOptimizedCompositeOpFactoryPerArch.h"

#include "KoColorSpaceTraits.h"
#include "KoCompositeOpAlphaDarken.h"
#include "KoAlphaDarkenParamsWrapper.h"
#include "KoCompositeOpOver.h"
#include "KoCompositeOpCopy2.h"

template<>
template<>
KoCompositeOp *
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpAlphaDarkenHard32>::
    create<xsimd::generic>(const KoColorSpace *param)
{
    return new KoCompositeOpAlphaDarken<KoBgrU8Traits, KoAlphaDarkenParamsWrapperHard>(param);
}

template<>
template<>
KoCompositeOp *KoOptimizedCompositeOpFactoryPerArch<
    KoOptimizedCompositeOpAlphaDarkenCreamy32>::
    create<xsimd::generic>(const KoColorSpace *param)
{
    return new KoCompositeOpAlphaDarken<KoBgrU8Traits, KoAlphaDarkenParamsWrapperCreamy>(param);
}

template<>
template<>
KoCompositeOp *
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpOver32>::create<
    xsimd::generic>(const KoColorSpace *param)
{
    return new KoCompositeOpOver<KoBgrU8Traits>(param);
}

template<>
template<>
KoCompositeOp *
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpAlphaDarkenHard128>::
    create<xsimd::generic>(const KoColorSpace *param)
{
    return new KoCompositeOpAlphaDarken<KoRgbF32Traits, KoAlphaDarkenParamsWrapperHard>(param);
}

template<>
template<>
KoCompositeOp *KoOptimizedCompositeOpFactoryPerArch<
    KoOptimizedCompositeOpAlphaDarkenCreamy128>::
    create<xsimd::generic>(const KoColorSpace *param)
{
    return new KoCompositeOpAlphaDarken<KoRgbF32Traits, KoAlphaDarkenParamsWrapperCreamy>(param);
}

template<>
template<>
KoCompositeOp *
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpOver128>::create<
    xsimd::generic>(const KoColorSpace *param)
{
    return new KoCompositeOpOver<KoRgbF32Traits>(param);
}

template<>
template<>
KoCompositeOp *
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpOverU64>::create<
    xsimd::generic>(const KoColorSpace *param)
{
    return new KoCompositeOpOver<KoBgrU16Traits>(param);
}

template<>
template<>
KoCompositeOp *
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpCopy128>::create<
    xsimd::generic>(const KoColorSpace *param)
{
    return new KoCompositeOpCopy2<KoRgbF32Traits>(param);
}

template<>
template<>
KoCompositeOp *
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpCopyU64>::create<
    xsimd::generic>(const KoColorSpace *param)
{
    return new KoCompositeOpCopy2<KoBgrU16Traits>(param);
}

template<>
template<>
KoCompositeOp *
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpCopy32>::create<
    xsimd::generic>(const KoColorSpace *param)
{
    return new KoCompositeOpCopy2<KoBgrU8Traits>(param);
}

template<>
template<>
KoCompositeOp *
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpAlphaDarkenHardU64>::
    create<xsimd::generic>(const KoColorSpace *param)
{
    return new KoCompositeOpAlphaDarken<KoBgrU16Traits, KoAlphaDarkenParamsWrapperHard>(param);
}

template<>
template<>
KoCompositeOp *KoOptimizedCompositeOpFactoryPerArch<
    KoOptimizedCompositeOpAlphaDarkenCreamyU64>::
    create<xsimd::generic>(const KoColorSpace *param)
{
    return new KoCompositeOpAlphaDarken<KoBgrU16Traits, KoAlphaDarkenParamsWrapperCreamy>(param);
}

