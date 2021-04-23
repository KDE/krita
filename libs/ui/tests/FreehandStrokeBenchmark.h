/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef FREEHANDSTROKEBENCHMARK_H
#define FREEHANDSTROKEBENCHMARK_H

#include <simpletest.h>

class FreehandStrokeBenchmark : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();

private Q_SLOTS:
    void testDefaultTip();
    void testSoftTip();
    void testGaussianTip();

    void testRectangularTip();
    void testRectGaussianTip();
    void testRectSoftTip();

    void testStampTip();

    void testColorsmudgeDefaultTip_dull_old_sa();
    void testColorsmudgeDefaultTip_dull_old_nsa();
    void testColorsmudgeDefaultTip_dull_new_sa();
    void testColorsmudgeDefaultTip_dull_new_nsa();
    void testColorsmudgeDefaultTip_smear_old_sa();
    void testColorsmudgeDefaultTip_smear_old_nsa();
    void testColorsmudgeDefaultTip_smear_new_sa();
    void testColorsmudgeDefaultTip_smear_new_nsa();

};

#endif // FREEHANDSTROKEBENCHMARK_H
