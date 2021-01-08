/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

    void testFindTrianglePoint();
};

#endif /* __KIS_ALGEBRA_2D_TEST_H */
