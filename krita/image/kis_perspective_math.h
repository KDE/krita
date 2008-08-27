/*
 * This file is part of Krita
 *
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_PERSPECTIVE_MATH_H_
#define _KIS_PERSPECTIVE_MATH_H_

#include <QPointF>

#include "kis_vec.h"
typedef Eigen::Matrix<qreal, 3, 3> Matrix3qreal;
typedef Eigen::Matrix<qreal, 9, 9> Matrix9qreal;
typedef Eigen::Matrix<qreal, 9, 1> Vector9qreal;

#include <krita_export.h>

class QRect;

class KRITAIMAGE_EXPORT LineEquation
{
private:
    // a*x + b*y = c. We normalize a and b so that a^2+b^2=1. This makes computing the line equation
    // slightly more complex, but simplifies computing intersections and point-to-line-distance.
    KisVector2D m_normal; // the coords of this vector are a and b. The norm of this vector is 1.
    qreal m_c;
    void compute(const KisVector2D& p1, const KisVector2D& p2);

public:
    inline LineEquation(const KisVector2D& p1, const KisVector2D& p2) {
        compute(p1, p2);
    }
    inline LineEquation(const QPointF* p1, const QPointF* p2) {
        compute(toKisVector2D(*p1), toKisVector2D(*p2));
    }
    KisVector2D intersection(const LineEquation& other) const;
    inline qreal distance(const KisVector2D& p) const {
        return std::abs(normal().dot(p) - c());
    }
    inline qreal distance(const QPointF& p) const {
        return distance(toKisVector2D(p));
    }
    inline const qreal& a() const {
        return m_normal.coeff(0);
    }
    inline const qreal& b() const {
        return m_normal.coeff(1);
    }
    inline const qreal& c() const {
        return m_c;
    }
    inline const KisVector2D normal() const {
        return m_normal;
    }
};

class KRITAIMAGE_EXPORT KisPerspectiveMath
{
private:
    KisPerspectiveMath() { }
public:
    static Matrix3qreal computeMatrixTransfo(const QPointF& topLeft1, const QPointF& topRight1, const QPointF& bottomLeft1, const QPointF& bottomRight1, const QPointF& topLeft2, const QPointF& topRight2, const QPointF& bottomLeft2, const QPointF& bottomRight2);
    static Matrix3qreal computeMatrixTransfoToPerspective(const QPointF& topLeft, const QPointF& topRight, const QPointF& bottomLeft, const QPointF& bottomRight, const QRect& r);
    static Matrix3qreal computeMatrixTransfoFromPerspective(const QRect& r, const QPointF& topLeft, const QPointF& topRight, const QPointF& bottomLeft, const QPointF& bottomRight);
    /// TODO: get rid of this in 2.0
    static inline QPointF matProd(const Matrix3qreal& m, const QPointF& p) {
        qreal s = qreal(1) / (p.x() * m.coeff(2, 0) + p.y() * m.coeff(2, 1) + 1.0);
        return QPointF((p.x() * m.coeff(0, 0) + p.y() * m.coeff(0, 1) + m.coeff(0, 2)) * s,
                       (p.x() * m.coeff(1, 0) + p.y() * m.coeff(1, 1) + m.coeff(1, 2)) * s);
    }
};

#endif
