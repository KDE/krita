/*
 *  SPDX-FileCopyrightText: 2010 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_chunk_allocator_test.h"
#include <QTest>

#include "kis_debug.h"

#include "../swap/kis_chunk_allocator.h"


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

    dbgKrita << "fragmentation(transactions)";
    for(qint32 t = 1; t < NUM_TRANSACTIONS; t += NUM_TRANSACTIONS/7) {
        qreal f = measureFragmentation(t, NUM_CHUNKS_ALLOC,
                                       NUM_CHUNKS_FREE, false);
        dbgKrita << t << f;
    }

    dbgKrita << "fragmentation(alloc)";
    for(qint32 t = 1; t < NUM_CHUNKS_ALLOC; t += NUM_CHUNKS_ALLOC/7) {
        qreal f = measureFragmentation(NUM_TRANSACTIONS,t,
                                       0.8*t, false);
        dbgKrita << t << f;
    }

    dbgKrita << "fragmentation(free)";
    for(qint32 t = NUM_CHUNKS_ALLOC/7; t < NUM_CHUNKS_ALLOC; t += NUM_CHUNKS_ALLOC/15) {
        qreal f = measureFragmentation(NUM_TRANSACTIONS,NUM_CHUNKS_ALLOC,
                                       t, false);
        dbgKrita << t << f;
    }

}


QTEST_MAIN(KisChunkAllocatorTest)

