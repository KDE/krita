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

class KisSplatGeneratorStrategy
{
public:
    virtual ~KisSplatGeneratorStrategy(){}
    virtual void generate(KoRTree<KisSplat *> *flowing, KisWetMap *wetMap, QPointF pos, qreal radius/*, KoColor color*/) = 0;

};

class KisSimpleBrushGenerator : public KisSplatGeneratorStrategy
{
public:
    void generate(KoRTree<KisSplat *> *flowing, KisWetMap *wetMap, QPointF pos, qreal radius/*, KoColor color*/) override;
};

class KisWetOnDryGenerator : public KisSplatGeneratorStrategy
{
public:
    void generate(KoRTree<KisSplat *> *flowing, KisWetMap *wetMap, QPointF pos, qreal radius) override;
};

class KisCrunchyGenerator : public KisSplatGeneratorStrategy
{
public:
    void generate(KoRTree<KisSplat *> *flowing, KisWetMap *wetMap, QPointF pos, qreal radius) override;
};

class KisWetOnWetGenerator : public KisSplatGeneratorStrategy
{
public:
    void generate(KoRTree<KisSplat *> *flowing, KisWetMap *wetMap, QPointF pos, qreal radius) override;
};

class KisBlobbyGenerator : public KisSplatGeneratorStrategy
{
public:
    void generate(KoRTree<KisSplat *> *flowing, KisWetMap *wetMap, QPointF pos, qreal radius) override;
};

#endif // KIS_SPLAT_GENERATOR_STRATEGY_H
