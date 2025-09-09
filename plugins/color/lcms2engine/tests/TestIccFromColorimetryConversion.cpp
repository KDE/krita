/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "TestIccFromColorimetryConversion.h"

#include <lcms2.h>

#include <KoColorProfile.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <simpletest.h>
#include <testpigment.h>

#include <surfacecolormanagement/KisSurfaceColorimetry.h>
#include <surfacecolormanagement/KisSurfaceColorimetryIccUtils.h>

#include <kis_debug.h>


void TestIccFromColorimetryConversion::testRequestConstruction_data()
{
    QTest::addColumn<KisSurfaceColorimetry::ColorSpace>("colorSpace");
    QTest::addColumn<bool>("isValid");
    QTest::addColumn<ColorPrimaries>("expectedPrimaries");
    QTest::addColumn<TransferCharacteristics>("expectedTransferFunction");

    using KisSurfaceColorimetry::ColorSpace;
    using KisSurfaceColorimetry::NamedPrimaries;
    using KisSurfaceColorimetry::NamedTransferFunction;

    auto makeCS = [] (NamedPrimaries p, NamedTransferFunction tf) {
        ColorSpace cs;
        cs.primaries = p;
        cs.transferFunction = tf;
        return cs;
    };

    QTest::newRow("srgb")
        << makeCS(NamedPrimaries::primaries_srgb, NamedTransferFunction::transfer_function_srgb)
        << true
        << PRIMARIES_ITU_R_BT_709_5
        << TRC_IEC_61966_2_1;

    QTest::newRow("srgb-linear")
        << makeCS(NamedPrimaries::primaries_srgb, NamedTransferFunction::transfer_function_ext_linear)
        << true
        << PRIMARIES_ITU_R_BT_709_5
        << TRC_LINEAR;

    QTest::newRow("srgb-2.2")
        << makeCS(NamedPrimaries::primaries_srgb, NamedTransferFunction::transfer_function_gamma22)
        << true
        << PRIMARIES_ITU_R_BT_709_5
        << TRC_ITU_R_BT_470_6_SYSTEM_M;

    QTest::newRow("srgb-2.8")
        << makeCS(NamedPrimaries::primaries_srgb, NamedTransferFunction::transfer_function_gamma28)
        << true
        << PRIMARIES_ITU_R_BT_709_5
        << TRC_ITU_R_BT_470_6_SYSTEM_B_G;

    QTest::newRow("bt2020-linear")
        << makeCS(NamedPrimaries::primaries_bt2020, NamedTransferFunction::transfer_function_ext_linear)
        << true
        << PRIMARIES_ITU_R_BT_2020_2_AND_2100_0
        << TRC_LINEAR;

    QTest::newRow("bt2020-pq")
        << makeCS(NamedPrimaries::primaries_bt2020, NamedTransferFunction::transfer_function_st2084_pq)
        << true
        << PRIMARIES_ITU_R_BT_2020_2_AND_2100_0
        << TRC_ITU_R_BT_2100_0_PQ;

    QTest::newRow("unknown-srgb")
        << makeCS(NamedPrimaries::primaries_unknown, NamedTransferFunction::transfer_function_srgb)
        << false
        << PRIMARIES_UNSPECIFIED
        << TRC_IEC_61966_2_1;

    QTest::newRow("srgb-unknown")
        << makeCS(NamedPrimaries::primaries_srgb, NamedTransferFunction::transfer_function_unknown)
        << false
        << PRIMARIES_ITU_R_BT_709_5
        << TRC_UNSPECIFIED;

    // any pq-space that is not bt2020pq is considered unsupported

    QTest::newRow("srgb-pq")
        << makeCS(NamedPrimaries::primaries_srgb, NamedTransferFunction::transfer_function_st2084_pq)
        << false
        << PRIMARIES_ITU_R_BT_709_5
        << TRC_UNSPECIFIED;

    QTest::newRow("adobergb-pq")
        << makeCS(NamedPrimaries::primaries_adobe_rgb, NamedTransferFunction::transfer_function_st2084_pq)
        << false
        << PRIMARIES_ADOBE_RGB_1998
        << TRC_UNSPECIFIED;
}

void TestIccFromColorimetryConversion::testRequestConstruction()
{
    QFETCH(KisSurfaceColorimetry::ColorSpace, colorSpace);
    QFETCH(bool, isValid);
    QFETCH(ColorPrimaries, expectedPrimaries);
    QFETCH(TransferCharacteristics, expectedTransferFunction);

    auto request = KisSurfaceColorimetry::colorSpaceToRequest(colorSpace);

    QCOMPARE(request.isValid(), isValid);
    QCOMPARE(request.colorPrimariesType, expectedPrimaries);
    QCOMPARE(request.transferFunction, expectedTransferFunction);
    QVERIFY(request.colorants.isEmpty());
}

void TestIccFromColorimetryConversion::testRequestConstructionCustomPrimaries()
{
    using KisColorimetryUtils::Colorimetry;
    using KisSurfaceColorimetry::ColorSpace;
    using KisSurfaceColorimetry::NamedTransferFunction;

    ColorSpace  colorSpace;
    colorSpace.primaries = Colorimetry::BT709;
    colorSpace.transferFunction = NamedTransferFunction::transfer_function_srgb;
    const QVector<double> expectedColorants = {
        0.3127, 0.3290, // white
        0.64, 0.33, // red
        0.30, 0.60, // green
        0.15, 0.06, // blue
    };

    auto request = KisSurfaceColorimetry::colorSpaceToRequest(colorSpace);

    QCOMPARE(request.isValid(), true);
    QCOMPARE(request.colorPrimariesType, PRIMARIES_UNSPECIFIED);
    QCOMPARE(request.transferFunction, TRC_IEC_61966_2_1);
    QCOMPARE(request.colorants.size(), expectedColorants.size());

    auto it =
        std::mismatch(request.colorants.begin(), request.colorants.end(),
                      expectedColorants.begin(), qOverload<double, double>(qFuzzyCompare));

    if (it.first != request.colorants.end()) {
        qWarning() << Qt::fixed << qSetRealNumberPrecision(8)
            <<  "Failed to compare colorants array, result:" << *it.first << "expected:" <<*it.second;
        qWarning() << Qt::fixed << qSetRealNumberPrecision(8)
            << "    " << ppVar(request.colorants);
        qWarning() << Qt::fixed << qSetRealNumberPrecision(8)
            << "    " << ppVar(expectedColorants);
        QFAIL("check failed");
    }

}

void TestIccFromColorimetryConversion::testRequestConstructionCustomGamma_data()
{
    QTest::addColumn<qreal>("gamma");
    QTest::addColumn<bool>("isValid");
    QTest::addColumn<TransferCharacteristics>("expectedTransferFunction");

    QTest::newRow("g1.0") << 1.0 << true << TRC_LINEAR;
    QTest::newRow("g1.8") << 1.8 << true << TRC_GAMMA_1_8;
    QTest::newRow("g2.2") << 2.2 << true << TRC_ITU_R_BT_470_6_SYSTEM_M;
    QTest::newRow("g2.4") << 2.4 << true << TRC_GAMMA_2_4;
    QTest::newRow("g2.8") << 2.8 << true << TRC_ITU_R_BT_470_6_SYSTEM_B_G;

    // we support only a predefined set of gamma exponents
    QTest::newRow("g1.6") << 1.6 << false << TRC_UNSPECIFIED;
}

void TestIccFromColorimetryConversion::testRequestConstructionCustomGamma()
{
    QFETCH(qreal, gamma);
    QFETCH(bool, isValid);
    QFETCH(TransferCharacteristics, expectedTransferFunction);

    using KisColorimetryUtils::Colorimetry;
    using KisSurfaceColorimetry::ColorSpace;
    using KisSurfaceColorimetry::NamedTransferFunction;
    using KisSurfaceColorimetry::NamedPrimaries;

    ColorSpace  colorSpace;
    colorSpace.primaries = NamedPrimaries::primaries_srgb;
    colorSpace.transferFunction = static_cast<uint32_t>(std::rint(gamma * 10000));

    auto request = KisSurfaceColorimetry::colorSpaceToRequest(colorSpace);

    QCOMPARE(request.isValid(), isValid);
    QCOMPARE(request.colorPrimariesType, TRC_ITU_R_BT_709_5);
    QCOMPARE(request.transferFunction, expectedTransferFunction);
    QVERIFY(request.colorants.isEmpty());

}

void TestIccFromColorimetryConversion::testProfileConstruction_data()
{
    testRequestConstruction_data();
}

void TestIccFromColorimetryConversion::testProfileConstruction()
{
    QFETCH(KisSurfaceColorimetry::ColorSpace, colorSpace);
    QFETCH(bool, isValid);
    QFETCH(ColorPrimaries, expectedPrimaries);
    QFETCH(TransferCharacteristics, expectedTransferFunction);

    // skip inherited invalid requests
    if (!isValid) return;

    auto request = KisSurfaceColorimetry::colorSpaceToRequest(colorSpace);
    QVERIFY(request.isValid());

    const KoColorProfile *profile = KoColorSpaceRegistry::instance()->profileFor(request.colorants,
                                                                                 request.colorPrimariesType,
                                                                                 request.transferFunction);

    QVERIFY(profile);

    if (QLatin1String(QTest::currentDataTag()) == "bt2020-pq") {
        // we use a fake profile with a special color space for bt2020pq space, so
        // just verify that the profile name is consistent
        QCOMPARE(profile->name(), "High Dynamic Range UHDTV Wide Color Gamut Display (Rec. 2020) - SMPTE ST 2084 PQ EOTF");
    } else {
        QCOMPARE(profile->getColorPrimaries(), expectedPrimaries);
        QCOMPARE(profile->getTransferCharacteristics(), expectedTransferFunction);
    }

    qDebug() << ppVar(profile->name()) << ppVar(profile->fileName());
}

void TestIccFromColorimetryConversion::testProfileConstructionCustomPrimaries()
{
    using KisColorimetryUtils::Colorimetry;
    using KisSurfaceColorimetry::ColorSpace;
    using KisSurfaceColorimetry::NamedTransferFunction;

    ColorSpace  colorSpace;
    colorSpace.primaries = Colorimetry::BT709;
    colorSpace.transferFunction = NamedTransferFunction::transfer_function_srgb;

    auto request = KisSurfaceColorimetry::colorSpaceToRequest(colorSpace);

    QCOMPARE(request.isValid(), true);

    const KoColorProfile *profile = KoColorSpaceRegistry::instance()->profileFor(request.colorants,
                                                                                 request.colorPrimariesType,
                                                                                 request.transferFunction);

    QVERIFY(profile);
    QCOMPARE(profile->getColorPrimaries(), PRIMARIES_ITU_R_BT_709_5);
    QCOMPARE(profile->getTransferCharacteristics(), TRC_IEC_61966_2_1);
}

KISTEST_MAIN(TestIccFromColorimetryConversion)
