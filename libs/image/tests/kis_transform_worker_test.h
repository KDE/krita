/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt boud @valdyas.org
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_TRANSFORM_WORKER_TEST_H
#define KIS_TRANSFORM_WORKER_TEST_H

#include <simpletest.h>

class KisTransformWorkerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void testCreation();

    void testMirrorX_Even();
    void testMirrorX_Odd();
    void testMirrorY_Even();
    void testMirrorY_Odd();

    void benchmarkMirrorX();
    void benchmarkMirrorY();

    void testOffset();
    void testMirrorTransactionX();
    void testMirrorTransactionY();
    void testIdentity();
    void testScaleUp();
    void testXScaleUp();
    void testYScaleUp();
    void testScaleDown();
    void testXScaleDown();
    void testYScaleDown();
    void testXShear();
    void testYShear();
    void testRotation();
    void testMatrices();
    void testRotationSpecialCases();
    void testScaleUp5times();
    void rotate90Left();
    void rotate90Right();
    void rotate180();

    void benchmarkScale();
    void benchmarkRotate();
    void benchmarkRotate1Q();
    void benchmarkShear();
    void benchmarkScaleRotateShear();

    void testPartialProcessing();

    void testXScaleUpPixelAlignment_data();
    void testXScaleUpPixelAlignment();

private:
    void generateTestImages();
};

#endif
