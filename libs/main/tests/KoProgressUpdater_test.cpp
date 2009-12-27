/*
 *  Copyright (c) 2007 Boudewijn Rempt boud@valdyas.org
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
#include <qtest_kde.h>

#include "KoProgressUpdater_test.h"

#include "KoProgressUpdater.h"
#include "KoUpdater.h"
#include <QThread>

#include <QThread>
#include <threadweaver/ThreadWeaver.h>
#include <threadweaver/Job.h>

class TestWeaverJob : public ThreadWeaver::Job
{
public:

    TestWeaverJob( QObject * parent, QPointer<KoUpdater> updater, int steps = 10 )
        : ThreadWeaver::Job( parent )
        , m_updater(updater)
        , m_steps(steps)
        {
        }

    void run()
        {
            for (int i = 1; i < m_steps + 1; ++i) {
                for (int j = 1; j < 10000; ++j){}
                m_updater->setProgress((100 / m_steps) * i);
                if ( m_updater->interrupted() ) {
                    m_updater->setProgress(100);
                    return;
                }
            }
            m_updater->setProgress(100);
        }


protected:
    QPointer<KoUpdater> m_updater;
    int m_steps;
};


class TestJob : public QThread {

public:

    TestJob( QPointer<KoUpdater> updater, int steps = 10 )
        : QThread()
        , m_updater( updater )
        , m_steps( steps )
        {
        }

    virtual void run()
        {
            for (int i = 1; i < m_steps + 1; ++i) {
                sleep(1);
                m_updater->setProgress((100 / m_steps) * i);
                if ( m_updater->interrupted() ) {
                    m_updater->setProgress(100);
                    return;
                }
            }
            m_updater->setProgress(100);
        }

private:

    QPointer<KoUpdater> m_updater;
    int m_steps;
};

class TestProgressBar : public KoProgressProxy
{

public:

    int min;
    int max;
    int value;
    QString formatString;

    TestProgressBar()
        : min(0)
        , max(0)
        , value(0)
        {
        }

    int maximum() const
        {
            return max;
        }

    void setValue( int v )
        {
            value = v;
        }

    void setRange( int minimum, int maximum )
        {
            min = minimum;
            max = maximum;
        }

    void setFormat( const QString & format )
        {
            formatString = format;
        }

};

void KoProgressUpdaterTest::testCreation()
{
    TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    QPointer<KoUpdater> updater = pu.startSubtask();
    QCOMPARE(bar.min, 0);
    QCOMPARE(bar.max, 0);
    QCOMPARE(bar.value, 0);
    QVERIFY(bar.formatString.isNull());
    pu.start();
    QCOMPARE(bar.min, 0);
    QCOMPARE(bar.max, 99);
    QCOMPARE(bar.value, 0);
}

void KoProgressUpdaterTest::testSimpleProgress()
{
    TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    pu.start();
    QPointer<KoUpdater> updater = pu.startSubtask();
    updater->setProgress(50);
    QTest::qSleep(250); // allow the action to do its job.
    QCoreApplication::processEvents(); // allow the actions 'gui' stuff to run.
    updater->setProgress(100);
    QTest::qSleep(250); // allow the action to do its job.
    QCoreApplication::processEvents(); // allow the actions 'gui' stuff to run.
    QCOMPARE(bar.value, 100);
}
void KoProgressUpdaterTest::testSimpleThreadedProgress()
{
    TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    pu.start();
    QPointer<KoUpdater> u = pu.startSubtask();
    TestJob t(u);
    t.start();
    while (!t.isFinished()) {
        QTest::qSleep(250); // allow the action to do its job.
        QCoreApplication::processEvents(); // allow the actions 'gui' stuff to run.
    }
    QCOMPARE(bar.value, 100);
}

void KoProgressUpdaterTest::testSubUpdaters()
{
    TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    pu.start();
    QPointer<KoUpdater> u1 = pu.startSubtask(4);
    QPointer<KoUpdater> u2 = pu.startSubtask(6);
    u1->setProgress(100);
    QTest::qSleep(250); // allow the action to do its job.
    QCoreApplication::processEvents(); // allow the actions 'gui' stuff to run.
    QCOMPARE(bar.value, 40);
    u2->setProgress(100);
    QTest::qSleep(250); // allow the action to do its job.
    QCoreApplication::processEvents(); // allow the actions 'gui' stuff to run.
    QCOMPARE(bar.value, 100);
}

void KoProgressUpdaterTest::testThreadedSubUpdaters()
{
    TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    pu.start();
    QPointer<KoUpdater> u1 = pu.startSubtask(4);
    QPointer<KoUpdater> u2= pu.startSubtask(6);

    TestJob t1(u1, 4);
    TestJob t2(u2, 6);
    t1.start();
    t2.start();
    while ( t1.isRunning() || t2.isRunning() ) {
        QTest::qSleep(250); // allow the action to do its job.
        QCoreApplication::processEvents(); // allow the actions 'gui' stuff to run.
    }
    QCOMPARE(bar.value, 100);
}

void KoProgressUpdaterTest::testRecursiveProgress()
{
    TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    pu.start();
    QPointer<KoUpdater> u1 = pu.startSubtask();

    KoProgressUpdater pu2(u1);
    pu2.start();
    QPointer<KoUpdater> u2 = pu2.startSubtask();
    u2->setProgress(50);
    u2->setProgress(100);
    while(bar.value < 100) {
        QCoreApplication::processEvents();
        QTest::qSleep(250);
    }
    QCOMPARE(bar.value, 100);
}

void KoProgressUpdaterTest::testThreadedRecursiveProgress()
{
    TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    pu.start();
    QPointer<KoUpdater> u1 = pu.startSubtask();

    KoProgressUpdater pu2(u1);
    pu2.start();
    QPointer<KoUpdater> u2 = pu2.startSubtask();

    TestJob t1(u2);
    t1.start();

    while (t1.isRunning() ) {
        QTest::qSleep(250); // allow the action to do its job.
        QCoreApplication::processEvents(); // allow the actions 'gui' stuff to run.
    }

    while(bar.value < 100) {
        QCoreApplication::processEvents();
        QTest::qSleep(250);
    }
    QCOMPARE(bar.value, 100);
}

void KoProgressUpdaterTest::testFromWeaver()
{
    jobsdone = 0;

    TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    pu.start(10);
    ThreadWeaver::Weaver * weaver = new ThreadWeaver::Weaver();
    weaver->setMaximumNumberOfThreads( 4 );
    connect( weaver, SIGNAL( jobDone(ThreadWeaver::Job*) ), this, SLOT( jobDone( ThreadWeaver::Job* ) ) );
    for (int i = 0; i < 10; ++i) {
        QPointer<KoUpdater> up = pu.startSubtask();
        ThreadWeaver::Job * job = new TestWeaverJob(this, up, 10);
        weaver->enqueue(job);
    }
    while (!weaver->isIdle()) {
         QTest::qSleep(250);
         QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    }
    weaver->finish();
    delete weaver;
    QCOMPARE(jobsdone, 10);
}

void KoProgressUpdaterTest::jobDone(ThreadWeaver::Job* job)
{
    Q_UNUSED(job);
    ++jobsdone;
}

QTEST_KDEMAIN(KoProgressUpdaterTest, GUI)
#include <KoProgressUpdater_test.moc>
