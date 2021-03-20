/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "KoOptimizedCompositeOpFactoryPerArch.h"

#include "KoColorSpaceTraits.h"
#include "KoCompositeOpAlphaDarken.h"
#include "KoAlphaDarkenParamsWrapper.h"
#include "KoCompositeOpOver.h"

template<>
template<>
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpAlphaDarkenHard32>::ReturnType
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpAlphaDarkenHard32>::create<Vc::ScalarImpl>(ParamType param)
{
    return new KoCompositeOpAlphaDarken<KoBgrU8Traits, KoAlphaDarkenParamsWrapperHard>(param);
}

template<>
template<>
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpAlphaDarkenCreamy32>::ReturnType
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpAlphaDarkenCreamy32>::create<Vc::ScalarImpl>(ParamType param)
{
    return new KoCompositeOpAlphaDarken<KoBgrU8Traits, KoAlphaDarkenParamsWrapperCreamy>(param);
}

template<>
template<>
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpOver32>::ReturnType
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpOver32>::create<Vc::ScalarImpl>(ParamType param)
{
    return new KoCompositeOpOver<KoBgrU8Traits>(param);
}

template<>
template<>
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpAlphaDarkenHard128>::ReturnType
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpAlphaDarkenHard128>::create<Vc::ScalarImpl>(ParamType param)
{
    return new KoCompositeOpAlphaDarken<KoRgbF32Traits, KoAlphaDarkenParamsWrapperHard>(param);
}

template<>
template<>
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpAlphaDarkenCreamy128>::ReturnType
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpAlphaDarkenCreamy128>::create<Vc::ScalarImpl>(ParamType param)
{
    return new KoCompositeOpAlphaDarken<KoRgbF32Traits, KoAlphaDarkenParamsWrapperCreamy>(param);
}


template<>
template<>
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpOver128>::ReturnType
KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpOver128>::create<Vc::ScalarImpl>(ParamType param)
{
    return new KoCompositeOpOver<KoRgbF32Traits>(param);
}
