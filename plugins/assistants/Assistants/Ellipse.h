/*
 * SPDX-FileCopyrightText: 2010 Geoffry Song <goffrie@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _ELLIPSE_H_
#define _ELLIPSE_H_

#include <QPointF>
#include <QTransform>

class Ellipse
{
public:
    Ellipse();
    Ellipse(const QPointF& p1, const QPointF& p2, const QPointF& p3);
    ~Ellipse();
    
    QPointF project(const QPointF&) const; // find a close point on the ellipse
    QRectF boundingRect() const; // find an axis-aligned box bounding this ellipse (inexact)
    
    bool set(const QPointF& m1, const QPointF& m2, const QPointF& p); // set all points
    
    const QPointF& major1() const { return p1; }
    bool setMajor1(const QPointF& p);
    const QPointF& major2() const { return p2; }
    bool setMajor2(const QPointF& p);
    const QPointF& point() const { return p3; }
    bool setPoint(const QPointF& p);
    const QTransform& getTransform() const { return matrix; }
    const QTransform& getInverse() const { return inverse; }
    qreal semiMajor() const { return a; }
    qreal semiMinor() const { return b; }
    
private:
    bool changeMajor(); // determine 'a', 'b', 'matrix' and 'inverse'
    bool changeMinor(); // determine 'b'
    
    QTransform matrix; // transformation turning p1, p2 and p3 into their corresponding points on the ellipse in canonical position
    QTransform inverse; // inverse transformation
    qreal a; // semi-major axis: half the distance between p1 and p2 (horizontal axis)
    qreal b; // semi-minor axis (vertical axis)
    // a may not actually be larger than b, but we don't care that much
    
    QPointF p1;
    QPointF p2;
    QPointF p3;
};

#endif
