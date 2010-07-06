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

#include "kis_memory_pool_test.h"
#include <qtest_kde.h>

#include "kis_debug.h"

#include "tiles3/kis_memory_pool.h"

#define OBJECT_SIZE 16384 //bytes (size of a usual tile data)
#define POOL_SIZE 32
#define NUM_THREADS 2
#define NUM_CYCLES 100000
#define NUM_OBJECTS 64 // for tiles - 64 == area of 512x512px

class TestingClass
{
private:
    quint8 m_data[OBJECT_SIZE];
};

typedef KisMemoryPool<TestingClass, POOL_SIZE> TestingMemoryPool;


class KisPoolStressJob : public QRunnable
{
public:
    KisPoolStressJob(TestingMemoryPool &pool)
        : m_pool(pool)
    {
    }

    void run() {
        qsrand(QTime::currentTime().msec());
        for(qint32 i = 0; i < NUM_CYCLES; i++) {
            qint32 type = i % 2;

            switch(type) {
            case 0:
                for(qint32 j = 0; j < NUM_OBJECTS; j++) {
                    m_pointer[j] = m_pool.pop();
                    Q_ASSERT(m_pointer[j]);
                    // check for sigsegv ;)
                    ((quint8*)m_pointer[j])[0] = i;
                }
                break;
            case 1:
                for(qint32 j = 0; j < NUM_OBJECTS; j++) {
                    // are we the only writers here?
                    Q_ASSERT(((quint8*)m_pointer[j])[0] == (i-1) % 256);
                    m_pool.push(m_pointer[j]);
                }
                break;
            }
        }
    }

private:
    void* m_pointer[NUM_OBJECTS];
    TestingMemoryPool &m_pool;
};

void KisMemoryPoolTest::benchmarkMemoryPool()
{
    TestingMemoryPool memoryPool;

    QThreadPool pool;
    pool.setMaxThreadCount(NUM_THREADS);

    QBENCHMARK {
        for(qint32 i = 0; i < NUM_THREADS; i++) {
            KisPoolStressJob *job = new KisPoolStressJob(memoryPool);
            pool.start(job);
        }

        pool.waitForDone();
    }
}


class KisAllocStressJob : public QRunnable
{
public:
    KisAllocStressJob()
    {
    }

    void run() {
        qsrand(QTime::currentTime().msec());
        for(qint32 i = 0; i < NUM_CYCLES; i++) {
            qint32 type = i % 2;

            switch(type) {
            case 0:
                for(qint32 j = 0; j < NUM_OBJECTS; j++) {
                    m_pointer[j] = new TestingClass;
                    Q_ASSERT(m_pointer[j]);
                    // check for sigsegv ;)
                    ((quint8*)m_pointer[j])[0] = i;
                }
                break;
            case 1:
                for(qint32 j = 0; j < NUM_OBJECTS; j++) {
                    // are we the only writers here?
                    Q_ASSERT(((quint8*)m_pointer[j])[0] == (i-1) % 256);
                    delete (TestingClass*)m_pointer[j];
                }
                break;
            }
        }
    }

private:
    void* m_pointer[NUM_OBJECTS];
};

void KisMemoryPoolTest::benchmarkAlloc()
{
    QThreadPool pool;
    pool.setMaxThreadCount(NUM_THREADS);

    QBENCHMARK {
        for(qint32 i = 0; i < NUM_THREADS; i++) {
            KisAllocStressJob *job = new KisAllocStressJob();
            pool.start(job);
        }

        pool.waitForDone();
    }
}


QTEST_KDEMAIN(KisMemoryPoolTest, NoGUI)
#include "kis_memory_pool_test.moc"

