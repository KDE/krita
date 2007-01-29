#include "TestAction.h"

#include <kdebug.h>
#include <KoAction.h>
#include <KoExecutePolicy.h>
#include <ThreadWeaver.h>

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

    KoExecutePolicy *policies[3] = { KoExecutePolicy::onlyLastPolicy,
        KoExecutePolicy::queuedPolicy,
        KoExecutePolicy::simpleQueuedPolicy };
    for(int i=0; i < 3; i++) {
        action.setExecutePolicy(policies[i]);
        m_notifier->count = 0;
        qDebug() << "i:" << i;
        QMutex mutex;
        mutex.lock();
        action.execute();
        bool success = m_notifier->waiter.wait(&mutex, 250);
        mutex.unlock();
        QCOMPARE(success, true);
        QCOMPARE(m_notifier->count, 1);
    }

   //QVERIFY(manager.shapeAt(QPointF(200, 200)) == 0);
}

QTEST_MAIN(TestAction)
#include "TestAction.moc"
