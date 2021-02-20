/*
 * This file is part of Krita
 *
 *  SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2008 Benoit Jacob <jacob.benoit.1@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_perspective_math.h"

#include <QRect>

#include <Eigen/LU>

Matrix3qreal KisPerspectiveMath::computeMatrixTransfo(const QPointF& topLeft1, const QPointF& topRight1, const QPointF& bottomLeft1, const QPointF& bottomRight1 , const QPointF& topLeft2, const QPointF& topRight2, const QPointF& bottomLeft2, const QPointF& bottomRight2)
{
    Matrix9qreal a = Matrix9qreal::Zero();
    Vector9qreal b = Vector9qreal::Zero();

    // topLeft
    a.coeffRef(0, 0) = topLeft1.x();
    a.coeffRef(0, 1) = topLeft1.y();
    a.coeffRef(0, 2) = 1;
    a.coeffRef(0, 6) = -topLeft2.x() * topLeft1.x();
    a.coeffRef(0, 7) = -topLeft2.x() * topLeft1.y();
    a.coeffRef(0, 8) = -topLeft2.x();
    a.coeffRef(1, 3) = topLeft1.x();
    a.coeffRef(1, 4) = topLeft1.y();
    a.coeffRef(1, 5) = 1;
    a.coeffRef(1, 6) = -topLeft2.y() * topLeft1.x();
    a.coeffRef(1, 7) = -topLeft2.y() * topLeft1.y();
    a.coeffRef(1, 8) = -topLeft2.y();
    // topRight
    a.coeffRef(2, 0) = topRight1.x();
    a.coeffRef(2, 1) = topRight1.y();
    a.coeffRef(2, 2) = 1;
    a.coeffRef(2, 6) = -topRight2.x() * topRight1.x();
    a.coeffRef(2, 7) = -topRight2.x() * topRight1.y();
    a.coeffRef(2, 8) = -topRight2.x();
    a.coeffRef(3, 3) = topRight1.x();
    a.coeffRef(3, 4) = topRight1.y();
    a.coeffRef(3, 5) = 1;
    a.coeffRef(3, 6) = -topRight2.y() * topRight1.x();
    a.coeffRef(3, 7) = -topRight2.y() * topRight1.y();
    a.coeffRef(3, 8) = -topRight2.y();
    // bottomLeft
    a.coeffRef(4, 0) = bottomLeft1.x();
    a.coeffRef(4, 1) = bottomLeft1.y();
    a.coeffRef(4, 2) = 1;
    a.coeffRef(4, 6) = -bottomLeft2.x() * bottomLeft1.x();
    a.coeffRef(4, 7) = -bottomLeft2.x() * bottomLeft1.y();
    a.coeffRef(4, 8) = -bottomLeft2.x();
    a.coeffRef(5, 3) = bottomLeft1.x();
    a.coeffRef(5, 4) = bottomLeft1.y();
    a.coeffRef(5, 5) = 1;
    a.coeffRef(5, 6) = -bottomLeft2.y() * bottomLeft1.x();
    a.coeffRef(5, 7) = -bottomLeft2.y() * bottomLeft1.y();
    a.coeffRef(5, 8) = -bottomLeft2.y();
    // bottomRight
    a.coeffRef(6, 0) = bottomRight1.x();
    a.coeffRef(6, 1) = bottomRight1.y();
    a.coeffRef(6, 2) = 1;
    a.coeffRef(6, 6) = -bottomRight2.x() * bottomRight1.x();
    a.coeffRef(6, 7) = -bottomRight2.x() * bottomRight1.y();
    a.coeffRef(6, 8) = -bottomRight2.x();
    a.coeffRef(7, 3) = bottomRight1.x();
    a.coeffRef(7, 4) = bottomRight1.y();
    a.coeffRef(7, 5) = 1;
    a.coeffRef(7, 6) = -bottomRight2.y() * bottomRight1.x();
    a.coeffRef(7, 7) = -bottomRight2.y() * bottomRight1.y();
    a.coeffRef(7, 8) = -bottomRight2.y();
    a.coeffRef(8, 8) = 1;
    b.coeffRef(8) = 1;
//     dbgImage <<" a := { {" << a(0,0) <<" ," << a(0,1) <<" ," << a(0,2) <<" ," << a(0,3) <<" ," << a(0,4) <<" ," << a(0,5) <<" ," << a(0,6) <<" ," << a(0,7) <<" ," << a(0,8) <<" } , {" << a(1,0) <<" ," << a(1,1) <<" ," << a(1,2) <<" ," << a(1,3) <<" ," << a(1,4) <<" ," << a(1,5) <<" ," << a(1,6) <<" ," << a(1,7) <<" ," << a(1,8) <<" } , {" << a(2,0) <<" ," << a(2,1) <<" ," << a(2,2) <<" ," << a(2,3) <<" ," << a(2,4) <<" ," << a(2,5) <<" ," << a(2,6) <<" ," << a(2,7) <<" ," << a(2,8) <<" } , {" << a(3,0) <<" ," << a(3,1) <<" ," << a(3,2) <<" ," << a(3,3) <<" ," << a(3,4) <<" ," << a(3,5) <<" ," << a(3,6) <<" ," << a(3,7) <<" ," << a(3,8) <<" } , {" << a(4,0) <<" ," << a(4,1) <<" ," << a(4,2) <<" ," << a(4,3) <<" ," << a(4,4) <<" ," << a(4,5) <<" ," << a(4,6) <<" ," << a(4,7) <<" ," << a(4,8) <<" } , {" << a(5,0) <<" ," << a(5,1) <<" ," << a(5,2) <<" ," << a(5,3) <<" ," << a(5,4) <<" ," << a(5,5) <<" ," << a(5,6) <<" ," << a(5,7) <<" ," << a(5,8) <<" } , {" << a(6,0) <<" ," << a(6,1) <<" ," << a(6,2) <<" ," << a(6,3) <<" ," << a(6,4) <<" ," << a(6,5) <<" ," << a(6,6) <<" ," << a(6,7) <<" ," << a(6,8) <<" } , {"<< a(7,0) <<" ," << a(7,1) <<" ," << a(7,2) <<" ," << a(7,3) <<" ," << a(7,4) <<" ," << a(7,5) <<" ," << a(7,6) <<" ," << a(7,7) <<" ," << a(7,8) <<" } , {"<< a(8,0) <<" ," << a(8,1) <<" ," << a(8,2) <<" ," << a(8,3) <<" ," << a(8,4) <<" ," << a(8,5) <<" ," << a(8,6) <<" ," << a(8,7) <<" ," << a(8,8) <<" } };";
    Vector9qreal v;
    v = a.lu().solve(b);
    Matrix3qreal matrix;
    for (int r = 0; r < 3; r++) for (int c = 0; c < 3; c++) matrix.coeffRef(r, c) = v.coeff(3 * r + c);
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
