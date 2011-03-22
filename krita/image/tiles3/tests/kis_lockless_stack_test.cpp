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

#include "kis_lockless_stack_test.h"
#include <qtest_kde.h>

#include "kis_debug.h"

#include "tiles3/kis_lockless_stack.h"

void KisLocklessStackTest::testOperations()
{
    KisLocklessStack<int> stack;


    for(qint32 i = 0; i < 1024; i++) {
        stack.push(i);
    }

    QCOMPARE(stack.size(), 1024);

    for(qint32 i = 1023; i >= 0; i--) {
        int value;

        bool result = stack.pop(value);
        QVERIFY(result);

        QCOMPARE(value, i);
    }

    QVERIFY(stack.isEmpty());

}

/************ BENCHMARKING INFRASTRACTURE ************************/

#define NUM_TYPES 2

// high-concurrency
#define NUM_CYCLES 500000
#define NUM_THREADS 10

// relaxed
//#define NUM_CYCLES 100
//#define NUM_THREADS 2

// single-threaded
//#define NUM_CYCLES 10000000
//#define NUM_THREADS 1


class KisAbstractIntStack
{
public:
    virtual ~KisAbstractIntStack() {}
    virtual void push(int value) = 0;
    virtual int pop() = 0;
    virtual bool isEmpty() = 0;
    virtual void clear() = 0;
};

class KisTestingLocklessStack : public KisAbstractIntStack
{
public:
    void push(int value) {
        m_stack.push(value);
    }

    int pop() {
        int value  = 0;

        bool result = m_stack.pop(value);
        Q_ASSERT(result);
        Q_UNUSED(result); // for release build

        return value;
    }

    bool isEmpty() {
        return m_stack.isEmpty();
    }

    void clear() {
        m_stack.clear();
    }

private:
    KisLocklessStack<int> m_stack;
};

class KisTestingLegacyStack : public KisAbstractIntStack
{
public:
    void push(int value) {
        m_mutex.lock();
        m_stack.push(value);
        m_mutex.unlock();
    }

    int pop() {
        m_mutex.lock();
        int result = m_stack.pop();
        m_mutex.unlock();

        return result;
    }

    bool isEmpty() {
        m_mutex.lock();
        bool result = m_stack.isEmpty();
        m_mutex.unlock();

        return result;
    }

    void clear() {
        m_mutex.lock();
        m_stack.clear();
        m_mutex.unlock();
    }

private:
    QStack<int> m_stack;
    QMutex m_mutex;
};


class KisStressJob : public QRunnable
{
public:
    KisStressJob(KisAbstractIntStack &stack, qint32 startValue)
        : m_stack(stack), m_startValue(startValue)
    {
        m_pushSum = 0;
        m_popSum = 0;
    }

    void run() {
        for(qint32 i = 0; i < NUM_CYCLES; i++) {
            qint32 type = i % NUM_TYPES;
            int newValue;

            switch(type) {
            case 0:
                newValue = m_startValue + i;

                m_pushSum += newValue;
                m_stack.push(newValue);
                break;
            case 1:
                m_popSum += m_stack.pop();
                break;
            }
        }
    }

    qint64 pushSum() {
        return m_pushSum;
    }

    qint64 popSum() {
        return m_popSum;
    }

private:
    KisAbstractIntStack &m_stack;
    qint32 m_startValue;
    qint64 m_pushSum;
    qint64 m_popSum;
};

void KisLocklessStackTest::runStressTest(KisAbstractIntStack &stack)
{
    QList<KisStressJob*> jobsList;
    KisStressJob *job;

    for(qint32 i = 0; i < NUM_THREADS; i++) {
        job = new KisStressJob(stack, 1);
        job->setAutoDelete(false);
        jobsList.append(job);
    }

    QThreadPool pool;
    pool.setMaxThreadCount(NUM_THREADS);

    QBENCHMARK {
        foreach(job, jobsList) {
            pool.start(job);
        }

        pool.waitForDone();
    }

    QVERIFY(stack.isEmpty());

    qint64 totalSum = 0;

    for(qint32 i = 0; i < NUM_THREADS; i++) {
        KisStressJob *job = jobsList.takeLast();

        totalSum += job->pushSum();
        totalSum -= job->popSum();

        qDebug() << ppVar(totalSum);

        delete job;
    }

    QCOMPARE(totalSum, (long long) 0);
}

void KisLocklessStackTest::stressTestLockless()
{
    KisTestingLocklessStack stack;
    runStressTest(stack);
}

void KisLocklessStackTest::stressTestQStack()
{
    KisTestingLegacyStack stack;
    runStressTest(stack);
}

class KisStressClearJob : public QRunnable
{
public:
    KisStressClearJob(KisLocklessStack<int> &stack, qint32 startValue)
        : m_stack(stack), m_startValue(startValue)
    {
    }

    void run() {
        for(qint32 i = 0; i < NUM_CYCLES; i++) {
            qint32 type = i % 4;
            int newValue;

            switch(type) {
            case 0:
            case 1:
                newValue = m_startValue + i;
                m_stack.push(newValue);
                break;
            case 2:
                int tmp;
                m_stack.pop(tmp);
                break;
            case 3:
                m_stack.clear();
                break;
            }
        }
    }

private:
    KisLocklessStack<int> &m_stack;
    qint32 m_startValue;
};

void KisLocklessStackTest::stressTestClear()
{
    KisLocklessStack<int> stack;
    KisStressClearJob *job;

    QThreadPool pool;
    pool.setMaxThreadCount(NUM_THREADS);

    for(qint32 i = 0; i < NUM_THREADS; i++) {
        job = new KisStressClearJob(stack, 1);
        pool.start(job);
    }
    pool.waitForDone();

    stack.clear();
    QVERIFY(stack.isEmpty());
}

QTEST_KDEMAIN(KisLocklessStackTest, NoGUI)
#include "kis_lockless_stack_test.moc"

