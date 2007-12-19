#ifndef TESTLOADING_H
#define TESTLOADING_H

#include <QObject>
#include <QtTest/QtTest>

class TestLoading : public QObject {
    Q_OBJECT
public:
    TestLoading();

private slots: // tests
    void testLoadLists1();
};

#endif
