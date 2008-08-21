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

#include "kis_perspective_math.h"

#include <QRect>

#include <Eigen/LU>

Matrix3qreal KisPerspectiveMath::computeMatrixTransfo( const QPointF& topLeft1, const QPointF& topRight1, const QPointF& bottomLeft1, const QPointF& bottomRight1 , const QPointF& topLeft2, const QPointF& topRight2, const QPointF& bottomLeft2, const QPointF& bottomRight2)
{
    Matrix9qreal a = Matrix9qreal::Zero();
    Vector9qreal b = Vector9qreal::Zero();

    // topLeft
    a.coeffRef(0,0) = topLeft1.x();
    a.coeffRef(0,1) = topLeft1.y();
    a.coeffRef(0,2) = 1;
    a.coeffRef(0,6) = -topLeft2.x() * topLeft1.x();
    a.coeffRef(0,7) = -topLeft2.x() * topLeft1.y();
    a.coeffRef(0,8) = -topLeft2.x();
    a.coeffRef(1,3) = topLeft1.x();
    a.coeffRef(1,4) = topLeft1.y();
    a.coeffRef(1,5) = 1;
    a.coeffRef(1,6) = -topLeft2.y() * topLeft1.x();
    a.coeffRef(1,7) = -topLeft2.y() * topLeft1.y();
    a.coeffRef(1,8) = -topLeft2.y();
    // topRight
    a.coeffRef(2,0) = topRight1.x();
    a.coeffRef(2,1) = topRight1.y();
    a.coeffRef(2,2) = 1;
    a.coeffRef(2,6) = -topRight2.x() * topRight1.x();
    a.coeffRef(2,7) = -topRight2.x() * topRight1.y();
    a.coeffRef(2,8) = -topRight2.x();
    a.coeffRef(3,3) = topRight1.x();
    a.coeffRef(3,4) = topRight1.y();
    a.coeffRef(3,5) = 1;
    a.coeffRef(3,6) = -topRight2.y() * topRight1.x();
    a.coeffRef(3,7) = -topRight2.y() * topRight1.y();
    a.coeffRef(3,8) = -topRight2.y();
    // bottomLeft
    a.coeffRef(4,0) = bottomLeft1.x();
    a.coeffRef(4,1) = bottomLeft1.y();
    a.coeffRef(4,2) = 1;
    a.coeffRef(4,6) = -bottomLeft2.x() * bottomLeft1.x();
    a.coeffRef(4,7) = -bottomLeft2.x() * bottomLeft1.y();
    a.coeffRef(4,8) = -bottomLeft2.x();
    a.coeffRef(5,3) = bottomLeft1.x();
    a.coeffRef(5,4) = bottomLeft1.y();
    a.coeffRef(5,5) = 1;
    a.coeffRef(5,6) = -bottomLeft2.y() * bottomLeft1.x();
    a.coeffRef(5,7) = -bottomLeft2.y() * bottomLeft1.y();
    a.coeffRef(5,8) = -bottomLeft2.y();
    // bottomRight
    a.coeffRef(6,0) = bottomRight1.x();
    a.coeffRef(6,1) = bottomRight1.y();
    a.coeffRef(6,2) = 1;
    a.coeffRef(6,6) = -bottomRight2.x() * bottomRight1.x();
    a.coeffRef(6,7) = -bottomRight2.x() * bottomRight1.y();
    a.coeffRef(6,8) = -bottomRight2.x();
    a.coeffRef(7,3) = bottomRight1.x();
    a.coeffRef(7,4) = bottomRight1.y();
    a.coeffRef(7,5) = 1;
    a.coeffRef(7,6) = -bottomRight2.y() * bottomRight1.x();
    a.coeffRef(7,7) = -bottomRight2.y() * bottomRight1.y();
    a.coeffRef(7,8) = -bottomRight2.y();
    a.coeffRef(8,8) = 1;
    b.coeffRef(8) = 1;
//     dbgImage <<" a := { {" << a.coeffRef(0,0) <<" ," << a.coeffRef(0,1) <<" ," << a.coeffRef(0,2) <<" ," << a.coeffRef(0,3) <<" ," << a.coeffRef(0,4) <<" ," << a.coeffRef(0,5) <<" ," << a.coeffRef(0,6) <<" ," << a.coeffRef(0,7) <<" ," << a.coeffRef(0,8) <<" } , {" << a.coeffRef(1,0) <<" ," << a.coeffRef(1,1) <<" ," << a.coeffRef(1,2) <<" ," << a.coeffRef(1,3) <<" ," << a.coeffRef(1,4) <<" ," << a.coeffRef(1,5) <<" ," << a.coeffRef(1,6) <<" ," << a.coeffRef(1,7) <<" ," << a.coeffRef(1,8) <<" } , {" << a.coeffRef(2,0) <<" ," << a.coeffRef(2,1) <<" ," << a.coeffRef(2,2) <<" ," << a.coeffRef(2,3) <<" ," << a.coeffRef(2,4) <<" ," << a.coeffRef(2,5) <<" ," << a.coeffRef(2,6) <<" ," << a.coeffRef(2,7) <<" ," << a.coeffRef(2,8) <<" } , {" << a.coeffRef(3,0) <<" ," << a.coeffRef(3,1) <<" ," << a.coeffRef(3,2) <<" ," << a.coeffRef(3,3) <<" ," << a.coeffRef(3,4) <<" ," << a.coeffRef(3,5) <<" ," << a.coeffRef(3,6) <<" ," << a.coeffRef(3,7) <<" ," << a.coeffRef(3,8) <<" } , {" << a.coeffRef(4,0) <<" ," << a.coeffRef(4,1) <<" ," << a.coeffRef(4,2) <<" ," << a.coeffRef(4,3) <<" ," << a.coeffRef(4,4) <<" ," << a.coeffRef(4,5) <<" ," << a.coeffRef(4,6) <<" ," << a.coeffRef(4,7) <<" ," << a.coeffRef(4,8) <<" } , {" << a.coeffRef(5,0) <<" ," << a.coeffRef(5,1) <<" ," << a.coeffRef(5,2) <<" ," << a.coeffRef(5,3) <<" ," << a.coeffRef(5,4) <<" ," << a.coeffRef(5,5) <<" ," << a.coeffRef(5,6) <<" ," << a.coeffRef(5,7) <<" ," << a.coeffRef(5,8) <<" } , {" << a.coeffRef(6,0) <<" ," << a.coeffRef(6,1) <<" ," << a.coeffRef(6,2) <<" ," << a.coeffRef(6,3) <<" ," << a.coeffRef(6,4) <<" ," << a.coeffRef(6,5) <<" ," << a.coeffRef(6,6) <<" ," << a.coeffRef(6,7) <<" ," << a.coeffRef(6,8) <<" } , {"<< a.coeffRef(7,0) <<" ," << a.coeffRef(7,1) <<" ," << a.coeffRef(7,2) <<" ," << a.coeffRef(7,3) <<" ," << a.coeffRef(7,4) <<" ," << a.coeffRef(7,5) <<" ," << a.coeffRef(7,6) <<" ," << a.coeffRef(7,7) <<" ," << a.coeffRef(7,8) <<" } , {"<< a.coeffRef(8,0) <<" ," << a.coeffRef(8,1) <<" ," << a.coeffRef(8,2) <<" ," << a.coeffRef(8,3) <<" ," << a.coeffRef(8,4) <<" ," << a.coeffRef(8,5) <<" ," << a.coeffRef(8,6) <<" ," << a.coeffRef(8,7) <<" ," << a.coeffRef(8,8) <<" } };";
    Vector9qreal v;
    a.lu().solve(b, &v);
    Matrix3qreal matrix;
    for(int r = 0; r < 3; r++) for(int c = 0; c < 3; c++) matrix.coeffRef(r,c) = v.coeff(3*r+c);
    return matrix;
}

Matrix3qreal KisPerspectiveMath::computeMatrixTransfoToPerspective(const QPointF& topLeft, const QPointF& topRight, const QPointF& bottomLeft, const QPointF& bottomRight, const QRect& r)
{
    return KisPerspectiveMath::computeMatrixTransfo(topLeft, topRight, bottomLeft, bottomRight, r.topLeft(), r.topRight(), r.bottomLeft(), r.bottomRight());
}

Matrix3qreal KisPerspectiveMath::computeMatrixTransfoFromPerspective(const QRect& r, const QPointF& topLeft, const QPointF& topRight, const QPointF& bottomLeft, const QPointF& bottomRight)
{
    return KisPerspectiveMath::computeMatrixTransfo(r.topLeft(), r.topRight(), r.bottomLeft(), r.bottomRight(), topLeft, topRight, bottomLeft, bottomRight);
}

QPointF KisPerspectiveMath::computeIntersection(const LineEquation& d1, const LineEquation& d2)
{
    qreal det = d1.a * d2.b - d1.b * d2.a;
    if(Eigen::ei_isMuchSmallerThan(det, qMax(qAbs(d1.c),qAbs(d2.c))))
    {   // special case where the two lines are approximately parallel. Pick any point on the first line.
        if(qAbs(d1.b)>qAbs(d1.a))
            return QPointF(d1.b, d1.c/d1.b-d1.a);
        else
            return QPointF(d1.c/d1.a-d1.b, d1.a);
    }
    else
    {   // general case
        qreal invdet = qreal(1) / det;
        return QPointF(invdet*(d2.b*d1.c-d1.b*d2.c), invdet*(d1.a*d1.c-d2.a*d2.c));
    }
}
