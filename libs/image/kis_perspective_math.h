/*
 * This file is part of Krita
 *
 *  SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_PERSPECTIVE_MATH_H_
#define _KIS_PERSPECTIVE_MATH_H_

#include "kis_vec.h"
#include <QPointF>
#include <Eigen/Geometry>

typedef Eigen::Matrix<qreal, 3, 3> Matrix3qreal;
typedef Eigen::Matrix<qreal, 9, 9> Matrix9qreal;
typedef Eigen::Matrix<qreal, 9, 1> Vector9qreal;
typedef Eigen::Hyperplane<qreal, 2> LineEquation;

#include <kritaimage_export.h>

class QRect;

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
