#ifndef KIS_MULTI_PAINT_DEVICE_TESTER_H
#define KIS_MULTI_PAINT_DEVICE_TESTER_H

#include <QtTest>

#include <KoColor.h>
#include <KoColorSpace.h>

class KisMultiPaintDeviceTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void testContextCreation();
    void testContextSwitching();
    void testDuplicateContextCreation();
    void testContextDropping();

    void cleanupTestCase();

private:

    const KoColorSpace *cs;
    quint8* red;
    quint8* green;
    quint8* blue;

};

#endif



