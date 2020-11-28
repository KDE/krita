/*
 *  SPDX-FileCopyrightText: 2007, 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_random_generator_test.h"

#include <QTest>
#include "kis_random_generator.h"

#include <math.h>

#include "kis_debug.h"

void KisRandomGeneratorTest::twoSeeds(quint64 seed1, quint64 seed2)
{
    KisRandomGenerator rand1(seed1);
    KisRandomGenerator rand2(seed2);
    for (int y = 0; y < 100; y++) {
        for (int x = 0; x < 100; x++) {
            QVERIFY(rand1.randomAt(x, y) != rand2.randomAt(x, y));
        }
    }
}

void KisRandomGeneratorTest::twoSeeds()
{
    twoSeeds(140, 1405);
    twoSeeds(140515215, 232351521LL);
    twoSeeds(470461, 848256);
    twoSeeds(189840, 353395);
    twoSeeds(719471126795LL, 566272);
    twoSeeds(154349154349LL, 847895847895LL);
}


void KisRandomGeneratorTest::twoCalls(quint64 seed)
{
    KisRandomGenerator rand1(seed);
    for (int y = 0; y < 100; y++) {
        for (int x = 0; x < 100; x++) {
            dbgKrita <<  rand1.randomAt(x, y) << rand1.randomAt(x, y);
            QCOMPARE(rand1.randomAt(x, y), rand1.randomAt(x, y));
            QVERIFY(fabs(rand1.doubleRandomAt(x, y) - rand1.doubleRandomAt(x, y)) < 1e-5);
        }
    }
}

void KisRandomGeneratorTest::twoCalls()
{
    twoCalls(5023325165LL);
    twoCalls(751461346LL);
    twoCalls(171463465LL);
    twoCalls(30014613460LL);
    twoCalls(5001463160LL);
    twoCalls(6013413451550LL);
}

void KisRandomGeneratorTest::testConstantness(quint64 seed)
{
    KisRandomGenerator rand1(seed);
    KisRandomGenerator rand2(seed);
    for (int y = 0; y < 100; y++) {
        for (int x = 0; x < 100; x++) {
            QCOMPARE(rand1.randomAt(x, y), rand2.randomAt(x, y));
            QVERIFY(fabs(rand1.doubleRandomAt(x, y) - rand2.doubleRandomAt(x, y)) < 1e-5);
        }
    }
}


void KisRandomGeneratorTest::testConstantness()
{
    testConstantness(50);
    testConstantness(75);
    testConstantness(175);
    testConstantness(3000);
    testConstantness(5000);
    testConstantness(6050);
}

#include <iostream>

void KisRandomGeneratorTest::testEvolution()
{
    int counter = 0;

    KisRandomGenerator randg(10000);
    for (int y = 0; y < 1024; y++) {
        for (int x = 0; x < 1024; x++) {
            quint64 number = randg.randomAt(x, y);

            for (int i = 0; i < 5; i++) {
                for (int j = 0; j < 5; j++) {
                    if (i != 0 || j != 0) {
                        quint64 number2 = randg.randomAt(x + i, y + j);
                        if (number == number2) {
                            counter++;
                        }
                    }
                }
            }
        }
    }
    // XXX: too many pixels are similar too close together?
    QVERIFY(counter == 0);
}

QTEST_MAIN(KisRandomGeneratorTest)
