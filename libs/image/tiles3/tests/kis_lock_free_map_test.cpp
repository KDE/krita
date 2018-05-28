#include "kis_lock_free_map_test.h"

#include <QDebug>

#include "kis_debug.h"
#include "tiles3/kis_memento_item.h"
#include "tiles3/kis_tile_hash_table2.h"

#define NUM_TYPES 3

// high-concurrency
#define NUM_CYCLES 60000
#define NUM_THREADS 10

class Wrapper : public KisShared
{
public:
    Wrapper(qint32 member) : m_member(member) {}

    qint32 member()
    {
        return m_member;
    }

private:
    qint32 m_member;
};

class StressJobWrapper : public QRunnable
{
public:
    StressJobWrapper(KisTileHashTableTraits2<Wrapper> &map)
        : m_map(map), m_eraseSum(0), m_insertSum(0)
    {
    }

    qint64 eraseSum()
    {
        return m_eraseSum;
    }

    qint64 insertSum()
    {
        return m_insertSum;
    }

protected:
    void run() override
    {
        for (qint32 i = 1; i < NUM_CYCLES + 1; ++i) {
            auto type = i % NUM_TYPES;

            switch (type) {
            case 0: {
                auto result = m_map.erase(i - 2);
                if (result.data()) {
                    m_eraseSum += result->member();
                }
                break;
            }
            case 1: {
                auto result = m_map.insert(i, new Wrapper(i));
                if (result.data()) {
                    m_insertSum -= result->member();
                }
                m_insertSum += i;
                break;
            }
            case 2: {
                m_map.get(i - 1);
                break;
            }
            }
        }
    }

private:
    KisTileHashTableTraits2<Wrapper> &m_map;
    qint64 m_eraseSum;
    qint64 m_insertSum;
    qint64 m_getSum;
};

void LockFreeMapTest::testWrapper()
{
    QList<StressJobWrapper *> jobs;
    KisTileHashTableTraits2<Wrapper> map;

    for (qint32 i = 0; i < NUM_THREADS; ++i) {
        StressJobWrapper *job = new StressJobWrapper(map);
        job->setAutoDelete(false);
        jobs.append(job);
    }

    QThreadPool pool;
    pool.setMaxThreadCount(NUM_THREADS);

    QBENCHMARK {
        for (auto &job : jobs)
        {
            pool.start(job);
        }

        pool.waitForDone();
    }

    qint64 insertSum = 0;
    qint64 eraseSum = 0;

    for (auto i = 0; i < NUM_THREADS; i++) {
        StressJobWrapper *job = jobs.takeLast();
        eraseSum += job->eraseSum();
        insertSum += job->insertSum();

        delete job;
    }

    QVERIFY(insertSum == eraseSum);
}

QTEST_GUILESS_MAIN(LockFreeMapTest)
