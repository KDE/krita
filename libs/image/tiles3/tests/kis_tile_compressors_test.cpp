/*
 *  SPDX-FileCopyrightText: 2010 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_tile_compressors_test.h"
#include <QTest>

#include "tiles3/kis_tiled_data_manager.h"
#include "tiles3/swap/kis_legacy_tile_compressor.h"
#include "tiles3/swap/kis_tile_compressor_2.h"

#include "tiles_test_utils.h"

void KisTileCompressorsTest::doRoundTrip(KisAbstractTileCompressor *compressor)
{
    quint8 defaultPixel = 0;
    KisTiledDataManager dm(1, &defaultPixel);

    quint8 oddPixel1 = 128;
    KisTileSP tile11;

    dm.clear(64, 64, 64, 64, &oddPixel1);

    tile11 = dm.getTile(1, 1, false);
    QVERIFY(memoryIsFilled(oddPixel1, tile11->data(), TILESIZE));

    KoStoreFake fakeStore;
    KisFakePaintDeviceWriter writer(&fakeStore);
    
    bool retval = compressor->writeTile(tile11, writer);
    Q_ASSERT(retval);
    tile11 = 0;

    fakeStore.startReading();

    dm.clear();

    tile11 = dm.getTile(1, 1, false);
    QVERIFY(memoryIsFilled(defaultPixel, tile11->data(), TILESIZE));
    tile11 = 0;

    bool res = compressor->readTile(fakeStore.device(), &dm);
    Q_ASSERT(res);
    Q_UNUSED(res);
    tile11 = dm.getTile(1, 1, false);
    QVERIFY(memoryIsFilled(oddPixel1, tile11->data(), TILESIZE));
    tile11 = 0;
}

void KisTileCompressorsTest::doLowLevelRoundTrip(KisAbstractTileCompressor *compressor)
{
    const qint32 pixelSize = 1;
    quint8 oddPixel1 = 128;
    quint8 oddPixel2 = 129;

    /**
     * A small hack to acquire a standalone tile data.
     * globalTileDataStore is not exported out of kritaimage.so,
     * so we get it from the data manager
     */
    KisTiledDataManager dm(pixelSize, &oddPixel1);
    KisTileSP tile = dm.getTile(0, 0, true);
    tile->lockForWrite();


    KisTileData *td = tile->tileData();
    QVERIFY(memoryIsFilled(oddPixel1, td->data(), TILESIZE));

    qint32 bufferSize = compressor->tileDataBufferSize(td);
    quint8 *buffer = new quint8[bufferSize];
    qint32 bytesWritten;
    compressor->compressTileData(td, buffer, bufferSize, bytesWritten);
    dbgKrita << ppVar(bytesWritten);


    memset(td->data(), oddPixel2, TILESIZE);
    QVERIFY(memoryIsFilled(oddPixel2, td->data(), TILESIZE));

    compressor->decompressTileData(buffer, bytesWritten, td);

    QVERIFY(memoryIsFilled(oddPixel1, td->data(), TILESIZE));

    delete[] buffer;
    tile->unlock();
}

void KisTileCompressorsTest::doLowLevelRoundTripIncompressible(KisAbstractTileCompressor *compressor)
{
    const qint32 pixelSize = 1;
    quint8 oddPixel1 = 128;
    quint8 oddPixel2 = 129;

    QFile file(QString(FILES_DATA_DIR) + QDir::separator() + "tile.png");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QByteArray incompressibleArray = file.readAll();

    /**
     * A small hack to acquire a standalone tile data.
     * globalTileDataStore is not exported out of kritaimage.so,
     * so we get it from the data manager
     */
    KisTiledDataManager dm(pixelSize, &oddPixel1);
    KisTileSP tile = dm.getTile(0, 0, true);
    tile->lockForWrite();


    KisTileData *td = tile->tileData();
    QVERIFY(memoryIsFilled(oddPixel1, td->data(), TILESIZE));

    memcpy(td->data(), incompressibleArray.data(), TILESIZE);
    QVERIFY(!memcmp(td->data(), incompressibleArray.data(), TILESIZE));

    qint32 bufferSize = compressor->tileDataBufferSize(td);
    quint8 *buffer = new quint8[bufferSize];
    qint32 bytesWritten;
    compressor->compressTileData(td, buffer, bufferSize, bytesWritten);
    dbgKrita << ppVar(bytesWritten);


    memset(td->data(), oddPixel2, TILESIZE);
    QVERIFY(memoryIsFilled(oddPixel2, td->data(), TILESIZE));

    compressor->decompressTileData(buffer, bytesWritten, td);

    QVERIFY(!memcmp(td->data(), incompressibleArray.data(), TILESIZE));

    delete[] buffer;
    tile->unlock();
}

void KisTileCompressorsTest::testRoundTripLegacy()
{
    KisAbstractTileCompressor *compressor = new KisLegacyTileCompressor();
    doRoundTrip(compressor);
    delete compressor;
}

void KisTileCompressorsTest::testLowLevelRoundTripLegacy()
{
    KisAbstractTileCompressor *compressor = new KisLegacyTileCompressor();
    doLowLevelRoundTrip(compressor);
    delete compressor;
}

void KisTileCompressorsTest::testRoundTrip2()
{
    KisAbstractTileCompressor *compressor = new KisTileCompressor2();
    doRoundTrip(compressor);
    delete compressor;
}

void KisTileCompressorsTest::testLowLevelRoundTrip2()
{
    KisAbstractTileCompressor *compressor = new KisTileCompressor2();
    doLowLevelRoundTrip(compressor);
    delete compressor;
}

void KisTileCompressorsTest::testLowLevelRoundTripIncompressible2()
{
    KisAbstractTileCompressor *compressor = new KisTileCompressor2();
    doLowLevelRoundTripIncompressible(compressor);
    delete compressor;
}


QTEST_MAIN(KisTileCompressorsTest)

