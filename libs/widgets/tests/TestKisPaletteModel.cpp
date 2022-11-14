/*
 *  SPDX-FileCopyrightText: 2020 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "TestKisPaletteModel.h"

#include <QVariant>
#include <QBrush>
#include <QColor>
#include <QList>

#include <simpletest.h>
#include <kundo2stack.h>

#include <KisSwatch.h>
#include <KisSwatchGroup.h>
#include <KisPaletteModel.h>

void TestKisPaletteModel::testSetColorSet()
{
    KoColorSetSP cs = createColorSet();
    KisPaletteModel model;
    model.setColorSet(cs);
    QVERIFY(model.colorSet());
}

void TestKisPaletteModel::testAddSwatch()
{
    KoColorSetSP cs = createColorSet();
    KisPaletteModel model;
    model.setColorSet(cs);

    QCOMPARE(model.rowCount(), 20);
    QCOMPARE(cs->rowCount(), 20);

    KisSwatch sw(red(), "red");
    model.addSwatch(sw);

    QCOMPARE(model.rowCount(), 20);
    QCOMPARE(cs->colorCount(), 1);
    QCOMPARE(cs->getGroup(QString())->colorCount(), 1);

    KisSwatchGroupSP grp = model.addGroup("newgroup");

    QCOMPARE(model.rowCount(), 41);
    QCOMPARE(cs->rowCount(), 40);
    QCOMPARE(cs->rowCountWithTitles(), 41);

    KisSwatch sw2(blue(), "blue");
    model.addSwatch(sw2, "newgroup");
    QCOMPARE(cs->colorCount(), 2);
    QCOMPARE(cs->getGroup("newgroup")->colorCount(), 1);
    QCOMPARE(grp->colorCount(), 1);
    QCOMPARE(cs->getGroup(QString())->colorCount(), 1);

    cs->undoStack()->undo(); // addSwatch
    cs->undoStack()->undo(); // addGroup
    QCOMPARE(model.rowCount(), 20);
}


void TestKisPaletteModel::testSetSwatch()
{
    KoColorSetSP cs = createColorSet();
    KisPaletteModel model;
    model.setColorSet(cs);

    KisSwatch sw(red(), "red");
    QModelIndex idx = model.index(10, 10, QModelIndex());
    model.setSwatch(sw, idx);

    QCOMPARE(cs->colorCount(), 1);

    KisSwatch sw2(blue(), "blue");
    idx = model.index(10, 10, QModelIndex());
    model.setSwatch(sw2, idx);

    QCOMPARE(cs->colorCount(), 1);

    idx = model.index(1, 1, QModelIndex());
    model.setSwatch(sw, idx);

    QCOMPARE(cs->colorCount(), 2);
}


void TestKisPaletteModel::testRemoveSwatch()
{
    KoColorSetSP cs = createColorSet();
    KisPaletteModel model;
    model.setColorSet(cs);

    KisSwatch sw(red(), "red");
    QModelIndex idx = model.index(10, 10, QModelIndex());
    model.setSwatch(sw, idx);

    QCOMPARE(cs->colorCount(), 1);

    model.removeSwatch(idx);
    QCOMPARE(cs->colorCount(), 0);
}


void TestKisPaletteModel::testChangeGroupName()
{
    KoColorSetSP cs = createColorSet();
    KisPaletteModel model;
    model.setColorSet(cs);
    model.addGroup("newgroup1");
    model.changeGroupName("newgroup1", "newnewgroup1");
    QVERIFY(cs->swatchGroupNames().contains("newnewgroup1"));
    QVERIFY(!cs->swatchGroupNames().contains("newgroup1"));

}

void TestKisPaletteModel::testRemoveGroup()
{
    KoColorSetSP cs = createColorSet();
    KisPaletteModel model;
    model.setColorSet(cs);
    model.addGroup("newgroup1");
    QVERIFY(cs->swatchGroupNames().contains("newgroup1"));
    model.removeGroup("newgroup1", false);
    QVERIFY(!cs->swatchGroupNames().contains("newgroup1"));
}


void TestKisPaletteModel::testAddGroup()
{
    KoColorSetSP cs = createColorSet();
    KisPaletteModel model;
    model.setColorSet(cs);

    QCOMPARE(model.rowCount(), 20);
    QCOMPARE(cs->rowCount(), 20);

    KisSwatchGroupSP grp1 = model.addGroup("newgroup");

    QCOMPARE(model.rowCount(), 41);
    QCOMPARE(cs->rowCount(), 40);
    QCOMPARE(cs->rowCountWithTitles(), 41);
    QCOMPARE(grp1->colorCount(), 0);

    KisSwatchGroupSP grp2 = model.addGroup("newgroup2");

    QCOMPARE(model.rowCount(), 62);
    QCOMPARE(cs->rowCount(), 60);
    QCOMPARE(cs->rowCountWithTitles(), 62);
    QCOMPARE(grp2->colorCount(), 0);

    cs->undoStack()->undo();
    QCOMPARE(model.rowCount(), 41);

    cs->undoStack()->undo();
    QCOMPARE(model.rowCount(), 20);

}

void TestKisPaletteModel::testSetRowCountForGroup()
{
    KoColorSetSP cs = createColorSet();
    KisPaletteModel model;
    model.setColorSet(cs);

    KisSwatchGroupSP group = model.addGroup("newgroup", 5, 5);
    QCOMPARE(group->rowCount(), 5);
    QCOMPARE(cs->rowCount(), 25);
    QCOMPARE(cs->rowCountWithTitles(), 26);

    model.setRowCountForGroup("newgroup", 10);
    QCOMPARE(group->rowCount(), 10);
    QCOMPARE(cs->rowCount(), 30);
    QCOMPARE(cs->rowCountWithTitles(), 31);
}


void TestKisPaletteModel::testClear()
{
    KoColorSetSP cs = createColorSet();
    KisPaletteModel model;
    model.setColorSet(cs);

    KisSwatchGroupSP grp = model.addGroup("newgroup");

    QCOMPARE(model.rowCount(), 41);
    QCOMPARE(cs->rowCount(), 40);
    QCOMPARE(cs->rowCountWithTitles(), 41);

    KisSwatch sw2(blue(), "blue");
    model.addSwatch(sw2, "newgroup");
    QCOMPARE(cs->colorCount(), 1);
    QCOMPARE(cs->getGroup("newgroup")->colorCount(), 1);
    QCOMPARE(grp->colorCount(), 1);
    QVERIFY(cs->swatchGroupNames().contains("newgroup"));

    model.clear();

    QVERIFY(!cs->swatchGroupNames().contains("newgroup"));
    QCOMPARE(cs->colorCount(), 0);
    QCOMPARE(model.rowCount(), KisSwatchGroup::DEFAULT_ROW_COUNT);
}

void TestKisPaletteModel::testIndexRowForInfo()
{
    KoColorSetSP cs = createColorSet();
    KisPaletteModel model;
    model.setColorSet(cs);

    KisSwatch sw(red(), "red");
    model.addGroup("group");
    QModelIndex idx = model.index(25, 10);
    model.setSwatch(sw, idx);

    QVERIFY(cs->getGroup("group"));
    QVERIFY(cs->getGroup("group")->infoList().size() > 0);

    KisSwatchGroup::SwatchInfo info = cs->getGroup("group")->infoList().first();

    QCOMPARE(info.row, 5);
    QCOMPARE(info.column, 10);
    QCOMPARE(info.swatch.color(), red());

    int row = model.indexRowForInfo(info);
    QCOMPARE(row, 25);

}

void TestKisPaletteModel::testIndexForClosest()
{
    KoColorSetSP cs = createColorSet();
    KisPaletteModel model;
    model.setColorSet(cs);

    KisSwatch sw(red(), "red");
    QModelIndex idx = model.index(5, 10, QModelIndex());
    model.setSwatch(sw, idx);

    QColor c;
    c.setRgb(255,10,10);
    KoColor  kc(c, KoColorSpaceRegistry::instance()->rgb8());

    KisSwatchGroup::SwatchInfo info = cs->getClosestSwatchInfo(kc);
    QCOMPARE(info.row, 5);
    QCOMPARE(info.column, 10);
    QCOMPARE(info.swatch.color().toQColor(), red().toQColor());

    QModelIndex idx2 = model.indexForClosest(kc);
    QCOMPARE(idx2.row(), 5);
    QCOMPARE(idx2.column(), 10);
    QColor c2 = idx2.data(Qt::BackgroundRole).value<QBrush>().color();
    QCOMPARE(c2, red().toQColor());
}



void TestKisPaletteModel::testData()
{
    KoColorSetSP cs = createColorSet();
    cs->getGlobalGroup()->setRowCount(7);
    KisPaletteModel model;
    model.setColorSet(cs);

    KisSwatch sw(red(), "red");
    QModelIndex idx = model.index(3, 5, QModelIndex());
    model.setSwatch(sw, idx);

    KisSwatch sw2 = cs->getSwatchFromGroup(5, 3);
    QCOMPARE(sw2.color(), sw.color());

    model.clear();
    cs->getGlobalGroup()->setRowCount(7);

    model.addGroup("group1", KisSwatchGroup::DEFAULT_COLUMN_COUNT, 6);
    model.addGroup("group2", KisSwatchGroup::DEFAULT_COLUMN_COUNT, 5);

    QCOMPARE(model.rowCount(), cs->rowCountWithTitles());

    int rowCount = model.rowCount();
    for (int row = 0; row < rowCount; ++row) {
        if (!cs->isGroupTitleRow(row)) {
            for (int column = 0; column < model.columnCount(); ++column) {
                QModelIndex idx = model.index(row, column);
                model.setSwatch(sw, idx);
                if (model.getSwatch(idx).color().toQColor() != sw.color().toQColor()) {
                    qDebug() << "row" << row << "column" << column;
                }
                QCOMPARE(model.getSwatch(idx).color().toQColor(), sw.color().toQColor());
            }
        }
    }

    for (int row = 0; row < rowCount; ++row) {
        if (cs->isGroupTitleRow(row)) {
            QModelIndex idx = model.index(row, 0);
            KisSwatchGroupSP grp = cs->getGroup(row);
            QCOMPARE(grp->name(), idx.data());
            QVERIFY(idx.data(KisPaletteModel::GroupNameRole).toBool());
        }
        else {
            for (int column = 0; column < model.columnCount(); ++column) {
                QModelIndex idx = model.index(row, column);
                if (model.getSwatch(idx).color() != sw.color()) {
                    qDebug() << "FALSE" << row << column;
                }
                QCOMPARE(model.getSwatch(idx).color(), sw.color());

            }
        }
    }

}

void TestKisPaletteModel::testRowNumberInGroup()
{
    KoColorSetSP cs = createColorSet();
    cs->getGlobalGroup()->setRowCount(7);
    cs->addGroup("group1", KisSwatchGroup::DEFAULT_COLUMN_COUNT, 6);
    cs->addGroup("group2", KisSwatchGroup::DEFAULT_COLUMN_COUNT, 5);

    KisPaletteModel model;
    model.setColorSet(cs);

    KisSwatch sw(red(), "red");
    QModelIndex idx = model.index(5, 10, QModelIndex());
    model.setSwatch(sw, idx);

    int rowNumber = model.rowNumberInGroup(5);
    QCOMPARE(rowNumber, 5);

    model.addGroup("group");
    idx = model.index(5, 0);
    model.setSwatch(sw, idx);

    rowNumber = model.rowNumberInGroup(25);
    QCOMPARE(rowNumber, 5);

}


KoColorSetSP TestKisPaletteModel::createColorSet()
{
    KoColorSetSP colorSet(new KoColorSet());
    colorSet->setPaletteType(KoColorSet::KPL);
    colorSet->setName("Dummy");
    colorSet->setFilename("dummy.kpl");
    colorSet->setModified(false);
    colorSet->undoStack()->clear();
    colorSet->setValid(true);
    return colorSet;
}

KoColor TestKisPaletteModel::blue()
{
    QColor c(Qt::blue);
    KoColor  kc(c, KoColorSpaceRegistry::instance()->rgb8());
    return kc;
}

KoColor TestKisPaletteModel::red()
{
    QColor c(Qt::red);
    KoColor  kc(c, KoColorSpaceRegistry::instance()->rgb8());
    return kc;
}


SIMPLE_TEST_MAIN(TestKisPaletteModel)
