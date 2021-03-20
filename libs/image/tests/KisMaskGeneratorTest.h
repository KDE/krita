/*
 *  SPDX-FileCopyrightText: 2018 Iván Santa María <ghevan@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISMASKGENERATORBENCHMARK_H
#define KISMASKGENERATORBENCHMARK_H

#include <simpletest.h>

class KisMaskGeneratorTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testDefaultScalarMask();
    void testDefaultVectorMask();

    void testCircularGaussScalarMask();
    void testCircularGaussVectorMask();

    void testCircularSoftScalarMask();
    void testCircularSoftVectorMask();

    void testRectangularScalarMask();
    void testRectangularVectorMask();

    void testRectangularGaussScalarMask();
    void testRectangularGaussVectorMask();

    void testRectangularSoftScalarMask();
    void testRectangularSoftVectorMask();

};

#endif // KISMASKGENERATORBENCHMARK_H
