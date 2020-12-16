/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_CAGE_TRANSFORM_WORKER_TEST_H
#define __KIS_CAGE_TRANSFORM_WORKER_TEST_H

#include <QtTest>

class KisCageTransformWorkerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCageClockwise();
    void testCageClockwisePrepareOnly();
    void testCageClockwisePixePrecision4();
    void testCageClockwisePixePrecision8QImage();
    void testCageCounterclockwise();
    void testCageClockwiseUnity();
    void testCageCounterclockwiseUnity();

    void stressTestRandomCages();

    void testUnityGreenCoordinates();

    void testTransformAsBase();
    void testAngleBetweenVectors();
};

#endif /* __KIS_CAGE_TRANSFORM_WORKER_TEST_H */
