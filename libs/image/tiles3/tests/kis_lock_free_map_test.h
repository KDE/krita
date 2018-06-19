#ifndef KIS_LOCK_FREE_MAP_TEST_H
#define KIS_LOCK_FREE_MAP_TEST_H

#include <QTest>

class LockFreeMapTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testMainOperations();
};

#endif // KIS_LOCK_FREE_MAP_TEST_H
