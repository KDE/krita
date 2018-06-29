#include "TestKisSwatchGroup.h"
#include <QtTest>

void TestKisSwatchGroup::testAddingOneEntry()
{
    KisSwatch e;
    e.setName("first");
    g.setEntry(e, 0, 0);
    QVERIFY(g.checkEntry(0, 0));
    QVERIFY(g.nRows() == 1);
    QVERIFY(!g.checkEntry(1, 2));
    QVERIFY(!g.checkEntry(10, 5));
    QVERIFY(g.getEntry(0, 0) == e);
    QVERIFY(g.nColors() == 1);
    testSwatches[QPair<int, int>(0, 0)] = e;
}

void TestKisSwatchGroup::testAddingMultipleEntries()
{
    KisSwatch e2;
    e2.setName("second");
    g.setEntry(e2, 9, 3);
    QVERIFY(g.nColumns() == 16);
    QVERIFY(g.checkEntry(9, 3));
    QVERIFY(!g.checkEntry(1, 2));
    QVERIFY(!g.checkEntry(10, 5));
    QVERIFY(g.nRows() == 4);
    QVERIFY(g.checkEntry(0, 0));
    QVERIFY(g.getEntry(0, 0).name() == "first");
    KisSwatch e3;
    e3.setName("third");
    g.setEntry(e3, 4, 12);
    QVERIFY(g.nColors() == 3);
    QVERIFY(g.nRows() == 13);
    QVERIFY(g.checkEntry(9, 3));
    QVERIFY(g.getEntry(9, 3).name() == "second");
    testSwatches[QPair<int, int>(9, 3)] = e2;
    testSwatches[QPair<int, int>(4, 12)] = e3;
}

void TestKisSwatchGroup::testReplaceEntries()
{
    KisSwatch e4;
    e4.setName("fourth");
    g.setEntry(e4, 0, 0);
    QVERIFY(g.nColors() == 3);
    QVERIFY(g.nRows() == 13);
    QVERIFY(g.checkEntry(0, 0));
    QVERIFY(g.getEntry(0, 0).name() == "fourth");
    testSwatches[QPair<int, int>(0, 0)] = e4;
}

void TestKisSwatchGroup::testRemoveEntries()
{
    testSwatches.remove(QPair<int, int>(9, 3));
}

void TestKisSwatchGroup::testChangeColumnNumber()
{
    /*
    g.setNColumns(20);
    QVERIFY(g.nColumns() == 20);
    Q_FOREACH(QPair<int, int> p, testSwatches.keys()) {
        QVERIFY(testSwatches[p] == g.getEntry(p.first, p.second));
    }
    g.setNColumns(10);
    */
}

QTEST_GUILESS_MAIN(TestKisSwatchGroup)
