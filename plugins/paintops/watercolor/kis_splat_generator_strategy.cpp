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

#include "kis_splat_generator_strategy.h"

#include <QVector2D>

void KisSimpleBrushGenerator::generate(QList<KisSplat *> *flowing, KisWetMap *wetMap, QPointF pos, qreal radius, const KoColor &color)
{
    KisSplat *newSplat = new KisSplat(pos, radius,
                                      color);
    wetMap->addWater(pos.toPoint(), radius / 2);
    flowing->push_back(newSplat);
}

void KisWetOnDryGenerator::generate(QList<KisSplat *> *flowing, KisWetMap *wetMap, QPointF pos, qreal radius, const KoColor &color)
{
    wetMap->addWater(pos.toPoint(), radius / 2);

    /// 1 splat in center, 6 around
    qreal radialSpeed = 2.f;
    int d = radius / 2;
    int r = d / 2;
    QVector2D offset;

    KisSplat *splat = new KisSplat(pos, radius,
                                   color);
    flowing->push_back(splat);

    for (int i = 0; i < 6; i++) {
        qreal theta = i * M_PI / 3;
        offset.setX(r * cos(theta));
        offset.setY(r * sin(theta));

        splat = new KisSplat(pos + offset.toPointF(),
                             0.1f * radialSpeed * offset.normalized().toPointF(),
                             d,
                             30,
                             0.5f * radialSpeed,
                             1.f,
                             radialSpeed,
                             color);
        flowing->push_back(splat);
    }
}

void KisCrunchyGenerator::generate(QList<KisSplat *> *flowing, KisWetMap *wetMap, QPointF pos, qreal radius, const KoColor &color)
{
    wetMap->addWater(pos.toPoint(), radius / 2);

    KisSplat *splat = new KisSplat(pos,
                                      QPointF(0, 0),
                                      radius,
                                      15, 5, 0.25f, 2.f,
                                   color);
    flowing->push_back(splat);
}

void KisWetOnWetGenerator::generate(QList<KisSplat *> *flowing, KisWetMap *wetMap, QPointF pos, qreal radius, const KoColor &color)
{
    wetMap->addWater(pos.toPoint(), radius / 2);

    int smallD = radius / 2;
    int bigD = 3 * radius / 2;

    KisSplat *splat = new KisSplat(pos,
                                   QPointF(0, 0),
                                   bigD,
                                   15, 5, 1.f, 2.f,
                                   color);
    flowing->push_back(splat);

    splat = new KisSplat(pos,
                         QPointF(0, 0),
                         smallD,
                         15, 5, 1.f, 2.f,
                         color);
    flowing->push_back(splat);
}

void KisBlobbyGenerator::generate(QList<KisSplat *> *flowing, KisWetMap *wetMap, QPointF pos, qreal radius, const KoColor &color)
{
    wetMap->addWater(pos.toPoint(), radius / 2);

    qreal firstD = (qreal) radius / 3;
    qreal lastD = (qreal) radius;
    KisSplat *splat;

    for (int i = 0; i < 4; i++) {
        qreal size = get_random(firstD, lastD);
        QPointF posN;
        switch (i) {
        case 0:
            posN = pos + QPointF(0, (radius - size) / 2);
            break;
        case 1:
            posN = pos + QPointF((radius - size) / 2, 0);
        case 2:
            posN = pos - QPointF(0, (radius - size) / 2);
            break;
        case 3:
            posN = pos - QPointF((radius - size) / 2, 0);
        default:
            break;
        }

        splat = new KisSplat(posN,
                             QPointF(0, 0),
                             size,
                             15, 5, 1.f, 2.f,
                             color);
        flowing->push_back(splat);
    }
}
