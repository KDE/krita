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

#ifndef KIS_SPLAT_H
#define KIS_SPLAT_H

#include <QPointF>
#include <QPolygonF>
#include <KoColor.h>
#include <QTime>
#include <QPainterPath>

#include "kis_wetmap.h"
#include "kis_random_generator.h"

#include "kritawatercolorpaintop_export.h"

#include "kis_painter.h"

class WATERCOLORPAINT_EXPORT KisSplat
{
public:
    enum SplatState {
        Flowing,
        Fixed,
        Dried
    };

    KisSplat(QPointF offset, int width, KoColor splatColor);
    KisSplat(QPointF offset, QPointF velocityBias, int width, int life,
          qreal roughness, qreal flow, qreal radialSpeed, KoColor splatColor);

    void doPaint(KisPainter *painter);

    QPainterPath shape() const;
    QRectF boundingRect() const;

    int update(KisWetMap *wetMap);
    int rewet(KisWetMap *wetMap, QPointF pos, qreal radius);

private:
    qreal CalcSize();

    const float alpha = 0.33f;

    QPolygonF m_vertices;
    QVector<QPointF> m_velocities;
    int m_life;
    qreal m_roughness;
    qreal m_flow;
    QPointF m_motionBias;

    QTime m_startTime;

    qreal m_initSize;
    KoColor m_initColor;

    int m_fix;
};

#endif
