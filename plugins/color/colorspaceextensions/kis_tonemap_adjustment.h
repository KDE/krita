/*
 *  SPDX-FileCopyrightText: 2026 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-only
 */

#ifndef KIS_TONEMAP_ADJUSTMENT_H
#define KIS_TONEMAP_ADJUSTMENT_H

#include "KoColorTransformationFactory.h"

class KisToneMapAdjustmentFactory : public KoColorTransformationFactory
{
public:
    KisToneMapAdjustmentFactory();

    QList< QPair< KoID, KoID > > supportedModels() const override;

    KoColorTransformation* createTransformation(const KoColorSpace* colorSpace, QHash<QString, QVariant> parameters) const override;
};

#endif // KIS_TONEMAP_ADJUSTMENT_H
