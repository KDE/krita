#ifndef TESTTABLECELLSTYLE_H
#define TESTTABLECELLSTYLE_H

#include <QObject>
#include <QtTest/QtTest>

class TestTableCellStyle : public QObject
{
    Q_OBJECT
public:
    TestTableCellStyle() {}

private slots:
    void testTableCellStyle();

};

#endif // TESTTABLECELLSTYLE_H
