/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisSharedThreadPoolAdapterTest.h"

#include <simpletest.h>

#include <QThreadPool>
#include <QAtomicInt>
#include <KisSharedThreadPoolAdapter.h>

#include "kis_debug.h"

struct NastyCounter : public KisSharedRunnable
{
    NastyCounter(QAtomicInt *value)
        : m_value(value)
    {
    }

    void runShared() override {
        for (int i = 0; i < numCycles(); i++) {
            QThread::msleep(qrand() % 10);
            m_value->ref();
        }
    }

    static int numCycles() {
        return 100;
    }

private:
    QAtomicInt *m_value;
};


void KisSharedThreadPoolAdapterTest::test()
{
    QAtomicInt value;

    KisSharedThreadPoolAdapter adapter(QThreadPool::globalInstance());

    const int numThreads = 30;

    for (int i = 0; i < numThreads; i++) {
        adapter.start(new NastyCounter(&value));
    }

    adapter.waitForDone();

    QCOMPARE(int(value), numThreads * NastyCounter::numCycles());
}

// TODO: test waitForDone on empty queue!!!!


SIMPLE_TEST_MAIN(KisSharedThreadPoolAdapterTest)
