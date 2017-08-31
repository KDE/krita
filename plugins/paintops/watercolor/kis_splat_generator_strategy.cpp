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

QList<KisSplat *> KisSimpleBrushGenerator::generate(KisWetMap *wetMap, QPointF pos, qreal radius, const KoColor &color, int gravityX, int gravityY)
{
    QList<KisSplat *> ret;
    KisSplat *newSplat = new KisSplat(pos, radius,
                                      color,
                                      gravityX, gravityY);
    wetMap->addWater(pos.toPoint(), radius / 2);
    ret << newSplat;

    return ret;
}

QList<KisSplat *> KisWetOnDryGenerator::generate(KisWetMap *wetMap, QPointF pos, qreal radius, const KoColor &color, int gravityX, int gravityY)
{
    QList<KisSplat *> ret;
    wetMap->addWater(pos.toPoint(), radius / 2);

    /// 1 splat in center, 6 around
    qreal radialSpeed = 2.f;
    int d = radius / 2;
    int r = d / 2;
    QVector2D offset;

    KisSplat *splat = new KisSplat(pos, radius,
                                   color,
                                   gravityX, gravityY);
    ret << splat;

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
                             color,
                             gravityX, gravityY);
        ret << splat;
    }
    return ret;
}

QList<KisSplat *> KisCrunchyGenerator::generate(KisWetMap *wetMap, QPointF pos, qreal radius, const KoColor &color, int gravityX, int gravityY)
{
    QList<KisSplat *> ret;
    wetMap->addWater(pos.toPoint(), radius / 2);

    KisSplat *splat = new KisSplat(pos,
                                   QPointF(0, 0),
                                   radius,
                                   30, 5, 0.25f, 2.f,
                                   color,
                                   gravityX, gravityY);
    ret << splat;
    return ret;
}

QList<KisSplat *> KisWetOnWetGenerator::generate(KisWetMap *wetMap, QPointF pos, qreal radius, const KoColor &color, int gravityX, int gravityY)
{
    QList<KisSplat *> ret;
    wetMap->addWater(pos.toPoint(), radius / 2);

    int smallD = radius / 2;
    int bigD = 3 * radius / 2;

    KisSplat *splat = new KisSplat(pos,
                                   QPointF(0, 0),
                                   bigD,
                                   30, 5, 1.f, 2.f,
                                   color,
                                   gravityX, gravityY);
    ret << splat;

    splat = new KisSplat(pos,
                         QPointF(0, 0),
                         smallD,
                         30, 5, 1.f, 2.f,
                         color,
                         gravityX, gravityY);
    ret << splat;
    return ret;
}

QList<KisSplat *> KisBlobbyGenerator::generate(KisWetMap *wetMap, QPointF pos, qreal radius, const KoColor &color, int gravityX, int gravityY)
{
    QList<KisSplat *> ret;
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
                             30, 5, 1.f, 2.f,
                             color,
                             gravityX, gravityY);
        ret << splat;
    }

    return ret;
}
