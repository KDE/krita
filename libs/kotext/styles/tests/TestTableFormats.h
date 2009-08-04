#ifndef TESTTABLEFORMATS_H
#define TESTTABLEFORMATS_H

#include <QObject>
#include <QtTest/QtTest>

class TestTableFormats : public QObject
{
    Q_OBJECT
public:
    TestTableFormats() {}

private slots:
    void testTableColumnFormat();
    void testTableRowFormat();
};

#endif // TESTTABLEFORMATS_H
