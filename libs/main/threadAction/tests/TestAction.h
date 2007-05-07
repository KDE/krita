#ifndef TESTACTION_H
#define TESTACTION_H

#include <QtTest/QtTest>

class TestAction : public QObject
{
    Q_OBJECT
public:
    TestAction();
    ~TestAction();

private:
    class Notifier;
    Notifier *m_notifier;
    Q_PRIVATE_SLOT(m_notifier, void notify())

private slots:
    // tests
    void test();
};

#endif
