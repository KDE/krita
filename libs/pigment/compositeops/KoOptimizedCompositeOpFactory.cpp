/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "KoOptimizedCompositeOpFactoryPerArch.h" // vc.h must come first
#include "KoOptimizedCompositeOpFactory.h"

#if defined(__clang__)
#pragma GCC diagnostic ignored "-Wundef"
#endif


KoCompositeOp* KoOptimizedCompositeOpFactory::createAlphaDarkenOpHard32(const KoColorSpace *cs)
{
    return createOptimizedClass<
        KoOptimizedCompositeOpFactoryPerArch<
            KoOptimizedCompositeOpAlphaDarkenHard32>>(cs);
}

KoCompositeOp* KoOptimizedCompositeOpFactory::createAlphaDarkenOpCreamy32(const KoColorSpace *cs)
{
    return createOptimizedClass<
        KoOptimizedCompositeOpFactoryPerArch<
            KoOptimizedCompositeOpAlphaDarkenCreamy32>>(cs);
}

KoCompositeOp* KoOptimizedCompositeOpFactory::createOverOp32(const KoColorSpace *cs)
{
    return createOptimizedClass<KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpOver32> >(cs);
}

KoCompositeOp* KoOptimizedCompositeOpFactory::createAlphaDarkenOpHard128(const KoColorSpace *cs)
{
    return createOptimizedClass<
        KoOptimizedCompositeOpFactoryPerArch<
            KoOptimizedCompositeOpAlphaDarkenHard128>>(cs);
}

KoCompositeOp* KoOptimizedCompositeOpFactory::createAlphaDarkenOpCreamy128(const KoColorSpace *cs)
{
    return createOptimizedClass<
        KoOptimizedCompositeOpFactoryPerArch<
            KoOptimizedCompositeOpAlphaDarkenCreamy128>>(cs);
}


KoCompositeOp* KoOptimizedCompositeOpFactory::createOverOp128(const KoColorSpace *cs)
{
    return createOptimizedClass<KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpOver128> >(cs);
}
