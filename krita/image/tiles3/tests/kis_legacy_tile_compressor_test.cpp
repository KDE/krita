/*
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_legacy_tile_compressor_test.h"
#include <qtest_kde.h>

#include "tiles3/kis_tiled_data_manager.h"
#include "tiles3/swap/kis_legacy_tile_compressor.h"

#include <KoStore_p.h>

class KoStoreFake : public KoStore
{
public:
    KoStoreFake() {
        d_ptr->stream = &m_buffer;
        d_ptr->isOpen = true;
        d_ptr->mode = KoStore::Write;
        m_buffer.open(QIODevice::ReadWrite);
    }
    ~KoStoreFake() {
        // Oh, no, please do not clean anything! :)
        d_ptr->stream = 0;
        d_ptr->isOpen = false;
    }

    void startReading() {
        m_buffer.seek(0);
        d_ptr->mode = KoStore::Read;
    }

    bool openWrite(const QString&) { return true; }
    bool openRead(const QString&) { return true; }
    bool closeRead() { return true; }
    bool closeWrite() { return true; }
    bool enterRelativeDirectory(const QString&) { return true; }
    bool enterAbsoluteDirectory(const QString&) { return true; }
    bool fileExists(const QString&) const { return true; }
private:
    QBuffer m_buffer;
};

bool KisLegacyTileCompressorTest::memoryIsFilled(quint8 c, quint8 *mem, qint32 size)
{
    for(; size > 0; size--)
        if(*(mem++) != c) {
            qDebug() << "Expected" << c << "but found" << *(mem-1);
            return false;
        }

    return true;
}

#define TILESIZE 64*64

void KisLegacyTileCompressorTest::testRoundTrip()
{
    quint8 defaultPixel = 0;
    KisTiledDataManager dm(1, &defaultPixel);

    quint8 oddPixel1 = 128;
    KisTileSP tile00;

    dm.clear(0, 0, 64, 64, &oddPixel1);

    tile00 = dm.getTile(0, 0, false);
    QVERIFY(memoryIsFilled(oddPixel1, tile00->data(), TILESIZE));

    KoStoreFake fakeStore;

    KisLegacyTileCompressor compressor;
    compressor.writeTile(tile00, &fakeStore);
    tile00 = 0;

    fakeStore.startReading();

    dm.clear();

    tile00 = dm.getTile(0, 0, false);
    QVERIFY(memoryIsFilled(defaultPixel, tile00->data(), TILESIZE));
    tile00 = 0;

    compressor.readTile(&fakeStore, &dm);

    tile00 = dm.getTile(0, 0, false);
    QVERIFY(memoryIsFilled(oddPixel1, tile00->data(), TILESIZE));
    tile00 = 0;
}

void KisLegacyTileCompressorTest::testLowLevelRoundTrip()
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

    KisLegacyTileCompressor compressor;

    qint32 bufferSize = compressor.tileDataBufferSize(td);
    quint8 *buffer = new quint8[bufferSize];
    qint32 bytesUsed;
    compressor.compressTileData(td, buffer, bufferSize, bytesUsed);
    QCOMPARE(bytesUsed, TILESIZE);

    memset(td->data(), oddPixel2, TILESIZE);
    QVERIFY(memoryIsFilled(oddPixel2, td->data(), TILESIZE));

    compressor.decompressTileData(buffer, bufferSize, td, bytesUsed);
    QCOMPARE(bytesUsed, TILESIZE);

    QVERIFY(memoryIsFilled(oddPixel1, td->data(), TILESIZE));

    delete[] buffer;
    tile->unlock();
}


QTEST_KDEMAIN(KisLegacyTileCompressorTest, NoGUI)
#include "kis_legacy_tile_compressor_test.moc"

