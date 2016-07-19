/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __KIS_DERIVED_RESOURCES_H
#define __KIS_DERIVED_RESOURCES_H

#include "KoDerivedResourceConverter.h"


class KisCompositeOpResourceConverter : public KoDerivedResourceConverter
{
public:
    KisCompositeOpResourceConverter();

    QVariant fromSource(const QVariant &value);
    QVariant toSource(const QVariant &value, const QVariant &sourceValue);
};

class KisEffectiveCompositeOpResourceConverter : public KoDerivedResourceConverter
{
public:
    KisEffectiveCompositeOpResourceConverter();

    QVariant fromSource(const QVariant &value);
    QVariant toSource(const QVariant &value, const QVariant &sourceValue);
};

class KisOpacityResourceConverter : public KoDerivedResourceConverter
{
public:
    KisOpacityResourceConverter();

    QVariant fromSource(const QVariant &value);
    QVariant toSource(const QVariant &value, const QVariant &sourceValue);
};

class KisLodAvailabilityResourceConverter : public KoDerivedResourceConverter
{
public:
    KisLodAvailabilityResourceConverter();

    QVariant fromSource(const QVariant &value);
    QVariant toSource(const QVariant &value, const QVariant &sourceValue);
};

class KisEraserModeResourceConverter : public KoDerivedResourceConverter
{
public:
    KisEraserModeResourceConverter();

    QVariant fromSource(const QVariant &value);
    QVariant toSource(const QVariant &value, const QVariant &sourceValue);
};



#endif /* __KIS_DERIVED_RESOURCES_H */
