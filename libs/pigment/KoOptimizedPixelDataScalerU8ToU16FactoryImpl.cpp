/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KoOptimizedPixelDataScalerU8ToU16FactoryImpl.h"

#include "KoOptimizedPixelDataScalerU8ToU16.h"

template<Vc::Implementation _impl>
KoOptimizedPixelDataScalerU8ToU16Base *KoOptimizedPixelDataScalerU8ToU16FactoryImpl::create(int channelsPerPixel)
{
    return new KoOptimizedPixelDataScalerU8ToU16<_impl>(channelsPerPixel);
}

template KoOptimizedPixelDataScalerU8ToU16Base *KoOptimizedPixelDataScalerU8ToU16FactoryImpl::create<Vc::CurrentImplementation::current()>(int);
