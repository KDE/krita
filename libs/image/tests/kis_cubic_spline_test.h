/*
 *  SPDX-FileCopyrightText: 2024 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_CUBIC_SPLINE_TEST_H_
#define _KIS_CUBIC_SPLINE_TEST_H_

#include <simpletest.h>

class KisCubicSplineTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testComparisonLegacyNew1();
    void testComparisonLegacyNew2();
    void testComparisonLegacyNew3();
    void testComparisonLegacyNew4();
};

#endif
