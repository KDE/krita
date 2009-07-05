#ifndef PRIORITYQUEUETEST_H
#define PRIORITYQUEUETEST_H

#include <QObject>
#include <QtTest/QtTest>

class PriorityQueue_test : public QObject
{
    Q_OBJECT
private slots:
    void testQueue();
};

#endif
