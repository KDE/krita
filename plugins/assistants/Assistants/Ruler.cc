/*
 * SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
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
