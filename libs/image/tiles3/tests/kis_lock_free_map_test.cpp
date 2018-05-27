#include "kis_lock_free_map_test.h"

#include <QDebug>

#include "kis_debug.h"
#include "tiles3/LockFreeMap/ConcurrentMap_Leapfrog.h"
#include "kis_shared_ptr.h"

#define NUM_TYPES 2

// high-concurrency
#define NUM_CYCLES 50000
#define NUM_THREADS 10

typedef ConcurrentMap_Leapfrog<qint32, qint32> ConcurrentMap;

class StressJobLockless : public QRunnable
{
public:
    StressJobLockless(ConcurrentMap &map)
        : m_map(map), m_insertSum(0), m_eraseSum(0)
    {
    }

    qint64 insertSum()
    {
        return m_insertSum;
    }

    qint64 eraseSum()
    {
        return m_eraseSum;
    }

protected:
    void run() override
    {
        QSBR::Context context = QSBR::instance().createContext();

        for (int i = 1; i < NUM_CYCLES + 1; i++) {
            auto type = i % NUM_TYPES;

            switch (type) {
            case 0:
                m_eraseSum += m_map.erase(i);
                break;
            case 1:
                m_eraseSum += m_map.assign(i + 1, i + 1);
                m_insertSum += i + 1;
                break;
            }

            if (i % 10000 == 0) {
                QSBR::instance().update(context);
            }
        }

        QSBR::instance().destroyContext(context);
    }

private:
    ConcurrentMap &m_map;
    qint64 m_insertSum;
    qint64 m_eraseSum;
};

void LockfreeMapTest::testOperations()
{
    ConcurrentMap map;
    qint64 totalSum = 0;

    for (auto i = 1; i < NUM_CYCLES + 1; i++) {
        totalSum += i + 1;
        map.assign(i, i + 1);
    }

    for (auto i = 1; i < NUM_CYCLES + 1; i++) {
        ConcurrentMap::Value result = map.erase(i);
        totalSum -= result;

        QVERIFY(result);
        QCOMPARE(i, result - 1);
    }

    QVERIFY(totalSum == 0);
}

void LockfreeMapTest::stressTestLockless()
{
    QList<StressJobLockless *> jobsList;
    ConcurrentMap map;

    for (auto i = 0; i < NUM_THREADS; ++i) {
        StressJobLockless *task = new StressJobLockless(map);
        task->setAutoDelete(false);
        jobsList.append(task);
    }

    QThreadPool pool;
    pool.setMaxThreadCount(NUM_THREADS);

    QBENCHMARK {
        for (auto &job : jobsList)
        {
            pool.start(job);
        }

        pool.waitForDone();
    }

    qint64 totalSum = 0;

    for (auto i = 0; i < NUM_THREADS; i++) {
        StressJobLockless *job = jobsList.takeLast();
        totalSum += job->insertSum();
        totalSum -= job->eraseSum();

        delete job;
    }

    QVERIFY(totalSum == 0);
}

void LockfreeMapTest::iteratorTest()
{
    ConcurrentMap map;
    qint32 sum = 0;
    for (qint32 i = 2; i < 100; ++i) {
        map.assign(i, i);
        sum += i;
    }

    ConcurrentMap::Iterator iter(map);
    qint32 testSum = 0;
    while (iter.isValid()) {
        testSum += iter.getValue();
        iter.next();
    }

    QVERIFY(sum == testSum);
}

class StressJobWrapper : public QRunnable
{
public:
    StressJobWrapper(KisLockFreeMap<Foo*> &map) : m_map(map) {}

protected:
    void run() override
    {
        for (int i = 1; i < NUM_CYCLES + 1; i++) {
            auto type = i % NUM_TYPES;

            switch (type) {
            case 0:
                m_map.erase(i);
                break;
            case 1:
                m_map.insert(i + 1, new Foo);
                break;
            }
        }
    }

private:
    KisLockFreeMap<Foo*> &m_map;
};

void LockfreeMapTest::testWrapper()
{
    QThreadPool pool;
    pool.setMaxThreadCount(NUM_THREADS);
    KisLockFreeMap<Foo*> map;

    for (auto i = 0; i < NUM_THREADS; ++i) {
        StressJobWrapper *task = new StressJobWrapper(map);
        task->setAutoDelete(true);
        pool.start(task);
    }

    pool.waitForDone();

    ConcurrentMap_Leapfrog<qint32, Foo*>::Iterator iter(map.m_map);

    qint32 sum = 0;
    while (iter.isValid()) {
        if (iter.getValue()) {
            ++sum;
        }
        iter.next();
    }
    qDebug() << sum;
}

QTEST_GUILESS_MAIN(LockfreeMapTest)
