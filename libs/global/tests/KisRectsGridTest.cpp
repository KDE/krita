/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisRectsGridTest.h"

#include "KisRectsGrid.h"
#include "kis_debug.h"

void KisRectsGridTest::test()
{
    KisRectsGrid grid;

    QVector<QRect> result;

    result = grid.addRect(QRect(5,5,10,10));

    QCOMPARE(result, QVector<QRect>({QRect(0,0,64,64)}));

    QVERIFY(grid.contains(QRect(10,10,15,15)));
    QVERIFY(grid.contains(QRect(0,0,64,64)));
    QVERIFY(!grid.contains(QRect(64,10,1,1)));
    QVERIFY(!grid.contains(QRect(0,0,65,65)));
    QVERIFY(!grid.contains(QRect(64,64,1,1)));

    result = grid.addRect(QRect(5,5,128,10));

    QCOMPARE(result, QVector<QRect>({QRect(64,0,64,64), QRect(128,0,64,64)}));

    QVERIFY(grid.contains(QRect(10,10,15,15)));
    QVERIFY(grid.contains(QRect(0,0,64,64)));
    QVERIFY(grid.contains(QRect(64,10,1,1)));
    QVERIFY(!grid.contains(QRect(0,0,65,65)));
    QVERIFY(!grid.contains(QRect(64,64,1,1)));

    result = grid.removeRect(QRect(64,65,10,10));

    QVERIFY(result.isEmpty());
    QVERIFY(grid.contains(QRect(64,10,1,1)));

    result = grid.removeRect(QRect(64,0,64,64));

    //qDebug() << ppVar(result);

    QCOMPARE(result, QVector<QRect>({QRect(64,0,64,64)}));
    QVERIFY(!grid.contains(QRect(64,10,1,1)));
    QVERIFY(grid.contains(QRect(128,10,1,1)));

    result = grid.removeRect(QRect(64,-1,128,70));

    QCOMPARE(result, QVector<QRect>({QRect(128,0,64,64)}));
    QVERIFY(!grid.contains(QRect(64,10,1,1)));
    QVERIFY(!grid.contains(QRect(128,10,1,1)));
}

QTEST_MAIN(KisRectsGridTest)
