#ifndef TESTTABLECELLSTYLE_H
#define TESTTABLECELLSTYLE_H

#include <QObject>

class TestTableCellStyle : public QObject
{
    Q_OBJECT
public:
    TestTableCellStyle() {}

private Q_SLOTS:
    void testPen();
    void testPadding();
    void testSpacing();
    void testMargin();

};

#endif // TESTTABLECELLSTYLE_H
