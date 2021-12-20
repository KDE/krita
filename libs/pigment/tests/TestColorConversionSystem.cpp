/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "TestColorConversionSystem.h"

#include <simpletest.h>

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

#include <KoColorConversionSystem_p.h>
#include <kis_debug.h>

namespace QTest {
inline bool qCompare(const std::vector<KoColorConversionSystem::NodeKey> &t1,
                     const std::vector<KoColorConversionSystem::NodeKey> &t2,
                     const char *actual, const char *expected,
                     const char *file, int line) {

    bool result = t1 == t2;

    if (!result) {
        QString actualStr;
        QDebug act(&actualStr);
        act.nospace() << actual << ": " << t1;

        QString expectedStr;
        QDebug exp(&expectedStr);
        exp.nospace() << expected << ": " << t2;

        QString message = QString("Compared paths are not the same:\n Expected: %1\n Actual: %2").arg(expectedStr).arg(actualStr);
        QTest::qFail(message.toLocal8Bit(), file, line);
    }

    return t1 == t2;
}
}

void TestColorConversionSystem::testAlphaConnectionPaths()
{
    const KoColorSpace *alpha8 = KoColorSpaceRegistry::instance()->alpha8();

    using Path = KoColorConversionSystem::Path;
    using Vertex = KoColorConversionSystem::Vertex;
    using Node = KoColorConversionSystem::Node;
    using NodeKey = KoColorConversionSystem::NodeKey;

    std::vector<NodeKey> expectedPath;

    auto calcPath = [] (const std::vector<NodeKey> &expectedPath) {

        const KoColorConversionSystem *system = KoColorSpaceRegistry::instance()->colorConversionSystem();

        Path path =
            system->findBestPath(expectedPath.front(), expectedPath.back());

        std::vector<NodeKey> realPath;

        Q_FOREACH (const Vertex *vertex, path.vertexes) {
            if (!vertex->srcNode->isEngine) {
                realPath.push_back(vertex->srcNode->key());
            }
        }
        realPath.push_back(path.vertexes.last()->dstNode->key());

        return realPath;
    };

    // to Alpha8 conversions. Everything should go via GrayA color space,
    // we expect alpha colorspace be just a flattened of graya color space
    // with srgb tone curve.

    expectedPath =
        {{GrayAColorModelID.id(), Integer8BitsColorDepthID.id(), "Gray-D50-elle-V2-srgbtrc.icc"},
         {alpha8->colorModelId().id(), alpha8->colorDepthId().id(), alpha8->profile()->name()}};
    QCOMPARE(calcPath(expectedPath), expectedPath);

    expectedPath =
        {{GrayAColorModelID.id(), Integer16BitsColorDepthID.id(), "Gray-D50-elle-V2-srgbtrc.icc"},
         {alpha8->colorModelId().id(), alpha8->colorDepthId().id(), alpha8->profile()->name()}};
    QCOMPARE(calcPath(expectedPath), expectedPath);

#ifdef HAVE_OPENEXR
    expectedPath =
        {{GrayAColorModelID.id(), Float16BitsColorDepthID.id(), "Gray-D50-elle-V2-srgbtrc.icc"},
         {alpha8->colorModelId().id(), alpha8->colorDepthId().id(), alpha8->profile()->name()}};
    QCOMPARE(calcPath(expectedPath), expectedPath);
#endif

    expectedPath =
        {{GrayAColorModelID.id(), Float32BitsColorDepthID.id(), "Gray-D50-elle-V2-srgbtrc.icc"},
         {alpha8->colorModelId().id(), alpha8->colorDepthId().id(), alpha8->profile()->name()}};
    QCOMPARE(calcPath(expectedPath), expectedPath);

    expectedPath =
        {{RGBAColorModelID.id(), Integer8BitsColorDepthID.id(), KoColorSpaceRegistry::instance()->p709SRGBProfile()->name()},
         {GrayAColorModelID.id(), Integer8BitsColorDepthID.id(), "Gray-D50-elle-V2-srgbtrc.icc"},
         {alpha8->colorModelId().id(), alpha8->colorDepthId().id(), alpha8->profile()->name()}};
    QCOMPARE(calcPath(expectedPath), expectedPath);

    expectedPath =
        {{RGBAColorModelID.id(), Integer16BitsColorDepthID.id(), KoColorSpaceRegistry::instance()->p709SRGBProfile()->name()},
         {GrayAColorModelID.id(), Integer8BitsColorDepthID.id(), "Gray-D50-elle-V2-srgbtrc.icc"},
         {alpha8->colorModelId().id(), alpha8->colorDepthId().id(), alpha8->profile()->name()}};
    QCOMPARE(calcPath(expectedPath), expectedPath);

    expectedPath =
        {{RGBAColorModelID.id(), Integer8BitsColorDepthID.id(), KoColorSpaceRegistry::instance()->p709SRGBProfile()->name()},
         {GrayAColorModelID.id(), Integer8BitsColorDepthID.id(), "Gray-D50-elle-V2-srgbtrc.icc"},
         {AlphaColorModelID.id(), Integer16BitsColorDepthID.id(), alpha8->profile()->name()}};
    QCOMPARE(calcPath(expectedPath), expectedPath);

    expectedPath =
        {{RGBAColorModelID.id(), Integer16BitsColorDepthID.id(), KoColorSpaceRegistry::instance()->p709SRGBProfile()->name()},
         {GrayAColorModelID.id(), Integer16BitsColorDepthID.id(), "Gray-D50-elle-V2-srgbtrc.icc"},
         {AlphaColorModelID.id(), Integer16BitsColorDepthID.id(), alpha8->profile()->name()}};
    QCOMPARE(calcPath(expectedPath), expectedPath);

    // from Alpha8 conversions. Everything should go via GrayA color space

    expectedPath =
        {{alpha8->colorModelId().id(), alpha8->colorDepthId().id(), alpha8->profile()->name()},
         {GrayAColorModelID.id(), Integer8BitsColorDepthID.id(), "Gray-D50-elle-V2-srgbtrc.icc"}};
    QCOMPARE(calcPath(expectedPath), expectedPath);

    expectedPath =
        {{alpha8->colorModelId().id(), alpha8->colorDepthId().id(), alpha8->profile()->name()},
         {GrayAColorModelID.id(), Integer16BitsColorDepthID.id(), "Gray-D50-elle-V2-srgbtrc.icc"}};
    QCOMPARE(calcPath(expectedPath), expectedPath);

#ifdef HAVE_OPENEXR
    expectedPath =
        {{alpha8->colorModelId().id(), alpha8->colorDepthId().id(), alpha8->profile()->name()},
         {GrayAColorModelID.id(), Float16BitsColorDepthID.id(), "Gray-D50-elle-V2-srgbtrc.icc"}};
    QCOMPARE(calcPath(expectedPath), expectedPath);
#endif

    expectedPath =
        {{alpha8->colorModelId().id(), alpha8->colorDepthId().id(), alpha8->profile()->name()},
         {GrayAColorModelID.id(), Float32BitsColorDepthID.id(), "Gray-D50-elle-V2-srgbtrc.icc"}};
    QCOMPARE(calcPath(expectedPath), expectedPath);

    expectedPath =
        {{alpha8->colorModelId().id(), alpha8->colorDepthId().id(), alpha8->profile()->name()},
         {GrayAColorModelID.id(), Integer8BitsColorDepthID.id(), "Gray-D50-elle-V2-srgbtrc.icc"},
         {RGBAColorModelID.id(), Integer8BitsColorDepthID.id(), KoColorSpaceRegistry::instance()->p709SRGBProfile()->name()}};
    QCOMPARE(calcPath(expectedPath), expectedPath);


    expectedPath =
        {{alpha8->colorModelId().id(), alpha8->colorDepthId().id(), alpha8->profile()->name()},
         {GrayAColorModelID.id(), Integer8BitsColorDepthID.id(), "Gray-D50-elle-V2-srgbtrc.icc"},
         {RGBAColorModelID.id(), Integer16BitsColorDepthID.id(), KoColorSpaceRegistry::instance()->p709SRGBProfile()->name()}};
    QCOMPARE(calcPath(expectedPath), expectedPath);

    expectedPath =
        {{AlphaColorModelID.id(), Integer16BitsColorDepthID.id(), alpha8->profile()->name()},
         {GrayAColorModelID.id(), Integer8BitsColorDepthID.id(), "Gray-D50-elle-V2-srgbtrc.icc"},
         {RGBAColorModelID.id(), Integer8BitsColorDepthID.id(), KoColorSpaceRegistry::instance()->p709SRGBProfile()->name()}};
    QCOMPARE(calcPath(expectedPath), expectedPath);

    expectedPath =
        {{AlphaColorModelID.id(), Integer16BitsColorDepthID.id(), alpha8->profile()->name()},
         {GrayAColorModelID.id(), Integer16BitsColorDepthID.id(), "Gray-D50-elle-V2-srgbtrc.icc"},
         {RGBAColorModelID.id(), Integer16BitsColorDepthID.id(), KoColorSpaceRegistry::instance()->p709SRGBProfile()->name()}};
    QCOMPARE(calcPath(expectedPath), expectedPath);
}

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
        QCOMPARE(c.opacityU8(), quint8(128));
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
        QCOMPARE(c.opacityU8(), quint8(128));
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
        QCOMPARE(c.opacityU8(), quint8(128));
    }

    {
        KoColor c(QColor(255,255,255,255), alpha16);
        QCOMPARE(c.opacityU8(), quint8(255));
        c.convertTo(rgb16);
        QCOMPARE(c.toQColor(), QColor(255,255,255));
        c.convertTo(alpha16);
        QCOMPARE(c.opacityU8(), quint8(255));
    }

    {
        KoColor c(QColor(255,255,255,0), alpha16);
        c.convertTo(rgb16);
        QCOMPARE(c.toQColor(), QColor(0,0,0,255));
        c.convertTo(alpha16);
        QCOMPARE(c.opacityU8(), quint8(0));
    }

    {
        KoColor c(QColor(255,255,255,128), alpha16);
        c.convertTo(rgb16);
        QCOMPARE(c.toQColor(), QColor(128,128,128,255));
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

void TestColorConversionSystem::testCmykBitnessConversion()
{
    const KoColorSpace *cmyk8 =
        KoColorSpaceRegistry::instance()->colorSpace(CMYKAColorModelID.id(),
                                                     Integer8BitsColorDepthID.id(),
                                                     "Chemical proof");

    const KoColorSpace *cmyk16 =
        KoColorSpaceRegistry::instance()->colorSpace(CMYKAColorModelID.id(),
                                                     Integer16BitsColorDepthID.id(),
                                                     "Chemical proof");

//    ENTER_FUNCTION() << ppVar(cmyk8);
//    ENTER_FUNCTION() << ppVar(cmyk8->profile()->name());
//    ENTER_FUNCTION() << ppVar(cmyk8->profile()->fileName());

//    ENTER_FUNCTION() << ppVar(cmyk16);
//    ENTER_FUNCTION() << ppVar(cmyk16->profile()->name());
//    ENTER_FUNCTION() << ppVar(cmyk16->profile()->fileName());


    KoColor color(QColor(177, 180, 42, 255), cmyk8);
//    qDebug() << ppVar(color);
    color.convertTo(cmyk16);
//    qDebug() << ppVar(color);
    KoColor color2 = color.convertedTo(cmyk8);
//    qDebug() << ppVar(color2);

    /**
     * For some reason out CMYK color spaces don't support rount-tripping
     * to-from 16-bit representation. So the code that relies on that should
     * use KoOptimizedCmykPixelDataScalerU8ToU16Factory::create().
     */
    QVERIFY(color != color2);

}


KISTEST_MAIN(TestColorConversionSystem)
