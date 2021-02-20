/*
 *  SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_MASK_GENERATOR_TEST_H
#define KIS_MASK_GENERATOR_TEST_H

#include <QtTest>

class KisMaskGeneratorTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void testCircleSerialisation();
    void testSquareSerialisation();

    void testCopyCtorCircle();
    void testCopyCtorRect();

    void testCopyCtorCurveCircle();
    void testCopyCtorCurveRect();

    void testCopyCtorGaussCircle();
    void testCopyCtorGaussRect();
};

#endif
