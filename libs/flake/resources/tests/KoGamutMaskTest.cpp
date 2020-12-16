/*
 *  SPDX-FileCopyrightText: 2019 Anna Medonosova <anna.medonosova@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */


#include <QTest>
#include <resources/KoGamutMask.h>

#include <testutil.h>

#include "KoGamutMaskTest.h"
#include <KisGlobalResourcesInterface.h>

KoGamutMaskTest::KoGamutMaskTest(QObject *parent) : QObject(parent)
{

}

void KoGamutMaskTest::testCoordIsClear()
{
    QFETCH(QString, maskFile);
    QFETCH(QPointF, coord);
    QFETCH(int, maskRotation);
    QFETCH(bool, expectedOutput);

    QScopedPointer<KoGamutMask> mask(new KoGamutMask(TestUtil::fetchDataFileLazy(maskFile)));
    mask->load(KisGlobalResourcesInterface::instance());
    Q_ASSERT(mask->valid());
    mask->setRotation(maskRotation);

    // for this test we have a hardcoded view size of 100
    QPointF translatedPoint = mask->viewToMaskTransform(100).map(coord);

    bool maskOutput = mask->coordIsClear(translatedPoint, false);
    QCOMPARE(maskOutput, expectedOutput);
}

void KoGamutMaskTest::testCoordIsClear_data()
{
    QTest::addColumn<QString>("maskFile");
    QTest::addColumn<QPointF>("coord");
    QTest::addColumn<int>("maskRotation");
    QTest::addColumn<bool>("expectedOutput");

    // single shape mask
    QTest::addRow("Atmospheric_Triad.kgm: disallowed coordinate, no rotation") << "Atmospheric_Triad.kgm"
                                                                        << QPointF(0.0, 0.0) << 0
                                                                        << false;

    QTest::addRow("Atmospheric_Triad.kgm: allowed coordinate, no rotation") << "Atmospheric_Triad.kgm"
                                                                          << QPointF(33.0, 71.0) << 0
                                                                          << true;

    QTest::addRow("Atmospheric_Triad.kgm: disallowed coordinate, with rotation") << "Atmospheric_Triad.kgm"
                                                                            << QPointF(33.0, 71.0) << 180
                                                                            << false;

    QTest::addRow("Atmospheric_Triad.kgm: allowed coordinate, with rotation") << "Atmospheric_Triad.kgm"
                                                                             << QPointF(76.4,60.9) << 180
                                                                             << true;


    // multiple shapes mask
    QTest::addRow("Dominant_Hue_With_Accent.kgm: allowed coordinate, shape 1, no rotation")
            << "Dominant_Hue_With_Accent.kgm"
            << QPointF(71.0, 49.0) << 0
            << true;

    QTest::addRow("Dominant_Hue_With_Accent.kgm: allowed coordinate, shape 2, no rotation")
            << "Dominant_Hue_With_Accent.kgm"
            << QPointF(11.0, 51.0) << 0
            << true;

    QTest::addRow("Dominant_Hue_With_Accent.kgm: allowed coordinate, shape 1, with rotation")
            << "Dominant_Hue_With_Accent.kgm"
            << QPointF(40.0, 21.0) << 256
            << true;

    QTest::addRow("Dominant_Hue_With_Accent.kgm: allowed coordinate, shape 2, with rotation")
            << "Dominant_Hue_With_Accent.kgm"
            << QPointF(57.0, 82.0) << 256
            << true;
}

void KoGamutMaskTest::testLoad()
{
    QFETCH(QString, maskFile);
    QFETCH(QString, expectedTitle);
    QFETCH(QString, expectedDescription);
    QFETCH(int, expectedShapeCount);

    QScopedPointer<KoGamutMask> mask(new KoGamutMask(TestUtil::fetchDataFileLazy(maskFile)));
    mask->load(KisGlobalResourcesInterface::instance());

    Q_ASSERT(mask->valid());

    QCOMPARE(mask->title(), expectedTitle);
    QCOMPARE(mask->description(), expectedDescription);
    QCOMPARE(mask->koShapes().size(), expectedShapeCount);
}

void KoGamutMaskTest::testLoad_data()
{
    QTest::addColumn<QString>("maskFile");
    QTest::addColumn<QString>("expectedTitle");
    QTest::addColumn<QString>("expectedDescription");
    QTest::addColumn<int>("expectedShapeCount");

    QTest::addRow("single shape mask")
            << "Atmospheric_Triad.kgm"
            << "Atmospheric Triad" << "test gamut mask description"
            << 1;

    QTest::addRow("multiple shape mask")
            << "Dominant_Hue_With_Accent.kgm"
            << "Dominant Hue With Accent" << ""
            << 2;
}

QTEST_MAIN(KoGamutMaskTest);
