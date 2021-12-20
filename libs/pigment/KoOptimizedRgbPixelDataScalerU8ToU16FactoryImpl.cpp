/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KoOptimizedRgbPixelDataScalerU8ToU16FactoryImpl.h"

#include "KoOptimizedRgbPixelDataScalerU8ToU16.h"

template<Vc::Implementation _impl>
KoOptimizedRgbPixelDataScalerU8ToU16Base *KoOptimizedRgbPixelDataScalerU8ToU16FactoryImpl::create(int channelsPerPixel)
{
    return new KoOptimizedRgbPixelDataScalerU8ToU16<_impl>(channelsPerPixel);
}

template KoOptimizedRgbPixelDataScalerU8ToU16Base *KoOptimizedRgbPixelDataScalerU8ToU16FactoryImpl::create<Vc::CurrentImplementation::current()>(int);
