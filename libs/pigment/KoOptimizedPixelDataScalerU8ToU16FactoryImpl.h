/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KoOptimizedPixelDataScalerU8ToU16FACTORYIMPL_H
#define KoOptimizedPixelDataScalerU8ToU16FACTORYIMPL_H

#include <KoOptimizedPixelDataScalerU8ToU16Base.h>
#include <KoMultiArchBuildSupport.h>

class KRITAPIGMENT_EXPORT KoOptimizedPixelDataScalerU8ToU16FactoryImpl
{
public:
    template<typename _impl>
    static KoOptimizedPixelDataScalerU8ToU16Base* create(int);
};

#endif // KoOptimizedPixelDataScalerU8ToU16FACTORYIMPL_H
