/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KoOptimizedPixelDataScalerU8ToU16Factory.h"

#include "KoOptimizedPixelDataScalerU8ToU16FactoryImpl.h"


KoOptimizedPixelDataScalerU8ToU16Base *KoOptimizedPixelDataScalerU8ToU16Factory::createRgbaScaler()
{
    return createOptimizedClass<
            KoOptimizedPixelDataScalerU8ToU16FactoryImpl>(4);
}

KoOptimizedPixelDataScalerU8ToU16Base *KoOptimizedPixelDataScalerU8ToU16Factory::createCmykaScaler()
{
    return createOptimizedClass<
            KoOptimizedPixelDataScalerU8ToU16FactoryImpl>(5);
}
