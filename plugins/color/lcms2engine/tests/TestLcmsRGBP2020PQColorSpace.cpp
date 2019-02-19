/*
 *  Copyright (c) 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "TestLcmsRGBP2020PQColorSpace.h"

#include <QTest>
#include "sdk/tests/kistest.h"

#include "kis_debug.h"

#include "KoColorProfile.h"
#include "KoColorSpaceRegistry.h"
#include "KoColor.h"
#include "KoColorModelStandardIds.h"

inline QString truncated(QString value) {
    value.truncate(24);
    return value;
}

enum SourceType {
    SDR,
    HDR,
    HDR_PQ
};

void testRoundTrip(const KoColorSpace *srcCS, const KoColorSpace *dstCS, SourceType sourceIsPQ)
{
    qDebug() << "Testing:" << srcCS->id() << truncated(srcCS->profile()->name())
             << "->"
             << dstCS->id() << truncated(dstCS->profile()->name());

    KoColor srcColor(srcCS);
    KoColor dstColor(dstCS);

    QVector<float> refChannels;

    if (sourceIsPQ == HDR) {
        refChannels << 2.8; // R
        refChannels << 1.8; // G
        refChannels << 0.8; // B
        refChannels << 0.9; // A
    } else if (sourceIsPQ == HDR_PQ) {
        refChannels << 0.9; // R (PQ)
        refChannels << 0.7; // G (PQ)
        refChannels << 0.1; // B (PQ)
        refChannels << 0.9; // A
    } else if (sourceIsPQ == SDR) {
        refChannels << 0.15; // R
        refChannels << 0.17; // G
        refChannels << 0.19; // B
        refChannels << 0.90; // A
    }

    srcCS->fromNormalisedChannelsValue(srcColor.data(), refChannels);

    srcCS->convertPixelsTo(srcColor.data(), dstColor.data(), dstCS, 1,
                           KoColorConversionTransformation::internalRenderingIntent(),
                           KoColorConversionTransformation::internalConversionFlags());

    dstCS->convertPixelsTo(dstColor.data(), srcColor.data(), srcCS, 1,
                           KoColorConversionTransformation::internalRenderingIntent(),
                           KoColorConversionTransformation::internalConversionFlags());

    QVector<float> result(4);
    srcCS->normalisedChannelsValue(srcColor.data(), result);

    QList<KoChannelInfo*> channels = srcCS->channels();

    // 5% tolerance for CMYK, 4% for 8-bit, and 1% for everything else
    const float tolerance =
        dstCS->colorModelId() == CMYKAColorModelID ? 0.05 :
        (dstCS->colorDepthId() == Integer8BitsColorDepthID ||
         srcCS->colorDepthId() == Integer8BitsColorDepthID) ? 0.04 :
        0.01;

    bool roundTripIsCorrect = true;
    for (int i = 0; i < 4; i++) {
        roundTripIsCorrect &= qAbs(refChannels[i] - result[i]) < tolerance;
    }

    if (!roundTripIsCorrect) {
        for (int i = 0; i < 4; i++) {
            qDebug() << channels[i]->name() << "ref" << refChannels[i] << "result" << result[i];
        }
    }

    QVERIFY(roundTripIsCorrect);
}

void testRoundTrip(const KoID &linearColorDepth, const KoID &pqColorDepth, SourceType sourceIsPQ)
{
    const KoColorProfile *p2020PQProfile = KoColorSpaceRegistry::instance()->p2020PQProfile();
    const KoColorProfile *p2020G10Profile = KoColorSpaceRegistry::instance()->p2020G10Profile();

    const KoColorSpace *srcCS = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(), linearColorDepth.id(), p2020G10Profile);
    const KoColorSpace *dstCS = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(), pqColorDepth.id(), p2020PQProfile);;

    if (sourceIsPQ == HDR_PQ) {
        std::swap(srcCS, dstCS);
    }

    testRoundTrip(srcCS, dstCS, sourceIsPQ);
}

void TestLcmsRGBP2020PQColorSpace::test()
{
    const KoColorProfile *p2020PQProfile = KoColorSpaceRegistry::instance()->p2020PQProfile();
    const KoColorProfile *p2020G10Profile = KoColorSpaceRegistry::instance()->p2020G10Profile();
    const KoColorProfile *p709G10Profile = KoColorSpaceRegistry::instance()->p709G10Profile();

    QVERIFY(p2020PQProfile);
    QVERIFY(p2020G10Profile);
    QVERIFY(p709G10Profile);

    QVector<KoID> linearModes;
    linearModes << Float16BitsColorDepthID;
    linearModes << Float32BitsColorDepthID;

    QVector<KoID> pqModes;
    pqModes << Integer8BitsColorDepthID;
    pqModes << Integer16BitsColorDepthID;
    pqModes << Float16BitsColorDepthID;
    pqModes << Float32BitsColorDepthID;

    Q_FOREACH(const KoID &src, linearModes) {
        Q_FOREACH(const KoID &dst, pqModes) {
            testRoundTrip(src, dst, HDR);
        }
    }

    Q_FOREACH(const KoID &src, linearModes) {
        Q_FOREACH(const KoID &dst, pqModes) {
            testRoundTrip(src, dst, HDR_PQ);
        }
    }
}

void TestLcmsRGBP2020PQColorSpace::testInternalConversions()
{
    const KoColorProfile *p2020PQProfile = KoColorSpaceRegistry::instance()->p2020PQProfile();

    QVector<KoID> pqModes;
    pqModes << Integer16BitsColorDepthID;
    pqModes << Float16BitsColorDepthID;
    pqModes << Float32BitsColorDepthID;

    Q_FOREACH(const KoID &src, pqModes) {
        Q_FOREACH(const KoID &dst, pqModes) {
            if (src == dst) continue;

            const KoColorSpace *srcCS = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(), src.id(), p2020PQProfile);
            const KoColorSpace *dstCS = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(), dst.id(), p2020PQProfile);

            testRoundTrip(srcCS, dstCS, HDR_PQ);
        }
    }
}

void TestLcmsRGBP2020PQColorSpace::testConvertToCmyk()
{
    const KoColorProfile *p2020PQProfile = KoColorSpaceRegistry::instance()->p2020PQProfile();

    const KoColorSpace *srcCS = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(), Integer16BitsColorDepthID.id(), p2020PQProfile);
    const KoColorSpace *dstCS = KoColorSpaceRegistry::instance()->colorSpace(CMYKAColorModelID.id(), Integer8BitsColorDepthID.id(), 0);

    testRoundTrip(srcCS, dstCS, SDR);
}

KISTEST_MAIN(TestLcmsRGBP2020PQColorSpace)
