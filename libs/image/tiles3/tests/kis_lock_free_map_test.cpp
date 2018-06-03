#include "kis_lock_free_map_test.h"

#include <QDebug>

#include "kis_debug.h"
#include "tiles3/kis_memento_item.h"
#include "tiles3/kis_tile_hash_table2.h"

#define NUM_THREADS 10

class Wrapper : public KisShared
{
public:
    Wrapper() : m_member(0) {}
    Wrapper(qint32 col, qint32 row,
            KisTileData *defaultTileData, KisMementoManager* mm)
        : m_member(col) {}

    qint32 member()
    {
        return m_member;
    }

private:
    qint32 m_member;
};

class StressJob : public QRunnable
{
public:
    StressJob(const std::function<void(qint64 &, qint64 &)> func)
        : m_func(func), m_eraseSum(0), m_insertSum(0)
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
        m_func(m_eraseSum, m_insertSum);
    }

private:
    const std::function<void(qint64 &, qint64 &)> m_func;
    qint64 m_eraseSum;
    qint64 m_insertSum;
};

void LockFreeMapTest::testMainOperations()
{
    const qint32 numCycles = 60000;
    const qint32 numTypes = 3;
    QList<StressJob *> jobs;
    KisTileHashTableTraits2<Wrapper> map;

    auto func = [&](qint64 & eraseSum, qint64 & insertSum) {
        for (qint32 i = 1; i < numCycles + 1; ++i) {
            auto type = i % numTypes;

            switch (type) {
            case 0: {
                auto result = map.erase(i - 2);
                if (result.data()) {
                    eraseSum += result->member();
                }
                break;
            }
            case 1: {
                auto result = map.insert(i, KisSharedPtr<Wrapper>(new Wrapper(i, 0, 0, 0)));
                if (result.data()) {
                    insertSum -= result->member();
                }
                insertSum += i;
                break;
            }
            case 2: {
                map.get(i - 1);
                break;
            }
            }
        }
    };

    for (qint32 i = 0; i < NUM_THREADS; ++i) {
        StressJob *job = new StressJob(func);
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

    for (qint32 i = 0; i < NUM_THREADS; ++i) {
        StressJob *job = jobs.takeLast();
        eraseSum += job->eraseSum();
        insertSum += job->insertSum();

        delete job;
    }

    QVERIFY(insertSum == eraseSum);
}

void LockFreeMapTest::testLazy()
{
    const qint32 numCycles = 50000;
    const qint32 numTypes = 2;
    QList<StressJob *> jobs;
    KisTileHashTableTraits2<Wrapper> map;

    auto func = [&](qint64 & eraseSum, qint64 & insertSum) {
        for (qint32 i = 1; i < numCycles + 1; ++i) {
            auto type = i % numTypes;

            switch (type) {
            case 0: {
                auto result = map.erase(i - 1);
                if (result.data()) {
                    eraseSum += result->member();
                }
                break;
            }
            case 1: {
                auto result = map.getLazy(i, KisSharedPtr<Wrapper>(new Wrapper()));
                if (result.data()) {
                    insertSum += result->member();
                }
                break;
            }
            }
        }
    };

    for (qint32 i = 0; i < NUM_THREADS; ++i) {
        StressJob *job = new StressJob(func);
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

    for (qint32 i = 0; i < NUM_THREADS; ++i) {
        StressJob *job = jobs.takeLast();
        eraseSum += job->eraseSum();
        insertSum += job->insertSum();

        delete job;
    }

    QVERIFY(insertSum == eraseSum);
}

QTEST_GUILESS_MAIN(LockFreeMapTest)
