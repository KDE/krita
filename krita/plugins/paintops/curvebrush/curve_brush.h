/*
 *  Copyright (c) 2008-2011 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#ifndef _CURVE_BRUSH_H_
#define _CURVE_BRUSH_H_

#include <KoColor.h>

#include <kis_paint_device.h>
#include <kis_paint_information.h>
#include <kis_random_accessor_ng.h>
#include <kis_painter.h>
#include "kis_curve_paintop_settings_widget.h"
#include <KoColorSpace.h>

class CurveProperties;
class Pen {
public:
    Pen():pos(QPointF(0,0)), rotation(0), scale(0)
    {

    }

    Pen(QPointF ipos,qreal irotation, qreal iscale)
        :pos(ipos),
        rotation(irotation),
        scale(iscale)
    {

    }

    ~Pen() {}

    QPointF pos;
    qreal rotation;
    qreal scale;
};

class CurveBrush
{

public:
    CurveBrush();
    ~CurveBrush();

private:
    QPointF getCubicBezier(const QPointF &p0, const QPointF &p1, const QPointF &p2, const QPointF &p3, qreal u);
    QPointF getQuadraticBezier(const QPointF &p0, const QPointF &p1, const QPointF &p2, qreal u);
    QPointF getLinearBezier(const QPointF &p1, const QPointF &p2, qreal u);
    void putPixel(QPointF pos, KoColor &color);

    KisRandomAccessorSP m_writeAccessor;
    KoColorSpace * cs;
    quint32 m_pixelSize;

    KisPainter * m_painter;

    QList<Pen> m_pens;
    int m_branch;
    Pen m_newPen;

    void strokePens(QPointF pi1, QPointF pi2, KisPainter &painter);
};

#endif
