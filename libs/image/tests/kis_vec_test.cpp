/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt boud @valdyas.org
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_vec_test.h"

#include <QTest>
#include "kis_vec.h"

void KisVecTest::testCreation()
{
    KisVector2D v2d = KisVector2D::Zero();
    QVERIFY(v2d.x() == 0.0);
    QVERIFY(v2d.y() == 0.0);
}


QTEST_MAIN(KisVecTest)
