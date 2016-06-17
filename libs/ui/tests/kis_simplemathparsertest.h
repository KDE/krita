#ifndef KISSIMPLEMATHPARSERTEST_H
#define KISSIMPLEMATHPARSERTEST_H

#include <QObject>

class KisSimpleMathParserTest : public QObject
{
    Q_OBJECT

public:
    KisSimpleMathParserTest();

private Q_SLOTS:
    void testDoubleComputation();
    void testIntComputation();
    void testIntFlooring();
};

#endif // KISSIMPLEMATHPARSERTEST_H
