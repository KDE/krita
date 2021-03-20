/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_FIXED_POINT_MATHS_TEST_H
#define __KIS_FIXED_POINT_MATHS_TEST_H

#include <simpletest.h>

class KisFixedPointMathsTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testOperators();
    void testOperatorsNegative();
    void testConversions();
    void testConversionsNegative();
};

#endif /* __KIS_FIXED_POINT_MATHS_TEST_H */
