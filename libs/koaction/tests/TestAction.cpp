/*
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "TestAction.h"

#include <kdebug.h>
#include <KoAction.h>
#include <KoExecutePolicy.h>
#include <threadweaver/ThreadWeaver.h>

class TestAction::Notifier
{
public:
    Notifier() : count(0) {}
    void notify() {
        count++;
        waiter.wakeAll();
    }

    int count;
    QWaitCondition waiter;
    QMutex lock;
};

TestAction::TestAction()
        : m_notifier(new TestAction::Notifier())
{
}

TestAction::~TestAction()
{
    delete m_notifier;
    m_notifier = 0;
}

void TestAction::test()
{
    m_notifier->count = 0;
    KoAction action;
    action.setExecutePolicy(KoExecutePolicy::directPolicy);
    connect(&action, SIGNAL(triggered(const QVariant &)), this, SLOT(notify()), Qt::DirectConnection);
    action.setWeaver(ThreadWeaver::Weaver::instance());
    action.execute();

    QCOMPARE(m_notifier->count, 1);

    KoExecutePolicy *policies[4] = { KoExecutePolicy::onlyLastPolicy,
                                     KoExecutePolicy::queuedPolicy,
                                     KoExecutePolicy::directPolicy,
                                     KoExecutePolicy::simpleQueuedPolicy
                                   };
    for (int i = 0; i < 4; i++) {
        action.setExecutePolicy(policies[i]);
        m_notifier->count = 0;
        //qDebug() << " test " << i+1;
        QMutex mutex;
        mutex.lock();
        action.execute();
        QTest::qSleep(250); // allow the action to do its job.
        QCoreApplication::processEvents(); // allow the actions 'gui' stuff to run.
        bool success = m_notifier->count != 0 || m_notifier->waiter.wait(&mutex, 550);
        mutex.unlock();
        QCOMPARE(success, true);
        QCOMPARE(m_notifier->count, 1);
    }
}

void TestAction::testExecuteTwice()
{
    m_notifier->count = 0;
    KoAction action;
    action.setExecutePolicy(KoExecutePolicy::directPolicy);
    connect(&action, SIGNAL(triggered(const QVariant &)), this, SLOT(notify()), Qt::DirectConnection);
    action.setWeaver(ThreadWeaver::Weaver::instance());
    action.execute();

    QCOMPARE(m_notifier->count, 1);

    KoExecutePolicy *policies[4] = { KoExecutePolicy::onlyLastPolicy,
                                     KoExecutePolicy::queuedPolicy,
                                     KoExecutePolicy::directPolicy,
                                     KoExecutePolicy::simpleQueuedPolicy
                                   };
    for (int i = 0; i < 4; i++) {
        action.setExecutePolicy(policies[i]);
        m_notifier->count = 0;
        //qDebug() << " test " << i+1;
        QMutex mutex;
        mutex.lock();
        action.execute();
        QTest::qSleep(250); // allow the action to do its job.
        QCoreApplication::processEvents(); // allow the actions 'gui' stuff to run.
        action.execute();
        QTest::qSleep(250); // allow the action to do its job.
        QCoreApplication::processEvents(); // allow the actions 'gui' stuff to run.
        bool success = m_notifier->count != 0 || m_notifier->waiter.wait(&mutex, 550);
        mutex.unlock();
        QCOMPARE(success, true);
        QCOMPARE(m_notifier->count, 2);
    }
}

QTEST_MAIN(TestAction)
#include "TestAction.moc"
