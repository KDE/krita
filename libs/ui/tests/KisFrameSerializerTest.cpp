/*
 *  SPDX-FileCopyrightText: 2018 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisFrameSerializerTest.h"

#include <KisFrameDataSerializer.h>
#include "opengl/kis_texture_tile_info_pool.h"

#include <testutil.h>

#include <simpletest.h>

static const int maxTileSize = 256;

KisFrameDataSerializer::Frame generateTestFrame(int frameSeed, KisTextureTileInfoPoolSP pool)
{
    KisFrameDataSerializer::Frame frame;
    frame.pixelSize = 4;

    for (int i = 0; i < qBound(1, frameSeed * 5, 100); i++) {
        KisFrameDataSerializer::FrameTile tile(pool);
        tile.col = i * 10;
        tile.row = i * 20;
        tile.rect = QRect(QPoint(i, 2 * i), QSize(qMin(i * 5, maxTileSize), qMin(i * 7, maxTileSize)));
        tile.data.allocate(frame.pixelSize);

        const int numPixels = tile.rect.width() * tile.rect.height();
        qint32 *dataPtr = reinterpret_cast<qint32*>(tile.data.data());

        for (int j = 0; j < numPixels; j++) {
            *dataPtr++ = frameSeed + j;
        }

        frame.frameTiles.push_back(std::move(tile));
    }

    return frame;
}

bool verifyTestFrame(int frameSeed, const KisFrameDataSerializer::Frame &frame)
{
    KIS_COMPARE_RF(frame.pixelSize, 4);
    KIS_COMPARE_RF(int(frame.frameTiles.size()), qBound(1, frameSeed * 5, 100));

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
            KIS_COMPARE_RF(*dataPtr++, frameSeed + j);
        }
    }

    return true;
}



void KisFrameSerializerTest::testFrameDataSerialization()
{
    KisTextureTileInfoPoolRegistry poolRegistry;
    KisTextureTileInfoPoolSP pool = poolRegistry.getPool(maxTileSize, maxTileSize);


    KisFrameDataSerializer serializer;

    KisFrameDataSerializer::Frame testFrame1 = generateTestFrame(2, pool);
    KisFrameDataSerializer::Frame testFrame2 = generateTestFrame(3, pool);
    KisFrameDataSerializer::Frame testFrame3 = generateTestFrame(503, pool);
    int testFrameId1 = -1;
    int testFrameId2 = -1;
    int testFrameId3 = -1;



    testFrameId1 = serializer.saveFrame(testFrame1);
    QCOMPARE(serializer.hasFrame(testFrameId1), true);
    QCOMPARE(serializer.hasFrame(testFrameId2), false);
    QCOMPARE(serializer.hasFrame(testFrameId3), false);

    testFrameId2 = serializer.saveFrame(testFrame2);
    QCOMPARE(serializer.hasFrame(testFrameId1), true);
    QCOMPARE(serializer.hasFrame(testFrameId2), true);
    QCOMPARE(serializer.hasFrame(testFrameId3), false);

    testFrameId3 = serializer.saveFrame(testFrame3);
    QCOMPARE(serializer.hasFrame(testFrameId1), true);
    QCOMPARE(serializer.hasFrame(testFrameId2), true);
    QCOMPARE(serializer.hasFrame(testFrameId3), true);

    QVERIFY(verifyTestFrame(2, serializer.loadFrame(testFrameId1, pool)));
    QVERIFY(verifyTestFrame(3, serializer.loadFrame(testFrameId2, pool)));
    QVERIFY(verifyTestFrame(503, serializer.loadFrame(testFrameId3, pool)));

    serializer.forgetFrame(testFrameId2);
    QCOMPARE(serializer.hasFrame(testFrameId1), true);
    QCOMPARE(serializer.hasFrame(testFrameId2), false);
    QCOMPARE(serializer.hasFrame(testFrameId3), true);

    serializer.forgetFrame(testFrameId3);
    QCOMPARE(serializer.hasFrame(testFrameId1), true);
    QCOMPARE(serializer.hasFrame(testFrameId2), false);
    QCOMPARE(serializer.hasFrame(testFrameId3), false);

    serializer.forgetFrame(testFrameId1);
    QCOMPARE(serializer.hasFrame(testFrameId1), false);
    QCOMPARE(serializer.hasFrame(testFrameId2), false);
    QCOMPARE(serializer.hasFrame(testFrameId3), false);
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
    QVERIFY(!!result);
    QVERIFY(qFuzzyCompare(*result, 0.0));

    KisFrameDataSerializer::Frame testFrame3 = generateTestFrame(3, pool);

    result = KisFrameDataSerializer::estimateFrameUniqueness(testFrame1, testFrame3, 0.1);
    QVERIFY(!result);

    // randomly reset 50% of the pixels
    randomizeFrame(testFrame2, 0.5);

    result = KisFrameDataSerializer::estimateFrameUniqueness(testFrame1, testFrame2, 0.01);
    QVERIFY(!!result);
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

    QVERIFY(!!result);
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
        QVERIFY(!!result);
        QVERIFY(*result == 0.0);
    }
}

SIMPLE_TEST_MAIN(KisFrameSerializerTest)
