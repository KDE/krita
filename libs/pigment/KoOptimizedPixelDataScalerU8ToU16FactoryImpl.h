/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KoOptimizedPixelDataScalerU8ToU16FACTORYIMPL_H
#define KoOptimizedPixelDataScalerU8ToU16FACTORYIMPL_H

#include <KoOptimizedPixelDataScalerU8ToU16Base.h>
#include <KoVcMultiArchBuildSupport.h>

class KRITAPIGMENT_EXPORT KoOptimizedPixelDataScalerU8ToU16FactoryImpl
{
public:
    typedef int ParamType;
    typedef KoOptimizedPixelDataScalerU8ToU16Base* ReturnType;

    template<Vc::Implementation _impl>
    static KoOptimizedPixelDataScalerU8ToU16Base* create(int);
};

#endif // KoOptimizedPixelDataScalerU8ToU16FACTORYIMPL_H
