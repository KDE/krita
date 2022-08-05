/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KoOptimizedPixelDataScalerU8ToU16FactoryImpl.h"

#if XSIMD_UNIVERSAL_BUILD_PASS
#include "KoOptimizedPixelDataScalerU8ToU16.h"

template<>
KoOptimizedPixelDataScalerU8ToU16Base *
KoOptimizedPixelDataScalerU8ToU16FactoryImpl::create<xsimd::current_arch>(
    int channelsPerPixel)
{
    return new KoOptimizedPixelDataScalerU8ToU16<xsimd::current_arch>(
        channelsPerPixel);
}

#endif // XSIMD_UNIVERSAL_BUILD_PASS
