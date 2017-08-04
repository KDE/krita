/* This file is part of the KDE project
 *
 * Copyright (C) 2017 Grigory Tantsevov <tantsevov@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KIS_SPLAT_GENERATOR_STRATEGY_H
#define KIS_SPLAT_GENERATOR_STRATEGY_H

#include <KoRTree.h>
#include "kis_splat.h"
#include "kis_wetmap.h"
#include <QList>

class KisSplatGeneratorStrategy
{
public:
    virtual ~KisSplatGeneratorStrategy(){}
    virtual QList<KisSplat *> generate(KisWetMap *wetMap, QPointF pos, qreal radius, const KoColor &color) = 0;

};

class KisSimpleBrushGenerator : public KisSplatGeneratorStrategy
{
public:
    QList<KisSplat *> generate(KisWetMap *wetMap, QPointF pos, qreal radius, const KoColor &color) override;
};

class KisWetOnDryGenerator : public KisSplatGeneratorStrategy
{
public:
    QList<KisSplat *> generate(KisWetMap *wetMap, QPointF pos, qreal radius, const KoColor &color) override;
};

class KisCrunchyGenerator : public KisSplatGeneratorStrategy
{
public:
    QList<KisSplat *> generate(KisWetMap *wetMap, QPointF pos, qreal radius, const KoColor &color) override;
};

class KisWetOnWetGenerator : public KisSplatGeneratorStrategy
{
public:
    QList<KisSplat *> generate(KisWetMap *wetMap, QPointF pos, qreal radius, const KoColor &color) override;
};

class KisBlobbyGenerator : public KisSplatGeneratorStrategy
{
public:
    QList<KisSplat *> generate(KisWetMap *wetMap, QPointF pos, qreal radius, const KoColor &color) override;
};

#endif // KIS_SPLAT_GENERATOR_STRATEGY_H
