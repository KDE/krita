#ifndef KIS_LOCK_FREE_MAP_TEST_H
#define KIS_LOCK_FREE_MAP_TEST_H

#include <QTest>

class LockfreeMapTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testOperations();
    void stressTestLockless();
    void iteratorTest();
};

#endif // KIS_LOCK_FREE_MAP_TEST_H
