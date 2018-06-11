#include <QTest>
#include "TestKoColorSet.h"
#include "KoColorSet.h"

void TestKoColorSet::initTestCase()
{ }

void TestKoColorSet::testAdding()
{
    QColor qc(255, 255, 255);
    KoColor c(qc, KoColorSpaceRegistry::instance()->rgb8());
    testColorSet.add(c);
    QCOMPARE(testColorSet.getColorGlobal(0, 0), c);
}
