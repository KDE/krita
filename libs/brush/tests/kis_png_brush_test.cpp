/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt boud @valdyas.org
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_png_brush_test.h"

#include <simpletest.h>
#include <KoColorSpaceRegistry.h>
#include "../kis_png_brush.h"
#include <KisGlobalResourcesInterface.h>

void KisPngBrushTest::testLoading_data()
{
    QTest::addColumn<QString>("filename");
    QTest::addColumn<int>("expectedBrushType");
    QTest::addColumn<int>("expectedBrushApplication");
    QTest::addColumn<bool>("expectedIsImageType");

    QTest::newRow("bw-alpha-transp") << "bw-alpha-transp.png" << int(IMAGE) << int(ALPHAMASK) << true;
    QTest::newRow("bw-alpha-solid") << "bw-alpha-solid.png" << int(MASK) << int(ALPHAMASK) << false;
    QTest::newRow("bw-no-alpha-solid") << "bw-no-alpha-solid.png" << int(MASK) << int(ALPHAMASK) << false;
    QTest::newRow("color-alpha-solid") << "color-alpha-solid.png" << int(IMAGE) << int(LIGHTNESSMAP) << true;
    QTest::newRow("color-alpha-transp") << "color-alpha-transp.png" << int(IMAGE) << int(LIGHTNESSMAP) << true;
}

void KisPngBrushTest::testLoading()
{
    QFETCH(QString, filename);
    QFETCH(int, expectedBrushType);
    QFETCH(int, expectedBrushApplication);
    QFETCH(bool, expectedIsImageType);


    QScopedPointer<KisPngBrush> brush(new KisPngBrush(QString(FILES_DATA_DIR) + '/' + filename));
    bool res = brush->load(KisGlobalResourcesInterface::instance());
    QVERIFY(res);
    QVERIFY(!brush->brushTipImage().isNull());

    QCOMPARE(brush->brushType(), expectedBrushType);
    QCOMPARE(int(brush->brushApplication()), expectedBrushApplication);
    QCOMPARE(brush->isImageType(), expectedIsImageType);

    QVERIFY(brush->metadata().contains(KisBrush::brushTypeMetaDataKey));
    QCOMPARE(brush->metadata().value(KisBrush::brushTypeMetaDataKey).toBool(), brush->isImageType());
}

SIMPLE_TEST_MAIN(KisPngBrushTest)
