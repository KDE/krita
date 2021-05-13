/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KOOPTIMIZEDRGBPIXELDATASCALERU8TOU16FACTORYIMPL_H
#define KOOPTIMIZEDRGBPIXELDATASCALERU8TOU16FACTORYIMPL_H

#include <KoOptimizedRgbPixelDataScalerU8ToU16Base.h>
#include <KoVcMultiArchBuildSupport.h>

class KRITAPIGMENT_EXPORT KoOptimizedRgbPixelDataScalerU8ToU16FactoryImpl
{
public:
    typedef int ParamType;
    typedef KoOptimizedRgbPixelDataScalerU8ToU16Base* ReturnType;

    template<Vc::Implementation _impl>
    static KoOptimizedRgbPixelDataScalerU8ToU16Base* create(int);
};

#endif // KOOPTIMIZEDRGBPIXELDATASCALERU8TOU16FACTORYIMPL_H
