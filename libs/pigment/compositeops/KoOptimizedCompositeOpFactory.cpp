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
