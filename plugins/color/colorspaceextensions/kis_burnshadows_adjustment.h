/*
 *  SPDX-FileCopyrightText: 2013 Sahil Nagpal <nagpal.sahil@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef _KIS_BURN_SHADOWS_ADJUSTMENT_H_
#define _KIS_BURN_SHADOWS_ADJUSTMENT_H_

#include "KoColorTransformationFactory.h"

class KisBurnShadowsAdjustmentFactory : public KoColorTransformationFactory
{
public:

    KisBurnShadowsAdjustmentFactory();

    QList< QPair< KoID, KoID > > supportedModels() const override;

    KoColorTransformation* createTransformation(const KoColorSpace* colorSpace, QHash<QString, QVariant> parameters) const override;

};

#endif
