#include "TestKisSwatchGroup.h"
#include <QtTest>

void TestKisSwatchGroup::testAddingOneEntry()
{
    KisSwatch e;
    e.setName("first");
    g.setEntry(e, 0, 0);
    QVERIFY(g.checkEntry(0, 0));
    QVERIFY(!g.checkEntry(1, 2));
    QVERIFY(!g.checkEntry(10, 5));
    QCOMPARE(g.getEntry(0, 0), e);
    QCOMPARE(g.colorCount(), 1);
    testSwatches[QPair<int, int>(0, 0)] = e;
}

void TestKisSwatchGroup::testAddingMultipleEntries()
{
    KisSwatch e2;
    e2.setName("second");
    g.setEntry(e2, 9, 3);
    QCOMPARE(g.columnCount(), 16);
    QVERIFY(g.checkEntry(9, 3));
    QVERIFY(!g.checkEntry(1, 2));
    QVERIFY(!g.checkEntry(10, 5));
    QVERIFY(g.checkEntry(0, 0));
    QCOMPARE(g.getEntry(0, 0).name(), QString("first"));
    KisSwatch e3;
    e3.setName("third");
    g.setEntry(e3, 4, 12);
    QCOMPARE(g.colorCount(), 3);
    QVERIFY(g.checkEntry(9, 3));
    QCOMPARE(g.getEntry(9, 3).name(), QString("second"));
    testSwatches[QPair<int, int>(9, 3)] = e2;
    testSwatches[QPair<int, int>(4, 12)] = e3;
}

void TestKisSwatchGroup::testReplaceEntries()
{
    KisSwatch e4;
    e4.setName("fourth");
    g.setEntry(e4, 0, 0);
    QCOMPARE(g.colorCount(), 3);
    QVERIFY(g.checkEntry(0, 0));
    QCOMPARE(g.getEntry(0, 0).name(), QString("fourth"));
    testSwatches[QPair<int, int>(0, 0)] = e4;
}

void TestKisSwatchGroup::testRemoveEntries()
{
    testSwatches.remove(QPair<int, int>(9, 3));
    QVERIFY(g.removeEntry(9, 3));
    QCOMPARE(g.colorCount(), testSwatches.size());
    QVERIFY(!g.removeEntry(13, 10));
    QVERIFY(!g.checkEntry(9, 3));
}

void TestKisSwatchGroup::testChangeColumnNumber()
{
    g.setColumnCount(20);
    QCOMPARE(g.columnCount(), 20);
    for (QPair<int, int> p : testSwatches.keys()) {
        QCOMPARE(testSwatches[p], g.getEntry(p.first, p.second));
    }
    g.setColumnCount(10);
    int keptCount = 0;
    for (QPair<int, int> p : testSwatches.keys()) {
        if (p.first < 10) {
            keptCount++;
            QCOMPARE(testSwatches[p], g.getEntry(p.first, p.second));
        }
    }
    QCOMPARE(keptCount, g.colorCount());
}

void TestKisSwatchGroup::testAddEntry()
{
    KisSwatchGroup g2;
    g2.setColumnCount(3);
    g2.setRowCount(1);
    for (int i = 0; i != 3; i++) {
        g2.addEntry(KisSwatch());
    }
    QCOMPARE(g2.rowCount(), 1);
    QCOMPARE(g2.columnCount(), 3);
    QCOMPARE(g2.colorCount(), 3);
    g2.addEntry(KisSwatch());
    QCOMPARE(g2.rowCount(), 2);
    QCOMPARE(g2.columnCount(), 3);
    QCOMPARE(g2.colorCount(), 4);
    g2.setRowCount(1);
    QCOMPARE(g2.rowCount(), 1);
    QCOMPARE(g2.columnCount(), 3);
    QCOMPARE(g2.colorCount(), 3);
    for (int i = 0; i != 4; i++) {
        g2.addEntry(KisSwatch());
    }
    QCOMPARE(g2.rowCount(), 3);
    QCOMPARE(g2.columnCount(), 3);
    QCOMPARE(g2.colorCount(), 7);
}

QTEST_GUILESS_MAIN(TestKisSwatchGroup)
