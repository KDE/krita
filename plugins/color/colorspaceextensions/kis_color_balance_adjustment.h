/*
 *  SPDX-FileCopyrightText: 2013 Sahil Nagpal <nagpal.sahil01@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef _KIS_COLOR_BALANCE_ADJUSTMENT_H_
#define _KIS_COLOR_BALANCE_ADJUSTMENT_H_

#include "KoColorTransformationFactory.h"

class KisColorBalanceAdjustmentFactory : public KoColorTransformationFactory
{
public:

   KisColorBalanceAdjustmentFactory();

   QList< QPair< KoID, KoID > > supportedModels() const override;

   KoColorTransformation* createTransformation(const KoColorSpace* colorSpace, QHash<QString, QVariant> parameters) const override;

};

class KisColorBalanceMath
{
public:

    KisColorBalanceMath();

    float colorBalanceTransform(float value, float lightness, float shadows, float midtones, float highlights);
};

#endif
