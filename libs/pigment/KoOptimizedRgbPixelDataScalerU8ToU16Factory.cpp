/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KoOptimizedRgbPixelDataScalerU8ToU16Factory.h"

#include "KoOptimizedRgbPixelDataScalerU8ToU16FactoryImpl.h"


KoOptimizedRgbPixelDataScalerU8ToU16Base *KoOptimizedRgbPixelDataScalerU8ToU16Factory::create()
{
    return createOptimizedClass<
            KoOptimizedRgbPixelDataScalerU8ToU16FactoryImpl>(4);
}

KoOptimizedRgbPixelDataScalerU8ToU16Base *KoOptimizedCmykPixelDataScalerU8ToU16Factory::create()
{
    return createOptimizedClass<
            KoOptimizedRgbPixelDataScalerU8ToU16FactoryImpl>(5);
}
