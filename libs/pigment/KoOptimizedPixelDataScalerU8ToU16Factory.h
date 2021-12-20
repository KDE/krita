/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KoOptimizedPixelDataScalerU8ToU16FACTORY_H
#define KoOptimizedPixelDataScalerU8ToU16FACTORY_H

#include "KoOptimizedPixelDataScalerU8ToU16Base.h"

/**
 * \see KoOptimizedPixelDataScalerU8ToU16Base
 */
class KRITAPIGMENT_EXPORT KoOptimizedPixelDataScalerU8ToU16Factory
{
public:
    static KoOptimizedPixelDataScalerU8ToU16Base* createRgbaScaler();
    static KoOptimizedPixelDataScalerU8ToU16Base* createCmykaScaler();

};


#endif // KoOptimizedPixelDataScalerU8ToU16FACTORY_H
