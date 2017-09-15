/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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

#include "KisSharedThreadPoolAdapterTest.h"

#include <QTest>

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


QTEST_MAIN(KisSharedThreadPoolAdapterTest)
