#ifndef TESTTABLEDATA_H
#define TESTTABLEDATA_H

#include <QObject>
#include <QtTest>

class TestTableData : public QObject
{
    Q_OBJECT

public:
    TestTableData() {}

private slots:
    void testCellContentRect();
};

#endif // TESTTABLEDATA_H
