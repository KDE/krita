/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "TestColorConversionSystem.h"

#include <QTest>

#include <DebugPigment.h>
#include <KoColorProfile.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorConversionSystem.h>
#include <KoColorModelStandardIds.h>
#include <sdk/tests/testpigment.h>

TestColorConversionSystem::TestColorConversionSystem()
{
    Q_FOREACH (const KoID& modelId, KoColorSpaceRegistry::instance()->colorModelsList(KoColorSpaceRegistry::AllColorSpaces)) {
        Q_FOREACH (const KoID& depthId, KoColorSpaceRegistry::instance()->colorDepthList(modelId, KoColorSpaceRegistry::AllColorSpaces)) {
            QList< const KoColorProfile * > profiles =
                KoColorSpaceRegistry::instance()->profilesFor(
                    KoColorSpaceRegistry::instance()->colorSpaceId(modelId, depthId));
            Q_FOREACH (const KoColorProfile * profile, profiles) {
                listModels.append(ModelDepthProfile(modelId.id(), depthId.id(), profile->name()));
            }
        }
    }
    //listModels.append(ModelDepthProfile(AlphaColorModelID.id(), Integer8BitsColorDepthID.id(), ""));
}

void TestColorConversionSystem::testConnections()
{
    Q_FOREACH (const ModelDepthProfile& srcCS, listModels) {
        Q_FOREACH (const ModelDepthProfile& dstCS, listModels) {
            QVERIFY2(KoColorSpaceRegistry::instance()->colorConversionSystem()->existsPath(srcCS.model, srcCS.depth, srcCS.profile, dstCS.model, dstCS.depth, dstCS.profile) , QString("No path between %1 / %2 and %3 / %4").arg(srcCS.model).arg(srcCS.depth).arg(dstCS.model).arg(dstCS.depth).toLatin1());
        }
    }
}

void TestColorConversionSystem::testGoodConnections()
{
    int countFail = 0;
    Q_FOREACH (const ModelDepthProfile& srcCS, listModels) {
        Q_FOREACH (const ModelDepthProfile& dstCS, listModels) {
            if (!KoColorSpaceRegistry::instance()->colorConversionSystem()->existsGoodPath(srcCS.model, srcCS.depth, srcCS.profile , dstCS.model, dstCS.depth, dstCS.profile)) {
                ++countFail;
                dbgPigment << "No good path between \"" << srcCS.model << " " << srcCS.depth << " " << srcCS.profile << "\" \"" << dstCS.model << " " << dstCS.depth << " " << dstCS.profile << "\"";
            }
        }
    }
    int failed = 0;
    if (!KoColorSpaceRegistry::instance()->colorSpace( RGBAColorModelID.id(), Float32BitsColorDepthID.id(), 0) && KoColorSpaceRegistry::instance()->colorSpace( "KS6", Float32BitsColorDepthID.id(), 0) ) {
        failed = 42;
    }
    QVERIFY2(countFail == failed, QString("%1 tests have fails (it should have been %2)").arg(countFail).arg(failed).toLatin1());
}

#include <KoColor.h>

void TestColorConversionSystem::testAlphaConversions()
{
    const KoColorSpace *alpha8 = KoColorSpaceRegistry::instance()->alpha8();
    const KoColorSpace *rgb8 = KoColorSpaceRegistry::instance()->rgb8();
    const KoColorSpace *rgb16 = KoColorSpaceRegistry::instance()->rgb16();

    {
        KoColor c(QColor(255,255,255,255), alpha8);
        QCOMPARE(c.opacityU8(), quint8(255));
        c.convertTo(rgb8);
        QCOMPARE(c.toQColor(), QColor(255,255,255));
        c.convertTo(alpha8);
        QCOMPARE(c.opacityU8(), quint8(255));
    }

    {
        KoColor c(QColor(255,255,255,0), alpha8);
        c.convertTo(rgb8);
        QCOMPARE(c.toQColor(), QColor(0,0,0,255));
        c.convertTo(alpha8);
        QCOMPARE(c.opacityU8(), quint8(0));
    }

    {
        KoColor c(QColor(255,255,255,128), alpha8);
        c.convertTo(rgb8);
        QCOMPARE(c.toQColor(), QColor(128,128,128,255));
        c.convertTo(alpha8);
        QCOMPARE(c.opacityU8(), quint8(137)); // alpha is linear, so the value increases
    }

    {
        KoColor c(QColor(255,255,255,255), alpha8);
        QCOMPARE(c.opacityU8(), quint8(255));
        c.convertTo(rgb16);
        QCOMPARE(c.toQColor(), QColor(255,255,255));
        c.convertTo(alpha8);
        QCOMPARE(c.opacityU8(), quint8(255));
    }

    {
        KoColor c(QColor(255,255,255,0), alpha8);
        c.convertTo(rgb16);
        QCOMPARE(c.toQColor(), QColor(0,0,0,255));
        c.convertTo(alpha8);
        QCOMPARE(c.opacityU8(), quint8(0));
    }

    {
        KoColor c(QColor(255,255,255,128), alpha8);
        c.convertTo(rgb16);
        QCOMPARE(c.toQColor(), QColor(128,128,128,255));
        c.convertTo(alpha8);
        QCOMPARE(c.opacityU8(), quint8(137));  // alpha is linear, so the value increases
    }
}

void TestColorConversionSystem::testAlphaU16Conversions()
{
    KoColorSpaceRegistry::instance();
    const KoColorSpace *alpha16 = KoColorSpaceRegistry::instance()->alpha16();
    const KoColorSpace *rgb8 = KoColorSpaceRegistry::instance()->rgb8();
    const KoColorSpace *rgb16 = KoColorSpaceRegistry::instance()->rgb16();

    {
        KoColor c(QColor(255,255,255,255), alpha16);
        QCOMPARE(c.opacityU8(), quint8(255));
        c.convertTo(rgb8);
        QCOMPARE(c.toQColor(), QColor(255,255,255));
        c.convertTo(alpha16);
        QCOMPARE(c.opacityU8(), quint8(255));
    }

    {
        KoColor c(QColor(255,255,255,0), alpha16);
        c.convertTo(rgb8);
        QCOMPARE(c.toQColor(), QColor(0,0,0,255));
        c.convertTo(alpha16);
        QCOMPARE(c.opacityU8(), quint8(0));
    }

    {
        KoColor c(QColor(255,255,255,128), alpha16);
        c.convertTo(rgb8);
        QCOMPARE(c.toQColor(), QColor(128,128,128,255));
        c.convertTo(alpha16);
        QCOMPARE(c.opacityU8(), quint8(137)); // alpha is linear, so the value increases
    }

    {
        KoColor c(QColor(255,255,255,255), alpha16);
        QCOMPARE(c.opacityU8(), quint8(255));
        c.convertTo(rgb16);
        QCOMPARE(c.toQColor(), QColor(254,255,255));
        c.convertTo(alpha16);
        QCOMPARE(c.opacityU8(), quint8(255));
    }

    {
        KoColor c(QColor(255,255,255,0), alpha16);
        c.convertTo(rgb16);
        QCOMPARE(c.toQColor(), QColor(0,0,1,255));
        c.convertTo(alpha16);
        QCOMPARE(c.opacityU8(), quint8(0));
    }

    {
        KoColor c(QColor(255,255,255,128), alpha16);
        c.convertTo(rgb16);
        QCOMPARE(c.toQColor(), QColor(118,120,120,255));
        c.convertTo(alpha16);
        QCOMPARE(c.opacityU8(), quint8(128));
    }
}

void TestColorConversionSystem::benchmarkAlphaToRgbConversion()
{
    const KoColorSpace *alpha8 = KoColorSpaceRegistry::instance()->alpha8();
    const KoColorSpace *rgb8 = KoColorSpaceRegistry::instance()->rgb8();

    const int numPixels = 1024 * 4096;
    QByteArray srcBuf(numPixels * alpha8->pixelSize(), '\0');
    QByteArray dstBuf(numPixels * rgb8->pixelSize(), '\0');

    qsrand(1);
    for (int i = 0; i < srcBuf.size(); i++) {
        srcBuf[i] = qrand() & 0xFF;
    }

    QBENCHMARK {
        alpha8->convertPixelsTo((quint8*)srcBuf.data(),
                                (quint8*)dstBuf.data(),
                                rgb8,
                                numPixels,
                                KoColorConversionTransformation::IntentPerceptual,
                                KoColorConversionTransformation::Empty);
    }
}

void TestColorConversionSystem::benchmarkRgbToAlphaConversion()
{
    const KoColorSpace *alpha8 = KoColorSpaceRegistry::instance()->alpha8();
    const KoColorSpace *rgb8 = KoColorSpaceRegistry::instance()->rgb8();

    const int numPixels = 1024 * 4096;
    QByteArray srcBuf(numPixels * rgb8->pixelSize(), '\0');
    QByteArray dstBuf(numPixels * alpha8->pixelSize(), '\0');

    qsrand(1);
    for (int i = 0; i < srcBuf.size(); i++) {
        srcBuf[i] = qrand() & 0xFF;
    }

    QBENCHMARK {
        rgb8->convertPixelsTo((quint8*)srcBuf.data(),
                              (quint8*)dstBuf.data(),
                              alpha8,
                              numPixels,
                              KoColorConversionTransformation::IntentPerceptual,
                              KoColorConversionTransformation::Empty);
    }
}


KISTEST_MAIN(TestColorConversionSystem)
