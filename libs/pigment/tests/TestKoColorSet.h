#include <QtTest>
#include "KoColorSet.h"

class TestKoColorSet : public QObject
{
private Q_SLOTS:
    void initTestCase();
    void testAdding();
private:
    KoColorSet testColorSet;
}
