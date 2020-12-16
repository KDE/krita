/*
 *  SPDX-FileCopyrightText: 2013 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-only
*/
#ifndef KIS_DESATURATE_ADJUSTMENT_H
#define KIS_DESATURATE_ADJUSTMENT_H

#include "KoColorTransformationFactory.h"

class KisDesaturateAdjustmentFactory : public KoColorTransformationFactory
{
public:

    KisDesaturateAdjustmentFactory();

    QList< QPair< KoID, KoID > > supportedModels() const override;

    KoColorTransformation* createTransformation(const KoColorSpace* colorSpace, QHash<QString, QVariant> parameters) const override;

};
#endif // KIS_DESATURATE_ADJUSTMENT_H
