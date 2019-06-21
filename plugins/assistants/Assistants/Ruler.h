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

#ifndef _RULER_H_
#define _RULER_H_

#include <QPointF>

class Ruler
{
public:
    Ruler();
    ~Ruler();
    QPointF project(const QPointF&);
    const QPointF& point1() const;
    void setPoint1(const QPointF& p) {
        p1 = p;
    }
    const QPointF& point2() const;
    void setPoint2(const QPointF& p) {
        p2 = p;
    }
private:
    QPointF p1;
    QPointF p2;
};

#endif
