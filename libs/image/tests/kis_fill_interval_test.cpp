/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_fill_interval_test.h"

#include <simpletest.h>

#include <floodfill/kis_fill_interval.h>

void KisFillIntervalTest::testIsValid()
{
    KisFillInterval i1;
    QVERIFY(!i1.isValid());

    KisFillInterval i2(0,1,0);
    QVERIFY(i2.isValid());

    i2.invalidate();
    QVERIFY(!i2.isValid());
}

SIMPLE_TEST_MAIN(KisFillIntervalTest)
