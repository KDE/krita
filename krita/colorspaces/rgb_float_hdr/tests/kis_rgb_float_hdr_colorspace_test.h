#ifndef KIS_RGB_FLOAT_HDR_COLORSPACE_TEST_H
#define KIS_RGB_FLOAT_HDR_COLORSPACE_TEST_H

#include <QtTest/QtTest>

class KisRgbFloatHDRColorSpaceTest : public QObject
{
    Q_OBJECT

private slots:
    void testFactory();
    void testProfile();
};

#endif

