/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; version 2
 * of the License.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef _KIS_HSV_ADJUSTMENT_H_
#define _KIS_HSV_ADJUSTMENT_H_

#include "KoColorTransformationFactory.h"

class KisHSVAdjustmentFactory : public KoColorTransformationFactory
{
public:

    KisHSVAdjustmentFactory();

    QList< QPair< KoID, KoID > > supportedModels() const override;

    KoColorTransformation* createTransformation(const KoColorSpace* colorSpace, QHash<QString, QVariant> parameters) const override;

};

class KisHSVCurveAdjustmentFactory : public KoColorTransformationFactory
{
public:

    KisHSVCurveAdjustmentFactory();

    QList< QPair< KoID, KoID > > supportedModels() const override;

    KoColorTransformation* createTransformation(const KoColorSpace* colorSpace, QHash<QString, QVariant> parameters) const override;

};

namespace KisHSVCurve {
    enum ColorChannel {
        Red = 0,
        Green = 1,
        Blue = 2,
        Alpha = 3,
        AllColors = 4,
        Hue = 5,
        Saturation = 6,
        Value = 7,
        ChannelCount
    };
}

#endif
