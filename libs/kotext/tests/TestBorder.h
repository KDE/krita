#ifndef TESTBORDER_H
#define TESTBORDER_H

#include <QObject>
#include <QtTest/QtTest>

class TestBorder : public QObject {
    Q_OBJECT
public:
    TestBorder() {}

private slots:
    void testBorder();

};

#endif
