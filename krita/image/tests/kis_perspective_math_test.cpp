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

typedef Eigen::Matrix<qreal, 2, 1> Vector2qreal;
QPointF convert(const Vector2qreal& v) { return QPointF(v.x(), v.y()); }

void KisPerspectiveMathTest::testComputeMatrixTransfo()
{
  for(int i = 0; i < 10; i++)
  {
    QPointF topLeft = convert(Vector2qreal::Random());
    QPointF topRight = convert(Vector2qreal::Random());
    QPointF bottomLeft = convert(Vector2qreal::Random());
    QPointF bottomRight = convert(Vector2qreal::Random());
    Matrix3qreal originalMatrix = Matrix3qreal::Random();
    originalMatrix(2,2) = 1;

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
  for(int i = 0; i < 10; i++)
  {
    Vector2qreal center = Vector2qreal::Random();
    Vector2qreal u = Vector2qreal::Random();
    Vector2qreal v = Vector2qreal::Random();
    qreal a = Eigen::ei_random<qreal>();

    QPointF pu  = convert(center + u);
    QPointF pau = convert(center + a*u);
    QPointF pv  = convert(center + v);
    QPointF pav = convert(center + a*v);

    KisPerspectiveMath::LineEquation line_u = KisPerspectiveMath::computeLineEquation(&pu, &pau);
    KisPerspectiveMath::LineEquation line_v = KisPerspectiveMath::computeLineEquation(&pv, &pav);

    // the line equations should be normalized so that a^2+b^2=1
    QVERIFY(Eigen::ei_isApprox(line_u.a*line_u.a+line_u.b*line_u.b, qreal(1)));
    QVERIFY(Eigen::ei_isApprox(line_v.a*line_v.a+line_v.b*line_v.b, qreal(1)));

    QPointF _result = KisPerspectiveMath::computeIntersection(line_u, line_v);
    Vector2qreal result(_result.x(), _result.y());

    // the lines should intersect at the point we called "center"
    QVERIFY(result.isApprox(center));
  }
}


QTEST_KDEMAIN(KisPerspectiveMathTest, GUI)
#include "kis_perspective_math_test.moc"
