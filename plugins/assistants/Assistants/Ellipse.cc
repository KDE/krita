/*
 * Copyright (c) 2010 Geoffry Song <goffrie@gmail.com>
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

#include "Ellipse.h"
#include <cmath>

#include <iostream>

Ellipse::Ellipse() : a(-1), b(-1)
{
}
Ellipse::Ellipse(const QPointF& _p1, const QPointF& _p2, const QPointF& _p3) : p1(_p1), p2(_p2), p3(_p3) {
    changeMajor();
}

Ellipse::~Ellipse()
{
}

bool Ellipse::set(const QPointF& m1, const QPointF& m2, const QPointF& p)
{
    bool changedMajor = m1 != p1 || m2 != p2,
         changedMinor = !changedMajor && p != p3;
    p1 = m1;
    p2 = m2;
    p3 = p;
    if (changedMajor) {
        return changeMajor();
    } else if (changedMinor) {
        return changeMinor();
    } else {
        return a > 0 && b > 0;
    }
}

QPointF Ellipse::project(const QPointF& pt) const
{
    if (a <= 0 || b <= 0) return pt; // not a valid ellipse
    QPointF p = matrix.map(pt);
    /*
     * intersect line from (0,0) to p with the ellipse in canonical position
     * the equation of the line is y = py/px x
     * the equation of the ellipse is x^2/a^2 + y^2/b^2 = 1
     * x=(a*b*px)/sqrt(a^2*py^2+b^2*px^2)
     * y=(a*b*py)/sqrt(a^2*py^2+b^2*px^2)
     */
    const qreal divisor = sqrt(a * a * p.y() * p.y() + b * b * p.x() * p.x());
    if (divisor <= 0) return inverse.map(QPointF(a, 0)); // give up
    const qreal ab = a * b, factor = 1.0 / divisor;
    QPointF ep(ab * p.x() * factor, ab * p.y() * factor);
    return inverse.map(ep);
/*    return inverse.map(closest(matrix.map(pt)));*/
}

inline QPointF rotate90(const QPointF& p) {
    return QPointF(p.y(), -p.x());
}

QRectF Ellipse::boundingRect() const
{
    const QPointF d = rotate90((p2 - p1) * 0.5 * b / a);
    const QPointF pts[4] = {
        p1 + d,
        p1 - d,
        p2 + d,
        p2 - d
    };
    QRectF ret;
    for (int i = 0; i < 4; ++i) {
        ret = ret.united(QRectF(pts[i], QSizeF(0.0001, 0.0001)));
    }
    return ret;
}

inline qreal sqrlength(const QPointF& vec)
{
    return vec.x() * vec.x() + vec.y() * vec.y();
}
inline qreal length(const QPointF& vec)
{
    return sqrt(vec.x() * vec.x() + vec.y() * vec.y());
}

bool Ellipse::changeMajor()
{
    a = length(p1 - p2) * 0.5;
    
    /*
     * calculate transformation matrix
     * x' = m11*x + m21*y + dx
     * y' = m22*y + m12*x + dy
     * m11 = m22, m12 = -m21 (rotations and translations only)
     * x' = m11*x - m12*y + dx
     * y' = m11*y + m12*x + dy
     * 
     * then, transforming (x1, y1) to (x1', y1') and (x2, y2) to (x2', y2'):
     * 
     * m11 = (y2*y2' + y1 * (y1'-y2') - y2*y1' + x2*x'2 - x1*x'2 + (x1-x2)*x1')
     *       ------------------------------------------------------------------
     *                 (y2^2 - 2*y1*y2 + y1^2 + x2^2 - 2*x1*x2 + x1^2)
     * m12 = -(x1*(y2'-y1') - x2*y2' + x2*y1' + x2'*y2 - x1'*y2 + (x1'-x2')*y1)
     *       ------------------------------------------------------------------
     *                 (y2^2 - 2*y1*y2 + y1^2 + x2^2 - 2*x1*x2 + x1^2)
     * dx = (x1*(-y2*y2' + y2*y1' - x2*x2') + y1*( x2*y2' - x2*y1' - x2'*y2 - x1'*y2) + x2'*y1^2 + x1^2*x2' + x1'*(y2^2 + x2^2 - x1*x2))
     *      ----------------------------------------------------------------------------------------------------------------------------
     *                 (y2^2 - 2*y1*y2 + y1^2 + x2^2 - 2*x1*x2 + x1^2)
     * dy = (x1*(-x2*y2' - x2*y1' + x2'*y2) + y1*(-y2*y2' - y2*y1' - x2*x2' + x2*x1') + y2'*y1^2 + x1^2*y2' + y1'(y2^2 + x2^2) - x1*x1'*y2)
     *      -------------------------------------------------------------------------------------------------------------------------------
     *                 (y2^2 - 2*y1*y2 + y1^2 + x2^2 - 2*x1*x2 + x1^2)
     * 
     * in our usage, to move the ellipse into canonical position:
     * 
     * (x1, y1) = p1
     * (x2, y2) = p2
     * (x1', y1') = (-a, 0)
     * (x2', y2') = (a, 0)
     */
    
    const qreal
        x1 = p1.x(),
        x2 = p2.x(),
        y1 = p1.y(),
        y2 = p2.y(),
        x1p = -a,
        x2p = a,
        x1sqr = x1 * x1,
        x2sqr = x2 * x2,
        y1sqr = y1 * y1,
        y2sqr = y2 * y2,
        factor = 1.0 / (x1sqr + y1sqr + x2sqr + y2sqr - 2.0 * y1 * y2 - 2.0 * x1 * x2),
        m11 = (x2*x2p - x1*x2p + (x1-x2)*x1p) * factor,
        m12 = -(x2p*y2 - x1p*y2 + (x1p-x2p)*y1) * factor,
        dx = (x1*(-x2*x2p) + y1*(-x2p*y2 - x1p*y2) + x2p*y1sqr + x1sqr*x2p + x1p*(y2sqr + x2sqr - x1*x2)) * factor,
        dy = (x1*(x2p*y2) + y1*(-x2*x2p + x2*x1p) - x1*x1p*y2) * factor;
    
    matrix = QTransform(m11, m12, -m12, m11, dx, dy);
    inverse = matrix.inverted();

    return changeMinor();
}

bool Ellipse::changeMinor()
{
    QPointF p = matrix.map(p3);

    /*
     * ellipse formula:
     * x^2/a^2 + y^2/b^2 = 1
     * b = sqrt(y^2 / (1 - x^2/a^2))
     */
    const qreal
        asqr = a * a,
        xsqr = p.x() * p.x(),
        ysqr = p.y() * p.y(),
        divisor = (1.0 - xsqr / asqr);
    if (divisor <= 0) {
        // division by zero!
        b = -1;
        return false;
    }
    b = sqrt(ysqr / divisor);
    return true;
}

bool Ellipse::setMajor1(const QPointF& p)
{
    p1 = p;
    return changeMajor();
}
bool Ellipse::setMajor2(const QPointF& p) {
    p2 = p;
    return changeMajor();
}
bool Ellipse::setPoint(const QPointF& p)
{
    p3 = p;
    return changeMinor();
}
