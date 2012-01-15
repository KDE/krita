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

#include "kis_chunk_allocator_test.h"
#include <qtest_kde.h>

#include "kis_debug.h"

#include "tiles3/swap/kis_chunk_allocator.h"


void KisChunkAllocatorTest::testOperations()
{
    KisChunkAllocator allocator;

    allocator.getChunk(10);
    allocator.getChunk(15);
    KisChunk chunk3 = allocator.getChunk(20);
    allocator.getChunk(25);
    allocator.getChunk(30);

    allocator.freeChunk(chunk3);
    chunk3 = allocator.getChunk(20);

    allocator.debugChunks();
    allocator.sanityCheck();
    QVERIFY(qFuzzyCompare(allocator.debugFragmentation(), 1./6));
}


#define NUM_TRANSACTIONS 30
#define NUM_CHUNKS_ALLOC 15000
#define NUM_CHUNKS_FREE 12000
#define CHUNK_AV_SIZE 1024*12
#define CHUNK_DEV_SIZE 1024*4
#define SWAP_SIZE (1ULL * CHUNK_AV_SIZE * NUM_CHUNKS_ALLOC * NUM_TRANSACTIONS)

quint64 getChunkSize()
{
    quint64 deviation = qrand() % (2 * CHUNK_DEV_SIZE);
    return CHUNK_AV_SIZE - CHUNK_DEV_SIZE + deviation;
}


qreal KisChunkAllocatorTest::measureFragmentation(qint32 transactions,
                                                  qint32 chunksAlloc,
                                                  qint32 chunksFree,
                                                  bool printDetails)
{
    KisChunkAllocator allocator(DEFAULT_SLAB_SIZE, SWAP_SIZE);
    QList<KisChunk> chunks;

    for(qint32 k = 0; k < transactions; k++) {
        if(chunks.size() > 0) {
            for(qint32 i = 0; i < chunksFree; i++) {
                qint32 idx = qrand() % chunks.size();
                allocator.freeChunk(chunks.takeAt(idx));
            }
        }
        allocator.sanityCheck();

        for(qint32 i = 0; i < chunksAlloc; i++) {
            chunks.append(allocator.getChunk(getChunkSize()));
        }
        allocator.sanityCheck();
    }

    allocator.sanityCheck();
    return allocator.debugFragmentation(printDetails);
}

void KisChunkAllocatorTest::testFragmentation()
{

    qsrand(QTime::currentTime().msec());

    measureFragmentation(NUM_TRANSACTIONS, NUM_CHUNKS_ALLOC,
                         NUM_CHUNKS_FREE, true);

    /**
     * The following tests are too slow, so we disable them by default
     */
    return;

    qDebug() << "fragmentation(transactions)";
    for(qint32 t = 1; t < NUM_TRANSACTIONS; t += NUM_TRANSACTIONS/7) {
        qreal f = measureFragmentation(t, NUM_CHUNKS_ALLOC,
                                       NUM_CHUNKS_FREE, false);
        qDebug() << t << f;
    }

    qDebug() << "fragmentation(alloc)";
    for(qint32 t = 1; t < NUM_CHUNKS_ALLOC; t += NUM_CHUNKS_ALLOC/7) {
        qreal f = measureFragmentation(NUM_TRANSACTIONS,t,
                                       0.8*t, false);
        qDebug() << t << f;
    }

    qDebug() << "fragmentation(free)";
    for(qint32 t = NUM_CHUNKS_ALLOC/7; t < NUM_CHUNKS_ALLOC; t += NUM_CHUNKS_ALLOC/15) {
        qreal f = measureFragmentation(NUM_TRANSACTIONS,NUM_CHUNKS_ALLOC,
                                       t, false);
        qDebug() << t << f;
    }

}


QTEST_KDEMAIN(KisChunkAllocatorTest, NoGUI)
#include "kis_chunk_allocator_test.moc"

