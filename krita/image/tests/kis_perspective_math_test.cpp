/*
 *  Copyright (c) 2007 Boudewijn Rempt boud@valdyas.org
 *  Copyright (c) 2008 Benoit Jacob jacob@math.jussieu.fr
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

#include "kis_perspective_math_test.h"

#include <qtest_kde.h>
#include "kis_perspective_math.h"

#include <Eigen/Array>

void KisPerspectiveMathTest::testComputeMatrixTransfo()
{
    for (int i = 0; i < 10; i++) {
        QPointF topLeft = toQPointF(KisVector2D::Random());
        QPointF topRight = toQPointF(KisVector2D::Random());
        QPointF bottomLeft = toQPointF(KisVector2D::Random());
        QPointF bottomRight = toQPointF(KisVector2D::Random());
        Matrix3qreal originalMatrix = Matrix3qreal::Random();
        originalMatrix(2, 2) = 1;

        Matrix3qreal resultMatrix = KisPerspectiveMath::computeMatrixTransfo(
                                        topLeft, topRight, bottomLeft, bottomRight,
                                        KisPerspectiveMath::matProd(originalMatrix, topLeft),
                                        KisPerspectiveMath::matProd(originalMatrix, topRight),
                                        KisPerspectiveMath::matProd(originalMatrix, bottomLeft),
                                        KisPerspectiveMath::matProd(originalMatrix, bottomRight)
                                    );

        QVERIFY(resultMatrix.isApprox(originalMatrix));
    }
}


void KisPerspectiveMathTest::testLines()
{
    for (int i = 0; i < 10; i++) {
        KisVector2D center = KisVector2D::Random();
        KisVector2D u = KisVector2D::Random();
        KisVector2D v = KisVector2D::Random();
        qreal a = Eigen::ei_random<qreal>();

        QPointF pu  = toQPointF(center + u);
        QPointF pau = toQPointF(center + a * u);
        QPointF pv  = toQPointF(center + v);
        QPointF pav = toQPointF(center + a * v);

        LineEquation line_u(&pu, &pau);
        LineEquation line_v(&pv, &pav);

        // the line equations should be normalized so that a^2+b^2=1
        QVERIFY(Eigen::ei_isApprox(line_u.a()*line_u.a() + line_u.b()*line_u.b(), qreal(1)));
        QVERIFY(Eigen::ei_isApprox(line_v.a()*line_v.a() + line_v.b()*line_v.b(), qreal(1)));

        KisVector2D result = line_u.intersection(line_v);

        // the lines should intersect at the point we called "center"
        QVERIFY(result.isApprox(center));
    }
}


QTEST_KDEMAIN(KisPerspectiveMathTest, GUI)
#include "kis_perspective_math_test.moc"
