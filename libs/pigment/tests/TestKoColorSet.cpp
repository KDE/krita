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

}
void TestKoColorSet::testLoadACT()
{
    KoColorSet set(QString(FILES_DATA_DIR)+ "/photoshop.act");
    QVERIFY(set.load(KisGlobalResourcesInterface::instance()));
    QCOMPARE(set.paletteType(), KoColorSet::ACT);
    QVERIFY(set.valid());

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

}

QTEST_GUILESS_MAIN(TestKoColorSet)
