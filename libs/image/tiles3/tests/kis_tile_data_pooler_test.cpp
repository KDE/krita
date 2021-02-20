/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_tile_data_pooler_test.h"
#include <QTest>

#include "tiles3/kis_tiled_data_manager.h"

#include "tiles3/kis_tile_data_store.h"
#include "tiles3/kis_tile_data_store_iterators.h"

#include "tiles3/kis_tile_data_pooler.h"

#ifdef DEBUG_TILES
#define PRETTY_TILE(idx, td)                                    \
    qDebug << "tile" << i                                     \
           << "\tusers" << td->numUsers()                     \
           << "\tclones" << td->m_clonesStack.size()          \
           << "\tage" << td->age();

#else
#define PRETTY_TILE(idx, td)
#endif

void KisTileDataPoolerTest::testCycles()
{
    const qint32 pixelSize = 1;
    quint8 defaultPixel = 128;

    KisTileDataStore::instance()->debugClear();

    for(int i = 0; i < 12; i++) {
        KisTileData *td =
            KisTileDataStore::instance()->createDefaultTileData(pixelSize, &defaultPixel);

        for(int j = 0; j < 1 + (2 - i % 3); j++) {
            td->acquire();
        }

        if(!(i/6)) {
            td->markOld();
        }

        if(!((i / 3) & 1)) {
            td->m_clonesStack.push(new KisTileData(*td));
        }

        PRETTY_TILE(i, td);
    }

    {
        KisTileDataPooler pooler(KisTileDataStore::instance(), 5);
        pooler.start();
        pooler.kick();
        pooler.kick();

        QTest::qSleep(500);

        pooler.terminatePooler();
    }

    int i = 0;
    KisTileData *item;
    KisTileDataStoreIterator *iter =
        KisTileDataStore::instance()->beginIteration();

    while(iter->hasNext()) {
        item = iter->next();

        int expectedClones;

        switch(i) {
        case 6:
        case 7:
        case 10:
            expectedClones = 1;
            break;
        case 9:
            expectedClones = 2;
            break;
        default:
            expectedClones = 0;
        }

        PRETTY_TILE(i, item);
        if (item->m_clonesStack.size() != expectedClones) {
            qDebug() << ppVar(item->m_clonesStack.size()) << ppVar(expectedClones);
            QEXPECT_FAIL("", "The clonesStack's size is not as expected", Continue);
        }
        QCOMPARE(item->m_clonesStack.size(), expectedClones);
        i++;
    }


    KisTileDataStore::instance()->endIteration(iter);
    KisTileDataStore::instance()->debugClear();
}

QTEST_MAIN(KisTileDataPoolerTest)
