/*
 * Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "Ruler.h"

Ruler::Ruler() : p1(QPointF(10, 10)), p2(QPointF(100, 190))
{
}

Ruler::~Ruler()
{
}

QPointF Ruler::project(const QPointF& pt)
{
    double x1 = p1.x();
    double y1 = p1.y();
    double x2 = p2.x();
    double y2 = p2.y();
    double a1 = (y2 - y1) / (x2 - x1);
    double b1 = y1 - x1 * a1;
    double a2 = (x2 - x1) / (y1 - y2);
    double b2 = pt.y() - a2 * pt.x();
    double xm = (b2 - b1) / (a1 - a2);
    return QPointF(xm, xm * a1 + b1);
}

const QPointF& Ruler::point1() const
{
    return p1;
}

const QPointF& Ruler::point2() const
{
    return p2;
}
