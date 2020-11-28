/*
 * SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
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
