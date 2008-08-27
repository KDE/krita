/*
 *  Copyright (c) 2008 Bart Coppens <kde@bartcoppens.be>
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

#include "kis_tiles_from_file_tester.h"
#include <qtest_kde.h>

#include "kis_global.h"

#include <KoColorSpaceRegistry.h>

// ### HACK so that we can easily test the internals
#define private public
#define protected public

#include "tiles_new/kis_tilestorememory.h"
#include "tiles_new/kis_tilesourcefile.h"
#include "tiles_new/kis_sharedtiledata.h"

#include "kis_datamanager.h" // ### Check that, indeed, this is a TILED dm?
#include "kis_iterator.h"
#include "kis_tilediterator.h"

#include "kis_tile.h"

#undef private
#undef protected

#include "kis_paint_device.h"
#include "kis_iterators_pixel.h"
#include "kis_transaction.h"

static quint8 defPixel[2] = { 145, 42 };

namespace
{
/// Somewhat realistic file loader, fakes loading a file with 2 U8 channels (like GRAYA(8)), with full and half-full tiles
class FakeTileSourceTest : public KisTileSourceFile
{
    static const qint32 w = /*KisTile::WIDTH*/  64* 2 + 20;
    static const qint32 h = /*KisTile::HEIGHT*/ 64 * 2 + 20;
public: // broadens privileges from protected:
    FakeTileSourceTest() : columnsGotten(0) {}

    const quint8* getColumnData(qint32 col) {
        Q_ASSERT(col < h);
        quint8* d = new quint8[2*w];
        for (int i = 0; i < w; i++) {
            d[2*i] = (quint8)(i + col); // Channel 1
            d[2*i+1] = (quint8)(i * col); // Channel 2
        }

        columnsGotten++;

        return d;
    }

    qint32 width() {
        return w;
    }
    qint32 height() {
        return h;
    }
    qint32 pixelSize() const {
        return 2;
    }

    int columnsGotten;
};
}

void KisTilesFromFileTester::loadFakeTilesTest()
{
    FakeTileSourceTest tileSource;
    KisTiledDataManager dm(tileSource.pixelSize(), defPixel);

    dm.setProxy(&tileSource);

    // Tests wholly filled tiles, partially filled tiles and unfilled tiles for their content
    for (int y = 0; y < tileSource.height() + 64; y++) {
        for (int x = 0; x < tileSource.width() + 64; x++) {
            KisTileDataWrapperSP data = dm.pixelPtrSafe(x, y, false /*writable*/);
            if (x < tileSource.width() && y < tileSource.height()) {
                QVERIFY(data->data()[0] == (quint8)(x + y)); // Channel 1
                QVERIFY(data->data()[1] == (quint8)(x*y)); // Channel 2
            } else {
                QVERIFY(data->data()[0] == defPixel[0]); // Channel 1
                QVERIFY(data->data()[1] == defPixel[1]); // Channel 2
            }
        }
    }

    // Since we looped over the data column-wise, each column should have been visited only once, despite multiple tiles
    // needing information from them:
    QVERIFY(tileSource.columnsGotten == tileSource.height());
}

void KisTilesFromFileTester::paintDeviceTest()
{
    const KoColorSpace* colorSpace = KoColorSpaceRegistry::instance()->colorSpace("GRAYA", 0);
    KisPaintDevice dev(colorSpace, "test");

    FakeTileSourceTest tileSource;
    KisDataManagerSP dm = new KisDataManager(tileSource.pixelSize(), defPixel);

    dm->setProxy(&tileSource);

    dev.setDataManager(dm, dev.colorSpace()); // Clones the colorspace, perhaps that's overkill?

    // Iterate over the tiles a first time:
    KisHLineConstIteratorPixel srcIt = dev.createHLineConstIterator(0, 0, tileSource.width() + 64);

    for (int y = 0; y < tileSource.height() + 64; y++) {
        while (!srcIt.isDone()) {
            const quint8* data = srcIt.rawData();
            if (srcIt.x() < tileSource.width() && y < tileSource.height()) {
                QVERIFY(data[0] == (quint8)(srcIt.x() + y)); // Channel 1
                QVERIFY(data[1] == (quint8)(srcIt.x()*y)); // Channel 2
            } else {
                QVERIFY(data[0] == defPixel[0]); // Channel 1
                QVERIFY(data[1] == defPixel[1]); // Channel 2
            }
            ++srcIt;
        }
        srcIt.nextRow();
    }

    QVERIFY(tileSource.columnsGotten == tileSource.height());

    // Iterate a second time:
    srcIt = dev.createHLineConstIterator(0, 0, tileSource.width() + 64);

    for (int y = 0; y < tileSource.height() + 64; y++) {
        while (!srcIt.isDone()) {
            const quint8* data = srcIt.rawData();
            if (srcIt.x() < tileSource.width() && y < tileSource.height()) {
                QVERIFY(data[0] == (quint8)(srcIt.x() + y)); // Channel 1
                QVERIFY(data[1] == (quint8)(srcIt.x()*y)); // Channel 2
            } else {
                QVERIFY(data[0] == defPixel[0]); // Channel 1
                QVERIFY(data[1] == defPixel[1]); // Channel 2
            }
            ++srcIt;
        }
        srcIt.nextRow();
    }

    // The image height is greater than the current cache size (2*64 lines)
    QVERIFY(tileSource.columnsGotten == 2 * tileSource.height());
}

void KisTilesFromFileTester::writeTest()
{
    const KoColorSpace* colorSpace = KoColorSpaceRegistry::instance()->colorSpace("GRAYA", 0);
    KisPaintDeviceSP dev = new KisPaintDevice(colorSpace, "test");

    KisSharedPtr<FakeTileSourceTest> tileSource = new FakeTileSourceTest;
    KisDataManagerSP dm = new KisDataManager(tileSource->pixelSize(), defPixel);

    dm->setProxy(tileSource);

    dev->setDataManager(dm, dev->colorSpace()); // Clones the colorspace, perhaps that's overkill?

    // Transaction, so that we ensure a Memento, and we get oldData
    KisTransaction trans("test", dev, 0);

    // Iterate over the tiles a first time:
    KisHLineIteratorPixel dstIt = dev->createHLineIterator(0, 0, tileSource->width() + 64);

    // Manually fill the pixels to 23,32:
    for (int y = 0; y < tileSource->height() + 64; y++) {
        while (!dstIt.isDone()) {
            quint8* data = dstIt.rawData();
            if (dstIt.x() < tileSource->width() && y < tileSource->height()) {
                QVERIFY(data[0] == (quint8)(dstIt.x() + y)); // Channel 1
                QVERIFY(data[1] == (quint8)(dstIt.x()*y)); // Channel 2
            } else {
                QVERIFY(data[0] == defPixel[0]); // Channel 1
                QVERIFY(data[1] == defPixel[1]); // Channel 2
            }
            data[0] = 23;
            data[1] = 32;
            ++dstIt;
        }
        dstIt.nextRow();
    }

    QVERIFY(tileSource->columnsGotten == tileSource->height());

    // Loop over the tiles again, the data should now be overwritten, oldData should be intact, and number of columns kept the same
    KisHLineConstIteratorPixel srcIt = dev->createHLineConstIterator(0, 0, tileSource->width() + 64);

    for (int y = 0; y < tileSource->height() + 64; y++) {
        while (!srcIt.isDone()) {
            // Check oldData
            const quint8* data = srcIt.oldRawData();
            if (srcIt.x() < tileSource->width() && y < tileSource->height()) {
                QVERIFY(data[0] == (quint8)(srcIt.x() + y)); // Channel 1
                QVERIFY(data[1] == (quint8)(srcIt.x()*y)); // Channel 2
            } else {
                QVERIFY(data[0] == defPixel[0]); // Channel 1
                QVERIFY(data[1] == defPixel[1]); // Channel 2
            }

            // Check overwritten data:
            data = srcIt.rawData();
            QVERIFY(data[0] == 23); // Channel 1
            QVERIFY(data[1] == 32); // Channel 2

            ++srcIt;
        }
        srcIt.nextRow();
    }

    QVERIFY(tileSource->columnsGotten == tileSource->height());
}

QTEST_KDEMAIN(KisTilesFromFileTester, GUI) // GUI for the possible colorspaces

#include "kis_tiles_from_file_tester.moc"
