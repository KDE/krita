/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "TestKoColorSet.h"
#include <KoColorSet.h>
#include <KisGlobalResourcesInterface.h>
#include <simpletest.h>

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
void TestKoColorSet::testLoadSBZ()
{
    KoColorSet set(QString(FILES_DATA_DIR)+ "/swatchbook.sbz");
    QVERIFY(set.load(KisGlobalResourcesInterface::instance()));
    QCOMPARE(set.paletteType(), KoColorSet::SBZ);

    QCOMPARE(set.colorCount(), 5);

    set.setFilename("test.sbz");
    QVERIFY(set.save());
    QVERIFY(set.filename() == "test.sbz");
    QCOMPARE(set.paletteType(), KoColorSet::SBZ);

    KoColorSet set2("test.sbz");
    QVERIFY(set2.load(KisGlobalResourcesInterface::instance()));
    QCOMPARE(set2.paletteType(), KoColorSet::KPL);
}

QTEST_GUILESS_MAIN(TestKoColorSet)
