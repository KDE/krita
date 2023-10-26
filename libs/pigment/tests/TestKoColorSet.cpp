/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "TestKoColorSet.h"
#include <KoColorSet.h>
#include <KisGlobalResourcesInterface.h>
#include <simpletest.h>
#include <KisSwatch.h>
#include <KisSwatchGroup.h>
#include <kundo2stack.h>
#include <KoColorSpaceRegistry.h>

#ifndef FILES_DATA_DIR
#error "FILES_DATA_DIR not set. A directory with the data used for testing the importing of files in krita"
#endif

void TestKoColorSet::testLoadGPL()
{
    KoColorSet set(QString(FILES_DATA_DIR)+ "/gimp.gpl");
    QVERIFY(set.load(KisGlobalResourcesInterface::instance()));
    QCOMPARE(set.paletteType(), KoColorSet::GPL);

    QCOMPARE(set.colorCount(), 17);

    set.setFilename("test.gpl");
    QVERIFY(set.save());
    QVERIFY(set.filename() == "test.gpl");
    QCOMPARE(set.paletteType(), KoColorSet::GPL);

    KoColorSet set2("test.gpl");
    QVERIFY(set2.load(KisGlobalResourcesInterface::instance()));
    QCOMPARE(set2.paletteType(), KoColorSet::GPL);

}
void TestKoColorSet::testLoadRIFF()
{

    KoColorSet set(QString(FILES_DATA_DIR)+ "/ms_riff.pal");
    QVERIFY(set.load(KisGlobalResourcesInterface::instance()));
    QCOMPARE(set.paletteType(), KoColorSet::RIFF_PAL);

    QCOMPARE(set.colorCount(), 17);

    set.setFilename("test_riff.pal");
    QVERIFY(set.save());
    QVERIFY(set.filename() == "test_riff.pal");
    QCOMPARE(set.paletteType(), KoColorSet::RIFF_PAL);

    KoColorSet set2("test_riff.pal");
    QVERIFY(set2.load(KisGlobalResourcesInterface::instance()));
    QCOMPARE(set2.paletteType(), KoColorSet::KPL);
}
void TestKoColorSet::testLoadACT()
{
    KoColorSet set(QString(FILES_DATA_DIR)+ "/photoshop.act");
    QVERIFY(set.load(KisGlobalResourcesInterface::instance()));
    QCOMPARE(set.paletteType(), KoColorSet::ACT);
    QVERIFY(set.valid());

    QCOMPARE(set.colorCount(), 258);

    QFile("test.act").remove();

    set.setFilename("test.act");
    QVERIFY(set.save());

    KoColorSet set2("test.act");
    QVERIFY(set2.load(KisGlobalResourcesInterface::instance()));
    QCOMPARE(set2.paletteType(), KoColorSet::KPL);
}

void TestKoColorSet::testLoadPSP_PAL()
{
    KoColorSet set(QString(FILES_DATA_DIR)+ "/jasc.pal");
    QVERIFY(set.load(KisGlobalResourcesInterface::instance()));
    QCOMPARE(set.paletteType(), KoColorSet::PSP_PAL);
    QVERIFY(set.valid());

    QCOMPARE(set.colorCount(), 249);

    QFile("test_jasc.pal").remove();

    set.setFilename("test_jasc.pal");
    QVERIFY(set.save());

    KoColorSet set2("test_jasc.pal");
    QVERIFY(set2.load(KisGlobalResourcesInterface::instance()));
    QCOMPARE(set2.paletteType(), KoColorSet::KPL);


}
void TestKoColorSet::testLoadACO()
{
    KoColorSet set(QString(FILES_DATA_DIR)+ "/photoshop.aco");
    QVERIFY(set.load(KisGlobalResourcesInterface::instance()));
    QCOMPARE(set.paletteType(), KoColorSet::ACO);
    QVERIFY(set.valid());

    QCOMPARE(set.colorCount(), 18);

    QFile("test.aco").remove();

    set.setFilename("test.aco");
    QVERIFY(set.save());

    KoColorSet set2("test.aco");
    QVERIFY(set2.load(KisGlobalResourcesInterface::instance()));
    QCOMPARE(set2.paletteType(), KoColorSet::KPL);
}
void TestKoColorSet::testLoadXML()
{
    KoColorSet set(QString(FILES_DATA_DIR)+ "/scribus.xml");
    QVERIFY(set.load(KisGlobalResourcesInterface::instance()));
    QCOMPARE(set.paletteType(), KoColorSet::XML);
    QVERIFY(set.valid());

    QCOMPARE(set.colorCount(), 8);

    QFile("test.xml").remove();

    set.setFilename("test.xml");
    QVERIFY(set.save());

    KoColorSet set2("test.xml");
    QVERIFY(set2.load(KisGlobalResourcesInterface::instance()));
    QCOMPARE(set2.paletteType(), KoColorSet::KPL);
}
void TestKoColorSet::testLoadKPL()
{
    KoColorSet set(QString(FILES_DATA_DIR)+ "/krita.kpl");
    QVERIFY(set.load(KisGlobalResourcesInterface::instance()));
    QCOMPARE(set.paletteType(), KoColorSet::KPL);
    QVERIFY(set.valid());

    QCOMPARE(set.colorCount(), 0);

    QFile("test.kpl").remove();

    set.setFilename("test.kpl");
    QVERIFY(set.save());
    QCOMPARE(set.paletteType(), KoColorSet::KPL);

    KoColorSet set2("test.kpl");
    QVERIFY(set2.load(KisGlobalResourcesInterface::instance()));
    QCOMPARE(set2.paletteType(), KoColorSet::KPL);

}

void TestKoColorSet::testLoadSBZ_data()
{
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<int>("expectedColorCount");
    QTest::addColumn<QString>("sampleGroup");
    QTest::addColumn<QPoint>("samplePoint");
    QTest::addColumn<QString>("expectedColorModel");
    QTest::addColumn<QString>("expectedColorDepth");
    QTest::addColumn<QString>("expectedName");
    QTest::addColumn<QString>("expectedId");
    QTest::addColumn<QVector<float>>("expectedResult");

    QTest::newRow("swatchbook.sbz")
        << "swatchbook.sbz"
        << 5
        << "Sample swatch book" << QPoint(1, 0)
        << RGBAColorModelID.id()
        << Float32BitsColorDepthID.id()
        << "Cyan 100%"
        << "C10"
        << QVector<float>({0,1,1,1});

    QTest::newRow("sample_swatchbook.sbz")
        << "sample_swatchbook.sbz"
        << 47
        << "" << QPoint(5, 1)
        << CMYKAColorModelID.id()
        << Float32BitsColorDepthID.id()
        << "Cyan 90%"
        << "C09"
        << QVector<float>({90,0,0,0,1.0});
}

void TestKoColorSet::testLoadSBZ()
{
    QFETCH(QString, fileName);
    QFETCH(int, expectedColorCount);
    QFETCH(QString, sampleGroup);
    QFETCH(QPoint, samplePoint);
    QFETCH(QString, expectedColorModel);
    QFETCH(QString, expectedColorDepth);
    QFETCH(QString, expectedName);
    QFETCH(QString, expectedId);
    QFETCH(QVector<float>, expectedResult);

    KoColorSet set(QString(FILES_DATA_DIR) + QDir::separator() + fileName);
    QVERIFY(set.load(KisGlobalResourcesInterface::instance()));
    QCOMPARE(set.paletteType(), KoColorSet::SBZ);

    QCOMPARE(set.colorCount(), expectedColorCount);

#if 0
    qDebug() << set.swatchGroupNames();

    const int numColumns = set.getGroup(sampleGroup)->columnCount();
    const int numRows = set.getGroup(sampleGroup)->rowCount();

    for (int row = 0; row < numRows; row++) {
        for (int col = 0; col < numColumns; col++) {
            KisSwatch s = set.getSwatchFromGroup(col, row, sampleGroup);

            qDebug() << row << col << ppVar(s.name()) << ppVar(s.id()) << ppVar(s.color());
        }
    }
#endif

    KisSwatch s = set.getSwatchFromGroup(samplePoint.x(), samplePoint.y(), sampleGroup);
    QCOMPARE(s.name(), expectedName);
    QCOMPARE(s.id(), expectedId);

    KoColor c = s.color();
    QCOMPARE(c.colorSpace()->colorModelId().id(), expectedColorModel);
    QCOMPARE(c.colorSpace()->colorDepthId().id(), expectedColorDepth);

    const float *ptr = reinterpret_cast<float*>(c.data());
    for (int i = 0; i < expectedResult.size(); i++) {
        QCOMPARE(ptr[i], expectedResult[i]);
    }

    set.setFilename("test.sbz");
    QVERIFY(set.save());
    QVERIFY(set.filename() == "test.sbz");
    QCOMPARE(set.paletteType(), KoColorSet::SBZ);

    KoColorSet set2("test.sbz");
    QVERIFY(set2.load(KisGlobalResourcesInterface::instance()));
    QCOMPARE(set2.paletteType(), KoColorSet::KPL);
}

void TestKoColorSet::testLoadASE()
{
    KoColorSet set(QString(FILES_DATA_DIR) + "/photoshop.ase");
    QVERIFY(set.load(KisGlobalResourcesInterface::instance()));
    QCOMPARE(set.paletteType(), KoColorSet::ASE);

    QCOMPARE(set.colorCount(), 249);

    set.setFilename("test.ase");
    QVERIFY(set.save());
    QVERIFY(set.filename() == "test.ase");
    QCOMPARE(set.paletteType(), KoColorSet::ASE);

    KoColorSet set2("test.ase");
    QVERIFY(set2.load(KisGlobalResourcesInterface::instance()));
    QCOMPARE(set2.paletteType(), KoColorSet::KPL);
}

void TestKoColorSet::testLoadACB()
{
    KoColorSet set(QString(FILES_DATA_DIR) + "/photoshop.acb");
    QVERIFY(set.load(KisGlobalResourcesInterface::instance()));
    QCOMPARE(set.paletteType(), KoColorSet::ACB);

    QCOMPARE(set.colorCount(), 17);

    set.setFilename("test.acb");
    QVERIFY(set.save());
    QVERIFY(set.filename() == "test.acb");
    QCOMPARE(set.paletteType(), KoColorSet::ACB);

    KoColorSet set2("test.acb");
    QVERIFY(set2.load(KisGlobalResourcesInterface::instance()));
    QCOMPARE(set2.paletteType(), KoColorSet::KPL);
}

void TestKoColorSet::TestKoColorSet::testLock()
{
    KoColorSetSP cs = createColorSet();
    QVERIFY(!cs->isLocked());
    QVERIFY(!cs->isDirty());
    cs->setLocked(true);
    QVERIFY(!cs->isDirty());
    QVERIFY(cs->isLocked());
    QVERIFY(cs->getGlobalGroup()->colorCount() == 0);
    cs->addSwatch(KisSwatch(red(), "red"));
    QVERIFY(cs->getGlobalGroup()->colorCount() == 0);
    QVERIFY(!cs->isDirty());
    cs->setLocked(false);
    cs->addSwatch(KisSwatch(red(), "red"));
    QVERIFY(cs->getGlobalGroup()->colorCount() == 1);
    QVERIFY(cs->isDirty());
}

void TestKoColorSet::testColumnCount()
{
    KoColorSetSP cs = createColorSet();
    QVERIFY(cs->columnCount() == KisSwatchGroup::DEFAULT_COLUMN_COUNT);
    cs->setColumnCount(200);
    QVERIFY(cs->columnCount() == 200);
    QVERIFY(cs->isDirty());
    cs->undoStack()->undo();
    QVERIFY(cs->columnCount() == KisSwatchGroup::DEFAULT_COLUMN_COUNT);
    QVERIFY(!cs->isDirty());
    cs->undoStack()->redo();
    QVERIFY(cs->columnCount() == 200);
    QVERIFY(cs->isDirty());
}

void TestKoColorSet::testComment()
{
    KoColorSetSP cs = createColorSet();
    QVERIFY(cs->comment().isEmpty());
    cs->setComment("dummy");
    QVERIFY(cs->comment() == "dummy");
    QVERIFY(cs->isDirty());
    cs->undoStack()->undo();
    QVERIFY(cs->comment().isEmpty());
    QVERIFY(!cs->isDirty());
    cs->undoStack()->redo();
    QVERIFY(cs->comment() == "dummy");
    QVERIFY(cs->isDirty());
}

void TestKoColorSet::testPaletteType()
{
    KoColorSetSP cs = createColorSet();
    QVERIFY(cs->paletteType() == KoColorSet::KPL);
    QCOMPARE(QFileInfo(cs->filename()).suffix().toLower(), "kpl");

    cs->setPaletteType(KoColorSet::GPL);
    QVERIFY(cs->paletteType() == KoColorSet::GPL);
    QCOMPARE(QFileInfo(cs->filename()).suffix().toLower(), "gpl");
    QVERIFY(cs->isDirty());

    cs->undoStack()->undo();
    QVERIFY(cs->paletteType() == KoColorSet::KPL);
    QCOMPARE(QFileInfo(cs->filename()).suffix().toLower(), "kpl");
    QVERIFY(!cs->isDirty());

    cs->undoStack()->redo();
    QVERIFY(cs->paletteType() == KoColorSet::GPL);
    QCOMPARE(QFileInfo(cs->filename()).suffix().toLower(), "gpl");
    QVERIFY(cs->isDirty());
}

void TestKoColorSet::testAddSwatch()
{
    KoColorSetSP cs = createColorSet();
    cs->addSwatch(KisSwatch(red(), "red"));
    QVERIFY(cs->getGlobalGroup()->colorCount() == 1);
    QVERIFY(cs->isDirty());

    cs->undoStack()->undo();
    QVERIFY(cs->getGlobalGroup()->colorCount() == 0);
    QVERIFY(!cs->isDirty());

    cs->undoStack()->redo();
    QVERIFY(cs->getGlobalGroup()->colorCount() == 1);
    QVERIFY(cs->isDirty());
}

void TestKoColorSet::testRemoveSwatch()
{
    KoColorSetSP cs = createColorSet();
    cs->addSwatch(KisSwatch(red()), KoColorSet::GLOBAL_GROUP_NAME, 5, 5);
    KisSwatch sw = cs->getSwatchFromGroup(5, 5, KoColorSet::GLOBAL_GROUP_NAME);
    QVERIFY(sw.isValid());
    QColor c = sw.color().toQColor();
    QVERIFY(c == QColor(Qt::red));

    KisSwatchGroupSP group = cs->getGlobalGroup();
    QVERIFY(group);

    cs->removeSwatch(5, 5, group);
    QVERIFY(cs->isDirty());
    sw = cs->getSwatchFromGroup(5, 5, KoColorSet::GLOBAL_GROUP_NAME);
    QVERIFY(!sw.isValid());

    cs->undoStack()->undo();
    sw = cs->getSwatchFromGroup(5, 5, KoColorSet::GLOBAL_GROUP_NAME);
    QVERIFY(sw.isValid());
    c = sw.color().toQColor();
    QVERIFY(c == QColor(Qt::red));

    QVERIFY(cs->isDirty());

    // Undo the initial addSwatch
    cs->undoStack()->undo();
    QVERIFY(!cs->isDirty());
}

void TestKoColorSet::testAddGroup()
{
    KoColorSetSP cs = createColorSet();
    cs->addGroup("newgroup");
    QVERIFY(cs->swatchGroupNames().size() == 2);
    QVERIFY(cs->swatchGroupNames().contains(KoColorSet::GLOBAL_GROUP_NAME));
    QVERIFY(cs->swatchGroupNames().contains("newgroup"));
    QVERIFY(cs->isDirty());

    cs->undoStack()->undo();
    QVERIFY(cs->swatchGroupNames().size() == 1);
    QVERIFY(cs->swatchGroupNames().contains(KoColorSet::GLOBAL_GROUP_NAME));
    QVERIFY(!cs->swatchGroupNames().contains("newgroup"));
    QVERIFY(!cs->isDirty());

    cs->undoStack()->redo();
    QVERIFY(cs->swatchGroupNames().size() == 2);
    QVERIFY(cs->swatchGroupNames().contains(KoColorSet::GLOBAL_GROUP_NAME));
    QVERIFY(cs->swatchGroupNames().contains("newgroup"));
    QVERIFY(cs->isDirty());

    cs->addGroup("newgroup2");

    QCOMPARE(cs->getGroup(11)->name(), "");
    QCOMPARE(cs->getGroup(21)->name(), "newgroup");
    QCOMPARE(cs->getGroup(50)->name(), "newgroup2");

}


void TestKoColorSet::testChangeGroupName()
{
    KoColorSetSP cs = createColorSet();
    cs->addGroup("newgroup");
    QVERIFY(cs->swatchGroupNames().size() == 2);
    QVERIFY(cs->swatchGroupNames().contains(KoColorSet::GLOBAL_GROUP_NAME));
    QVERIFY(cs->swatchGroupNames().contains("newgroup"));
    QVERIFY(cs->isDirty());

    cs->changeGroupName("newgroup", "newnewgroup");
    QVERIFY(cs->swatchGroupNames().size() == 2);
    QVERIFY(cs->swatchGroupNames().contains(KoColorSet::GLOBAL_GROUP_NAME));
    QVERIFY(cs->swatchGroupNames().contains("newnewgroup"));
    QVERIFY(cs->isDirty());

    cs->undoStack()->undo();
    QVERIFY(cs->swatchGroupNames().size() == 2);
    QVERIFY(cs->swatchGroupNames().contains(KoColorSet::GLOBAL_GROUP_NAME));
    QVERIFY(cs->swatchGroupNames().contains("newgroup"));
    QVERIFY(cs->isDirty());

    cs->undoStack()->redo();
    QVERIFY(cs->swatchGroupNames().size() == 2);
    QVERIFY(cs->swatchGroupNames().contains(KoColorSet::GLOBAL_GROUP_NAME));
    QVERIFY(cs->swatchGroupNames().contains("newnewgroup"));
    QVERIFY(cs->isDirty());

    cs->undoStack()->undo();
    cs->undoStack()->undo();

    QVERIFY(cs->swatchGroupNames().size() == 1);
    QVERIFY(cs->swatchGroupNames().contains(KoColorSet::GLOBAL_GROUP_NAME));
    QVERIFY(!cs->swatchGroupNames().contains("newgroup"));
    QVERIFY(!cs->isDirty());
}


void TestKoColorSet::testMoveGroup()
{
    KoColorSetSP cs = createColorSet();
    cs->addGroup("group1");
    cs->addGroup("group2");
    cs->addGroup("group3");
    cs->addGroup("group4");

    QStringList original({"", "group1", "group2", "group3", "group4"});
    QCOMPARE(cs->swatchGroupNames(), original);

    cs->moveGroup("group3", "group2");

    QStringList move3({"", "group1", "group3", "group2", "group4"});
    QCOMPARE(cs->swatchGroupNames(), move3);

    cs->undoStack()->undo();
    QCOMPARE(cs->swatchGroupNames(), original);

    cs->undoStack()->redo();
    QCOMPARE(cs->swatchGroupNames(), move3);
}

void TestKoColorSet::testRemoveGroup()
{
    KoColorSetSP cs = createColorSet();

    // Discard Colors

    cs->addGroup("newgroup");
    QVERIFY(cs->swatchGroupNames().size() == 2);
    QVERIFY(cs->swatchGroupNames().contains(KoColorSet::GLOBAL_GROUP_NAME));
    QVERIFY(cs->swatchGroupNames().contains("newgroup"));
    QVERIFY(cs->isDirty());

    cs->removeGroup("newgroup", false);
    QVERIFY(cs->swatchGroupNames().size() == 1);
    QVERIFY(cs->swatchGroupNames().contains(KoColorSet::GLOBAL_GROUP_NAME));
    QVERIFY(!cs->swatchGroupNames().contains("newgroup"));
    QVERIFY(cs->isDirty());

    cs->undoStack()->undo();
    QVERIFY(cs->swatchGroupNames().size() == 2);
    QVERIFY(cs->swatchGroupNames().contains(KoColorSet::GLOBAL_GROUP_NAME));
    QVERIFY(cs->swatchGroupNames().contains("newgroup"));
    QVERIFY(cs->isDirty());

    cs->undoStack()->redo();
    QVERIFY(cs->swatchGroupNames().size() == 1);
    QVERIFY(cs->swatchGroupNames().contains(KoColorSet::GLOBAL_GROUP_NAME));
    QVERIFY(!cs->swatchGroupNames().contains("newgroup"));
    QVERIFY(cs->isDirty());

    cs->undoStack()->undo();
    cs->undoStack()->undo();

    QVERIFY(cs->swatchGroupNames().size() == 1);
    QVERIFY(cs->swatchGroupNames().contains(KoColorSet::GLOBAL_GROUP_NAME));
    QVERIFY(!cs->swatchGroupNames().contains("newgroup"));
    QVERIFY(!cs->isDirty());

    // Keep Colors

    cs->clear();
    cs->addGroup("newgroup");
    cs->addSwatch(KisSwatch(red()), "newgroup", 5, 5);

    QCOMPARE(cs->swatchGroupNames().size(), 2);
    QVERIFY(cs->swatchGroupNames().contains(KoColorSet::GLOBAL_GROUP_NAME));
    QVERIFY(cs->swatchGroupNames().contains("newgroup"));
    QCOMPARE(cs->colorCount(), 1);
    QVERIFY(cs->isDirty());

    cs->removeGroup("newgroup", true);
    QCOMPARE(cs->swatchGroupNames().size(), 1);
    QVERIFY(cs->swatchGroupNames().contains(KoColorSet::GLOBAL_GROUP_NAME));
    QVERIFY(!cs->swatchGroupNames().contains("newgroup"));
    QCOMPARE(cs->colorCount(), 1);
    QVERIFY(cs->isDirty());

    cs->undoStack()->undo();
    QCOMPARE(cs->swatchGroupNames().size(), 2);
    QVERIFY(cs->swatchGroupNames().contains(KoColorSet::GLOBAL_GROUP_NAME));
    QVERIFY(cs->swatchGroupNames().contains("newgroup"));
    QCOMPARE(cs->colorCount(), 2);
    QVERIFY(cs->isDirty());

    cs->undoStack()->redo();
    QCOMPARE(cs->swatchGroupNames().size(), 1);
    QVERIFY(cs->swatchGroupNames().contains(KoColorSet::GLOBAL_GROUP_NAME));
    QVERIFY(!cs->swatchGroupNames().contains("newgroup"));
    QCOMPARE(cs->colorCount(), 1);
    QVERIFY(cs->isDirty());

    cs->undoStack()->undo();
    cs->undoStack()->undo();
    cs->undoStack()->undo();

    QCOMPARE(cs->swatchGroupNames().size(), 1);
    QVERIFY(cs->swatchGroupNames().contains(KoColorSet::GLOBAL_GROUP_NAME));
    QVERIFY(!cs->swatchGroupNames().contains("newgroup"));
    QVERIFY(cs->isDirty());

    QCOMPARE(cs->colorCount(), 1);

    cs->undoStack()->undo();
    QCOMPARE(cs->colorCount(), 0);
    QVERIFY(!cs->isDirty());
}

void TestKoColorSet::testClear()
{
    KoColorSetSP cs = createColorSet();
    cs->addGroup("newgroup");
    cs->addSwatch(KisSwatch(red()), "newgroup", 5, 5);
    cs->addSwatch(KisSwatch(blue()), KoColorSet::GLOBAL_GROUP_NAME, 1, 1);

    QVERIFY(cs->colorCount() == 2);

    cs->clear();
    QVERIFY(cs->colorCount() == 0);
    QVERIFY(cs->swatchGroupNames().size() == 1);

    cs->undoStack()->undo();
    QVERIFY(cs->colorCount() == 2);
    QVERIFY(cs->swatchGroupNames().size() == 2);

    cs->undoStack()->redo();
    QVERIFY(cs->colorCount() == 0);
    QVERIFY(cs->swatchGroupNames().size() == 1);
}

void TestKoColorSet::testGetSwatchFromGroup()
{
    KoColorSetSP cs = createColorSet();
    cs->addGroup("newgroup");
    cs->addSwatch(KisSwatch(red()), "newgroup", 5, 5);
    cs->addSwatch(blue(), KoColorSet::GLOBAL_GROUP_NAME, 1, 1);

    KisSwatch sw = cs->getSwatchFromGroup(5, 5, "newgroup");
    QVERIFY(sw.isValid());
    QColor c = sw.color().toQColor();
    QVERIFY(c == QColor(Qt::red));

    sw = cs->getSwatchFromGroup(1, 1, KoColorSet::GLOBAL_GROUP_NAME);
    QVERIFY(sw.isValid());
    c = sw.color().toQColor();
    QVERIFY(c == QColor(Qt::blue));

    sw = cs->getSwatchFromGroup(1, 1, "newgroup");
    QVERIFY(!sw.isValid());
}

void TestKoColorSet::testIsGroupNameRow()
{
    KoColorSetSP cs = createColorSet();
    cs->getGlobalGroup()->setRowCount(7);
    cs->addGroup("group1", KisSwatchGroup::DEFAULT_COLUMN_COUNT, 6);
    cs->addGroup("group2", KisSwatchGroup::DEFAULT_COLUMN_COUNT, 5);

    for (int i = 0; i < cs->rowCountWithTitles(); ++i) {
        if (i == 7 || i == 14) {
            QVERIFY(cs->isGroupTitleRow(i));
        }
        else {
            QVERIFY(!cs->isGroupTitleRow(i));
        }
    }
}

void TestKoColorSet::testStartRowForNamedGroup()
{
    KoColorSetSP cs = createColorSet();
    cs->getGlobalGroup()->setRowCount(7);
    cs->addGroup("group1", KisSwatchGroup::DEFAULT_COLUMN_COUNT, 6);
    cs->addGroup("group2", KisSwatchGroup::DEFAULT_COLUMN_COUNT, 5);
    cs->addGroup("group3", KisSwatchGroup::DEFAULT_COLUMN_COUNT, 4);

    QCOMPARE(cs->rowCountWithTitles(), 25);


    QCOMPARE(cs->startRowForGroup(""), 0);
    QCOMPARE(cs->startRowForGroup("group1"), 7);
    QCOMPARE(cs->startRowForGroup("group2"), 14);
    QCOMPARE(cs->startRowForGroup("group3"), 20);
}

void TestKoColorSet::testGetClosestSwatchInfo()
{
    KoColorSetSP cs = createColorSet();
    cs->addSwatch(red(), "", 10, 5);
    QColor c;
    c.setRgb(255,10,10);
    KoColor  kc(c, KoColorSpaceRegistry::instance()->rgb8());
    KisSwatchGroup::SwatchInfo info = cs->getClosestSwatchInfo(kc);
    QCOMPARE(info.row, 5);
    QCOMPARE(info.column, 10);
    QCOMPARE(info.swatch.color().toQColor(), red().toQColor());
}

void TestKoColorSet::testGetGroup()
{
    KoColorSetSP cs = createColorSet();
    cs->getGlobalGroup()->setRowCount(7);
    cs->addGroup("group1", KisSwatchGroup::DEFAULT_COLUMN_COUNT, 6);
    cs->addGroup("group2", KisSwatchGroup::DEFAULT_COLUMN_COUNT, 5);

    QCOMPARE(cs->rowCount(), 18);
    QCOMPARE(cs->rowCountWithTitles(), 20);
    QVERIFY(cs->getGroup("group1"));
    QVERIFY(cs->getGroup("group2"));

    KisSwatchGroupSP grp = cs->getGroup(0);
    QVERIFY(grp);
    QVERIFY(grp->name().isEmpty());

    grp = cs->getGroup(7); // titlerow
    QVERIFY(grp);
    QCOMPARE(grp->name(), "group1");

    grp = cs->getGroup(8);
    QVERIFY(grp);
    QCOMPARE(grp->name(), "group1");

    grp = cs->getGroup(13); // last row
    QVERIFY(grp);
    QCOMPARE(grp->name(), "group1");

    grp = cs->getGroup(14); // titlerow
    QVERIFY(grp);
    QCOMPARE(grp->name(), "group2");

    grp = cs->getGroup(17);
    QVERIFY(grp);
    QCOMPARE(grp->name(), "group2");

    grp = cs->getGroup(19);
    QVERIFY(grp);
    QCOMPARE(grp->name(), "group2");

    grp = cs->getGroup(40);
    QVERIFY(grp.isNull());

}

void TestKoColorSet::testAllRows()
{
    KoColorSetSP cs = createColorSet();
    cs->getGlobalGroup()->setRowCount(7);
    cs->addGroup("group1", KisSwatchGroup::DEFAULT_COLUMN_COUNT, 6);
    cs->addGroup("group2", KisSwatchGroup::DEFAULT_COLUMN_COUNT, 5);

    QCOMPARE(cs->rowCountWithTitles(), 20);
    QVERIFY(cs->isGroupTitleRow(7));
    QVERIFY(cs->isGroupTitleRow(14));

    for (int i = 0; i < 21; ++i) {
        KisSwatchGroupSP grp = cs->getGroup(i);

        if (i < 7) {
            QVERIFY(grp->name().isEmpty());
        }
        else if (i > 6 && i < 14) {
            QCOMPARE(grp->name(), "group1");
        }
        else if (i > 13 && i < 20) {
            QCOMPARE(grp->name(), "group2");
        }
        else if (i > 19) {
            QVERIFY(grp.isNull());
        }
    }

}

void TestKoColorSet::testRowNumberInGroup()
{
    KoColorSetSP cs = createColorSet();
    cs->getGlobalGroup()->setRowCount(7);
    cs->addGroup("group1", KisSwatchGroup::DEFAULT_COLUMN_COUNT, 6);
    cs->addGroup("group2", KisSwatchGroup::DEFAULT_COLUMN_COUNT, 5);
    cs->addGroup("group3", KisSwatchGroup::DEFAULT_COLUMN_COUNT, 4);

    QCOMPARE(cs->rowCountWithTitles(), 25);

    QVector<int> rowCountsInGroup {    0, 1, 2, 3, 4, 5, 6,
                                   -1, 0, 1, 2, 3, 4, 5,
                                   -1, 0, 1, 2, 3, 4,
                                   -1, 0, 1, 2, 3};


    QCOMPARE(cs->rowCountWithTitles(), rowCountsInGroup.size());

    for (int i = 0; i < cs->rowCountWithTitles(); ++i) {
        QCOMPARE(cs->rowNumberInGroup(i), rowCountsInGroup[i]);
    }
}

void TestKoColorSet::testGetColorGlobal()
{
    KoColorSetSP cs = createColorSet();
    cs->getGlobalGroup()->setRowCount(7);
    cs->setColumnCount(3);
    cs->addGroup("group1", 3, 6);
    cs->addGroup("group2", 3, 5);
    cs->addGroup("group3", 3, 4);

    KisSwatch sw(red(), "red");

    Q_FOREACH(const QString &groupName, cs->swatchGroupNames()) {
        KisSwatchGroupSP group = cs->getGroup(groupName);
        //qDebug() << group->name();
        for (int row = 0; row < group->rowCount(); row++) {
            for (int col = 0; col < group->columnCount(); col++) {
                QColor c(row, col, 0);
                sw.setColor(KoColor(c, KoColorSpaceRegistry::instance()->rgb8()));
                //qDebug() << "rgb" << c.red() << c.green() << c.blue() << "row/col" << row << "," << col;
                cs->addSwatch(sw, groupName, col, row);
            }
        }
    }


    //qDebug() << "==================";

    for (int row = 0; row < cs->rowCountWithTitles(); row++) {
        if (row == 7 || row == 14 || row == 20) {
            QVERIFY(cs->isGroupTitleRow(row));
            //qDebug() << cs->getGroup(row)->name();
        }

        for (int col = 0; col < cs->columnCount(); col++) {
            if (row != 7 && row != 14 && row != 20) {
                QVERIFY(!cs->isGroupTitleRow(row));
                KisSwatch sw2 = cs->getColorGlobal(col, row);
                QColor c = sw2.color().toQColor();
                //qDebug() << "rgb" << c.red() << c.green() << c.blue() << "row/col" << cs->rowNumberInGroup(row) << "," << col;
                QCOMPARE(c.red(), cs->rowNumberInGroup(row));
                QCOMPARE(c.green(), col);
            }
        }
    }

}

KoColorSetSP TestKoColorSet::createColorSet()
{
    KoColorSetSP colorSet(new KoColorSet());
    colorSet->setPaletteType(KoColorSet::KPL);
    colorSet->setName("Dummy");
    colorSet->setFilename("dummy.kpl");
    colorSet->setModified(false);
    colorSet->undoStack()->clear();
    return colorSet;
}

KoColor TestKoColorSet::blue()
{
    QColor c(Qt::blue);
    KoColor  kc(c, KoColorSpaceRegistry::instance()->rgb8());
    return kc;
}

KoColor TestKoColorSet::red()
{
    QColor c(Qt::red);
    KoColor  kc(c, KoColorSpaceRegistry::instance()->rgb8());
    return kc;
}


SIMPLE_TEST_MAIN(TestKoColorSet)

