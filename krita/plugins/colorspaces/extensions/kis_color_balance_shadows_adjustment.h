/*
 *  Copyright (c) 2013 Sahil Nagpal <nagpal.sahil01@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; version 2
 * of the License, or (at your option) any later version.
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

#ifndef _KIS_COLOR_BALANCE_SHADOWS_ADJUSTMENT_H_
#define _KIS_COLOR_BALANCE_SHADOWS_ADJUSTMENT_H_

#include "KoColorTransformationFactory.h"

class KisColorBalanceShadowsAdjustmentFactory : public KoColorTransformationFactory
{
public:

   KisColorBalanceShadowsAdjustmentFactory();

   virtual QList< QPair< KoID, KoID > > supportedModels() const;

   virtual KoColorTransformation* createTransformation(const KoColorSpace* colorSpace, QHash<QString, QVariant> parameters) const;

};

#endif

