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

#ifndef _KIS_PERSPECTVE_MATH_H_
#define _KIS_PERSPECTVE_MATH_H_

#include <QPointF>

#include <Eigen/Core>
typedef Eigen::Matrix<qreal, 3, 3> Matrix3qreal;
typedef Eigen::Matrix<qreal, 9, 9> Matrix9qreal;
typedef Eigen::Matrix<qreal, 9, 1> Vector9qreal;

#include <krita_export.h>

class QRect;

class KRITAIMAGE_EXPORT KisPerspectiveMath {
    private:
        KisPerspectiveMath() { }
    public:
        static Matrix3qreal computeMatrixTransfo( const QPointF& topLeft1, const QPointF& topRight1, const QPointF& bottomLeft1, const QPointF& bottomRight1, const QPointF& topLeft2, const QPointF& topRight2, const QPointF& bottomLeft2, const QPointF& bottomRight2);
        static Matrix3qreal computeMatrixTransfoToPerspective(const QPointF& topLeft, const QPointF& topRight, const QPointF& bottomLeft, const QPointF& bottomRight, const QRect& r);
        static Matrix3qreal computeMatrixTransfoFromPerspective(const QRect& r, const QPointF& topLeft, const QPointF& topRight, const QPointF& bottomLeft, const QPointF& bottomRight);
        struct LineEquation {
            // a*x + b*y = c
            qreal a, b, c;
        };
        /// TODO: get rid of this in 2.0
        static inline QPointF matProd(const Matrix3qreal& m, const QPointF& p)
        {
            qreal s = qreal(1) / ( p.x() * m.coeff(2,0) + p.y() * m.coeff(2,1) + 1.0);
            return QPointF( (p.x() * m.coeff(0,0) + p.y() * m.coeff(0,1) + m.coeff(0,2) ) * s,
                              (p.x() * m.coeff(1,0) + p.y() * m.coeff(1,1) + m.coeff(1,2) ) * s );
        }
        static inline LineEquation computeLineEquation(const QPointF* p1, const QPointF* p2)
        {
            LineEquation eq;
            eq.a = p1->y() - p2->y();
            eq.b = p2->x() - p1->x();
            eq.c = eq.b * p1->x() + eq.a * p1->y();
            return eq;
        }
                    
        static QPointF computeIntersection(const LineEquation& d1, const LineEquation& d2);
};

#endif
