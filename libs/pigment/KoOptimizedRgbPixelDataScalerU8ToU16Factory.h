/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KOOPTIMIZEDRGBPIXELDATASCALERU8TOU16FACTORY_H
#define KOOPTIMIZEDRGBPIXELDATASCALERU8TOU16FACTORY_H

#include "KoOptimizedRgbPixelDataScalerU8ToU16Base.h"

/**
 * \see KoOptimizedRgbPixelDataScalerU8ToU16Base
 */
class KRITAPIGMENT_EXPORT KoOptimizedRgbPixelDataScalerU8ToU16Factory
{
public:
    static KoOptimizedRgbPixelDataScalerU8ToU16Base* create();

};

class KRITAPIGMENT_EXPORT KoOptimizedCmykPixelDataScalerU8ToU16Factory
{
public:
    static KoOptimizedRgbPixelDataScalerU8ToU16Base* create();

};


#endif // KOOPTIMIZEDRGBPIXELDATASCALERU8TOU16FACTORY_H
