#ifndef TESTTABLEFORMATS_H
#define TESTTABLEFORMATS_H

#include <QObject>
#include <QtTest>

class TestTableFormats : public QObject
{
    Q_OBJECT
public:
    TestTableFormats() {}

private Q_SLOTS:
    void testTableColumnFormat();
    void testTableRowFormat();
};

#endif // TESTTABLEFORMATS_H
