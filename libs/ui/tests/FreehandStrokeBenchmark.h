/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef FREEHANDSTROKEBENCHMARK_H
#define FREEHANDSTROKEBENCHMARK_H

#include <QtTest>

class FreehandStrokeBenchmark : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();

    void testDefaultTip();
    void testSoftTip();
    void testGaussianTip();

    void testRectangularTip();
    void testRectGaussianTip();
    void testRectSoftTip();

    void testStampTip();

    void testColorsmudgeDefaultTip();
};

#endif // FREEHANDSTROKEBENCHMARK_H
