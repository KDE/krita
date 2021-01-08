/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_ALGEBRA_2D_TEST_H
#define __KIS_ALGEBRA_2D_TEST_H

#include <QtTest/QtTest>

class KisAlgebra2DTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testHalfPlane();
    void testOuterCircle();

    void testQuadraticEquation();
    void testIntersections();
    void testWeirdIntersections();

    void testMatrixDecomposition1();
    void testMatrixDecomposition2();

    void testDivisionWithFloor();
    void testDrawEllipse();

    void testNullRectProcessing();

    void testLineIntersections();
};

#endif /* __KIS_ALGEBRA_2D_TEST_H */
