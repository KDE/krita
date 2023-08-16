/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "TestKisSwatchGroup.h"
#include <simpletest.h>
#include <KoColorSet.h>

void TestKisSwatchGroup::testAddingOneEntry()
{
    KisSwatch e;
    e.setName("first");
    g.setSwatch(e, 0, 0);
    QVERIFY(g.checkSwatchExists(0, 0));
    QVERIFY(!g.checkSwatchExists(1, 2));
    QVERIFY(!g.checkSwatchExists(10, 5));
    QCOMPARE(g.getSwatch(0, 0), e);
    QCOMPARE(g.colorCount(), 1);
    testSwatches[QPair<int, int>(0, 0)] = e;
}

void TestKisSwatchGroup::testAddingMultipleEntries()
{
    KisSwatch e2;
    e2.setName("second");
    g.setSwatch(e2, 9, 3);
    QCOMPARE(g.columnCount(), 16);
    QVERIFY(g.checkSwatchExists(9, 3));
    QVERIFY(!g.checkSwatchExists(1, 2));
    QVERIFY(!g.checkSwatchExists(10, 5));
    QVERIFY(g.checkSwatchExists(0, 0));
    QCOMPARE(g.getSwatch(0, 0).name(), QString("first"));
    KisSwatch e3;
    e3.setName("third");
    g.setSwatch(e3, 4, 12);
    QCOMPARE(g.colorCount(), 3);
    QVERIFY(g.checkSwatchExists(9, 3));
    QCOMPARE(g.getSwatch(9, 3).name(), QString("second"));
    testSwatches[QPair<int, int>(9, 3)] = e2;
    testSwatches[QPair<int, int>(4, 12)] = e3;
}

void TestKisSwatchGroup::testReplaceEntries()
{
    KisSwatch e4;
    e4.setName("fourth");
    g.setSwatch(e4, 0, 0);
    QCOMPARE(g.colorCount(), 3);
    QVERIFY(g.checkSwatchExists(0, 0));
    QCOMPARE(g.getSwatch(0, 0).name(), QString("fourth"));
    testSwatches[QPair<int, int>(0, 0)] = e4;
}

void TestKisSwatchGroup::testRemoveEntries()
{
    testSwatches.remove(QPair<int, int>(9, 3));
    QVERIFY(g.removeSwatch(9, 3));
    QCOMPARE(g.colorCount(), testSwatches.size());
    QVERIFY(!g.removeSwatch(13, 10));
    QVERIFY(!g.checkSwatchExists(9, 3));
}

void TestKisSwatchGroup::testChangeColumnNumber()
{
    g.setColumnCount(20);
    QCOMPARE(g.columnCount(), 20);
    for (QPair<int, int> p : testSwatches.keys()) {
        QCOMPARE(testSwatches[p], g.getSwatch(p.first, p.second));
    }
    g.setColumnCount(10);
    int keptCount = 0;
    for (QPair<int, int> p : testSwatches.keys()) {
        if (p.first < 10) {
            keptCount++;
            QCOMPARE(testSwatches[p], g.getSwatch(p.first, p.second));
        }
    }
    QCOMPARE(keptCount, g.colorCount());
}

void TestKisSwatchGroup::testAddEntry()
{
    KisSwatchGroup g2;
    g2.setColumnCount(3);
    g2.setRowCount(1);

    g2.addSwatch(KisSwatch(KoColor()));
    g2.addSwatch(KisSwatch(KoColor()));
    g2.addSwatch(KisSwatch(KoColor()));

    QCOMPARE(g2.rowCount(), 1);
    QCOMPARE(g2.columnCount(), 3);
    QCOMPARE(g2.colorCount(), 3);

    g2.addSwatch(KisSwatch(KoColor()));

    QCOMPARE(g2.rowCount(), 2);
    QCOMPARE(g2.columnCount(), 3);
    QCOMPARE(g2.colorCount(), 4);

    g2.setRowCount(1);
    QCOMPARE(g2.rowCount(), 1);
    QCOMPARE(g2.columnCount(), 3);
    QCOMPARE(g2.colorCount(), 3);

    g2.addSwatch(KisSwatch(KoColor()));
    g2.addSwatch(KisSwatch(KoColor()));
    g2.addSwatch(KisSwatch(KoColor()));
    g2.addSwatch(KisSwatch(KoColor()));

    QCOMPARE(g2.rowCount(), 3);
    QCOMPARE(g2.columnCount(), 3);
    QCOMPARE(g2.colorCount(), 7);
}

void TestKisSwatchGroup::testName()
{
    KisSwatchGroup g2;
    QCOMPARE(g2.name(), "");
    g2.setName("test");
    QCOMPARE(g2.name(), "test");
}

void TestKisSwatchGroup::testColorCount()
{
    KisSwatchGroup g2;
    g2.addSwatch(KisSwatch(red(), "red"));
    QCOMPARE(g2.colorCount(), 1);
    g2.addSwatch(KisSwatch(blue(), "blue"));
    QCOMPARE(g2.colorCount(), 2);
    g2.removeSwatch(0, 0);
    QCOMPARE(g2.colorCount(), 1);
    g2.removeSwatch(1, 0);
    QCOMPARE(g2.colorCount(), 0);
}

void TestKisSwatchGroup::testInfoList()
{
    KisSwatchGroup g2;
    g2.addSwatch(KisSwatch(red(), "red"));
    QCOMPARE(g2.infoList().size(), 1);
    KisSwatchGroup::SwatchInfo info = g2.infoList().first();
    QCOMPARE(info.column, 0);
    QCOMPARE(info.row, 0);
    QCOMPARE(info.group, KoColorSet::GLOBAL_GROUP_NAME);
    QCOMPARE(info.swatch.color(), red());

    g2.addSwatch(KisSwatch(blue(), "blue"));
    QCOMPARE(g2.infoList().size(), 2);
    info = g2.infoList().at(1);
    QCOMPARE(info.column, 1);
    QCOMPARE(info.row, 0);
    QCOMPARE(info.group, KoColorSet::GLOBAL_GROUP_NAME);
    QCOMPARE(info.swatch.color(), blue());
}

KoColor TestKisSwatchGroup::blue()
{
    QColor c(Qt::blue);
    KoColor  kc(c, KoColorSpaceRegistry::instance()->rgb8());
    return kc;
}

KoColor TestKisSwatchGroup::red()
{
    QColor c(Qt::red);
    KoColor  kc(c, KoColorSpaceRegistry::instance()->rgb8());
    return kc;
}


SIMPLE_TEST_MAIN(TestKisSwatchGroup)
