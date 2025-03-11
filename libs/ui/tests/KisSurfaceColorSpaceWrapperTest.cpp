/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisSurfaceColorSpaceWrapperTest.h"

#include <simpletest.h>
#include <KisSurfaceColorSpaceWrapper.h>

void KisSurfaceColorSpaceWrapperTest::test()
{
    KisSurfaceColorSpaceWrapper w1;
    KisSurfaceColorSpaceWrapper w2(KisSurfaceColorSpaceWrapper::DefaultColorSpace);
    KisSurfaceColorSpaceWrapper w3(KisSurfaceColorSpaceWrapper::sRGBColorSpace);
    KisSurfaceColorSpaceWrapper w4(KisSurfaceColorSpaceWrapper::scRGBColorSpace);
    KisSurfaceColorSpaceWrapper w5(KisSurfaceColorSpaceWrapper::bt2020PQColorSpace);

    // test comparison operators
    QVERIFY(w1 == w2);
    QVERIFY(w1 != w3);

    // test direct conversion to QColorSpace
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QSurfaceFormat::ColorSpace cs4 = w4;
    QVERIFY(cs4 == QSurfaceFormat::scRGBColorSpace);
#else
    QColorSpace cs4 = w4;
    QVERIFY(cs4 == QColorSpace::SRgbLinear);
#endif

    // test indirect conversion to QColorSpace
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QVERIFY(w1 == QSurfaceFormat::DefaultColorSpace);
    QVERIFY(w2 == QSurfaceFormat::DefaultColorSpace);
    QVERIFY(w3 == QSurfaceFormat::sRGBColorSpace);
    QVERIFY(w4 == QSurfaceFormat::scRGBColorSpace);
    QVERIFY(w5 == QSurfaceFormat::bt2020PQColorSpace);
#else
    QVERIFY(w1 == QColorSpace());
    QVERIFY(w2 == QColorSpace());
    QVERIFY(w3 == QColorSpace::SRgb);
    QVERIFY(w4 == QColorSpace::SRgbLinear);
    QVERIFY(w5 == QColorSpace::Bt2100Pq);
#endif

    // test assignment
    w1 = w5;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QVERIFY(w1 == QSurfaceFormat::bt2020PQColorSpace);
#else
    QVERIFY(w1 == QColorSpace::Bt2100Pq);
#endif

    // test move assignment
    w1 = std::move(w4);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QVERIFY(w1 == QSurfaceFormat::scRGBColorSpace);
#else
    QVERIFY(w1 == QColorSpace::SRgbLinear);
#endif
}

void KisSurfaceColorSpaceWrapperTest::testConstruction() 
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    auto w2 = KisSurfaceColorSpaceWrapper::fromQtColorSpace(QSurfaceFormat::DefaultColorSpace);
    auto w3 = KisSurfaceColorSpaceWrapper::fromQtColorSpace(QSurfaceFormat::sRGBColorSpace);
    auto w4 = KisSurfaceColorSpaceWrapper::fromQtColorSpace(QSurfaceFormat::scRGBColorSpace);
    auto w5 = KisSurfaceColorSpaceWrapper::fromQtColorSpace(QSurfaceFormat::bt2020PQColorSpace);

    QVERIFY(w2 == QSurfaceFormat::DefaultColorSpace);
    QVERIFY(w3 == QSurfaceFormat::sRGBColorSpace);
    QVERIFY(w4 == QSurfaceFormat::scRGBColorSpace);
    QVERIFY(w5 == QSurfaceFormat::bt2020PQColorSpace);
#else
    auto w2 = KisSurfaceColorSpaceWrapper::fromQtColorSpace(QColorSpace());
    auto w3 = KisSurfaceColorSpaceWrapper::fromQtColorSpace(QColorSpace::SRgb);
    auto w4 = KisSurfaceColorSpaceWrapper::fromQtColorSpace(QColorSpace::SRgbLinear);
    auto w5 = KisSurfaceColorSpaceWrapper::fromQtColorSpace(QColorSpace::Bt2100Pq);

    QVERIFY(w2 == QColorSpace());
    QVERIFY(w3 == QColorSpace::SRgb);
    QVERIFY(w4 == QColorSpace::SRgbLinear);
    QVERIFY(w5 == QColorSpace::Bt2100Pq);
#endif
}

SIMPLE_TEST_MAIN(KisSurfaceColorSpaceWrapperTest)
