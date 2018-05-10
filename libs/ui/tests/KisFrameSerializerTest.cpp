/*
 *  Copyright (c) 2018 Dmitry Kazakov <dimula73@gmail.com>
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

#include "KisFrameSerializerTest.h"

#include <KisFrameDataSerializer.h>
#include "opengl/kis_texture_tile_info_pool.h"

#include <testutil.h>

#include <QTest>

static const int maxTileSize = 256;

KisFrameDataSerializer::Frame generateTestFrame(int frameId, KisTextureTileInfoPoolSP pool)
{
    KisFrameDataSerializer::Frame frame;
    frame.frameId = frameId;
    frame.pixelSize = 4;

    for (int i = 0; i < qBound(1, frameId * 5, 100); i++) {
        KisFrameDataSerializer::FrameTile tile(pool);
        tile.col = i * 10;
        tile.row = i * 20;
        tile.rect = QRect(QPoint(i, 2 * i), QSize(qMin(i * 5, maxTileSize), qMin(i * 7, maxTileSize)));
        tile.data.allocate(frame.pixelSize);

        const int numPixels = tile.rect.width() * tile.rect.height();
        qint32 *dataPtr = reinterpret_cast<qint32*>(tile.data.data());

        for (int j = 0; j < numPixels; j++) {
            *dataPtr++ = frameId + j;
        }

        frame.frameTiles.push_back(std::move(tile));
    }

    return std::move(frame);
}

bool verifyTestFrame(int frameId, const KisFrameDataSerializer::Frame &frame)
{
    KIS_COMPARE_RF(frame.frameId, frameId);
    KIS_COMPARE_RF(frame.pixelSize, 4);
    KIS_COMPARE_RF(int(frame.frameTiles.size()), qBound(1, frameId * 5, 100));

    for (int i = 0; i < int(frame.frameTiles.size()); i++) {
        const KisFrameDataSerializer::FrameTile &tile = frame.frameTiles[i];

        KIS_COMPARE_RF(tile.col, i * 10);
        KIS_COMPARE_RF(tile.row, i * 20);
        KIS_COMPARE_RF(tile.rect.x(), i);
        KIS_COMPARE_RF(tile.rect.y(), 2 * i);
        KIS_COMPARE_RF(tile.rect.size(), QSize(qMin(i * 5, maxTileSize), qMin(i * 7, maxTileSize)));

        const int numPixels = tile.rect.width() * tile.rect.height();
        qint32 *dataPtr = reinterpret_cast<qint32*>(tile.data.data());

        for (int j = 0; j < numPixels; j++) {
            KIS_COMPARE_RF(*dataPtr++, frameId + j);
        }
    }

    return true;
}



void KisFrameSerializerTest::testFrameDataSerialization()
{
    KisTextureTileInfoPoolRegistry poolRegistry;
    KisTextureTileInfoPoolSP pool = poolRegistry.getPool(maxTileSize, maxTileSize);


    KisFrameDataSerializer serializer(pool);

    KisFrameDataSerializer::Frame testFrame1 = generateTestFrame(2, pool);
    KisFrameDataSerializer::Frame testFrame2 = generateTestFrame(3, pool);
    KisFrameDataSerializer::Frame testFrame3 = generateTestFrame(503, pool);


    serializer.saveFrame(testFrame1);
    QCOMPARE(serializer.hasFrame(2), true);
    QCOMPARE(serializer.hasFrame(3), false);
    QCOMPARE(serializer.hasFrame(503), false);

    serializer.saveFrame(testFrame2);
    QCOMPARE(serializer.hasFrame(2), true);
    QCOMPARE(serializer.hasFrame(3), true);
    QCOMPARE(serializer.hasFrame(503), false);

    serializer.saveFrame(testFrame3);
    QCOMPARE(serializer.hasFrame(2), true);
    QCOMPARE(serializer.hasFrame(3), true);
    QCOMPARE(serializer.hasFrame(503), true);

    QVERIFY(verifyTestFrame(2, serializer.loadFrame(2)));
    QVERIFY(verifyTestFrame(3, serializer.loadFrame(3)));
    QVERIFY(verifyTestFrame(503, serializer.loadFrame(503)));

    serializer.forgetFrame(3);
    QCOMPARE(serializer.hasFrame(2), true);
    QCOMPARE(serializer.hasFrame(3), false);
    QCOMPARE(serializer.hasFrame(503), true);

    serializer.forgetFrame(503);
    QCOMPARE(serializer.hasFrame(2), true);
    QCOMPARE(serializer.hasFrame(3), false);
    QCOMPARE(serializer.hasFrame(503), false);

    serializer.forgetFrame(2);
    QCOMPARE(serializer.hasFrame(2), false);
    QCOMPARE(serializer.hasFrame(3), false);
    QCOMPARE(serializer.hasFrame(503), false);
}

#include "kis_random_source.h"

void randomizeFrame(KisFrameDataSerializer::Frame &frame, qreal portion)
{
    // randomly reset 50% of the pixels
    KisRandomSource rnd(1);
    for (KisFrameDataSerializer::FrameTile &tile : frame.frameTiles) {
        const int numPixels = tile.rect.width() * tile.rect.height();
        qint32 *pixelPtr = reinterpret_cast<qint32*>(tile.data.data());

        for (int j = 0; j < numPixels; j++) {
            if (rnd.generateNormalized() < portion) {
                (*pixelPtr) = 0;
            }

            pixelPtr++;
        }
    }
}

void KisFrameSerializerTest::testFrameUniquenessEstimation()
{
    KisTextureTileInfoPoolRegistry poolRegistry;
    KisTextureTileInfoPoolSP pool = poolRegistry.getPool(maxTileSize, maxTileSize);

    KisFrameDataSerializer::Frame testFrame1 = generateTestFrame(2, pool);
    KisFrameDataSerializer::Frame testFrame2 = generateTestFrame(2, pool);

    boost::optional<qreal> result;

    result = KisFrameDataSerializer::estimateFrameUniqueness(testFrame1, testFrame2, 0.1);
    QVERIFY(result);
    QVERIFY(qFuzzyCompare(*result, 0.0));

    KisFrameDataSerializer::Frame testFrame3 = generateTestFrame(3, pool);

    result = KisFrameDataSerializer::estimateFrameUniqueness(testFrame1, testFrame3, 0.1);
    QVERIFY(!result);

    // randomly reset 50% of the pixels
    randomizeFrame(testFrame2, 0.5);

    result = KisFrameDataSerializer::estimateFrameUniqueness(testFrame1, testFrame2, 0.01);
    QVERIFY(result);
    QVERIFY(*result >= 0.45);
    QVERIFY(*result <= 0.55);
}

void KisFrameSerializerTest::testFrameArithmetics()
{
    KisTextureTileInfoPoolRegistry poolRegistry;
    KisTextureTileInfoPoolSP pool = poolRegistry.getPool(maxTileSize, maxTileSize);

    KisFrameDataSerializer::Frame testFrame1 = generateTestFrame(2, pool);
    KisFrameDataSerializer::Frame testFrame2 = generateTestFrame(2, pool);
    randomizeFrame(testFrame2, 0.2);

    boost::optional<qreal> result =
        KisFrameDataSerializer::estimateFrameUniqueness(testFrame1, testFrame2, 0.01);

    QVERIFY(result);
    QVERIFY(*result >= 0.15);
    QVERIFY(*result <= 0.25);


    {
        KisFrameDataSerializer::Frame testFrame3 = generateTestFrame(2, pool);
        randomizeFrame(testFrame3, 0.2);

        const bool framesAreSame = KisFrameDataSerializer::subtractFrames(testFrame3, testFrame2);
        QVERIFY(framesAreSame);
    }

    {
        KisFrameDataSerializer::Frame testFrame3 = generateTestFrame(2, pool);
        randomizeFrame(testFrame3, 0.2);

        const bool framesAreSame = KisFrameDataSerializer::subtractFrames(testFrame3, testFrame1);
        QVERIFY(!framesAreSame);

        KisFrameDataSerializer::addFrames(testFrame3, testFrame1);

        result = KisFrameDataSerializer::estimateFrameUniqueness(testFrame3, testFrame2, 1.0);
        QVERIFY(result);
        QVERIFY(*result == 0.0);
    }
}

QTEST_MAIN(KisFrameSerializerTest)
