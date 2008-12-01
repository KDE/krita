/*
 *  Copyright (c) 2007 Boudewijn Rempt boud@valdyas.org
 *  Copyright (c) 2008 Benoit Jacob jacob.benoit.1@gmail.com
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

QTEST_KDEMAIN(KisPerspectiveMathTest, GUI)
#include "kis_perspective_math_test.moc"
