#include "TestAction.h"

#include <kdebug.h>
#include <KoAction.h>
#include <KoExecutePolicy.h>
#include <threadweaver/ThreadWeaver.h>

class TestAction::Notifier {
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

TestAction::~TestAction() {
    delete m_notifier;
    m_notifier = 0;
}

void TestAction::test() {
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
        KoExecutePolicy::simpleQueuedPolicy };
    for(int i=0; i < 4; i++) {
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

QTEST_MAIN(TestAction)
#include "TestAction.moc"
