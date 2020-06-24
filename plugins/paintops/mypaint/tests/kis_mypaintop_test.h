#ifndef KIS_MYPAINTOP_TEST_H
#define KIS_MYPAINTOP_TEST_H

#include <QObject>
#include <stroke_testing_utils.h>
#include <qimage_based_test.h>
#include <QTest>
#include <QtTest/QtTest>

class KisMyPaintOpTest: public QObject, public TestUtil::QImageBasedTest
{
    Q_OBJECT
public:
    KisMyPaintOpTest();
    virtual ~KisMyPaintOpTest() {}

private Q_SLOTS:
    void testDab();
    void testGetColor();

};

#include "kis_mypaintop_test.moc"
#endif // KIS_MYPAINTOP_TEST_H
