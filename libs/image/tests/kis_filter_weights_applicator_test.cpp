/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_filter_weights_applicator_test.h"

#include <simpletest.h>

#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include "kis_paint_device.h"
#include "kistest.h"

#include <sstream>

//#define DEBUG_ENABLED
#include "kis_filter_weights_applicator.h"

void debugSpan(const KisFilterWeightsApplicator::BlendSpan &span)
{
    qDebug() << ppVar(span.weights->centerIndex);
    for (int i = 0; i < span.weights->span; i++) {
        qDebug() << "Weights" << i << span.weights->weight[i];
    }

    qDebug() << ppVar(span.firstBlendPixel);
    qDebug() << ppVar(span.offset);
    qDebug() << ppVar(span.offsetInc);
}

void testSpan(qreal scale, qreal dx, int dst_l,
              int expectedFirstPixel,
              qreal expectedOffset,
              qreal expectedOffsetInc)
{
    KisFilterStrategy *filter = new KisBilinearFilterStrategy();

    KisFilterWeightsBuffer buf(filter, qAbs(scale));
    KisFilterWeightsApplicator applicator(0, 0, scale, 0.0, dx, false);
    KisFilterWeightsApplicator::BlendSpan span;
    span = applicator.calculateBlendSpan(dst_l, 0, &buf);

    //debugSpan(span);

    if (span.firstBlendPixel != expectedFirstPixel ||
        span.offset != KisFixedPoint(expectedOffset) ||
        span.offsetInc != KisFixedPoint(expectedOffsetInc)) {

        qDebug() << "Failed to generate a span:";
        qDebug() << ppVar(scale) << ppVar(dx) << ppVar(dst_l);
        qDebug() << ppVar(span.firstBlendPixel) << ppVar(expectedFirstPixel);
        qDebug() << ppVar(span.offset) << ppVar(KisFixedPoint(expectedOffset));
        qDebug() << ppVar(span.offsetInc) << ppVar(KisFixedPoint(expectedOffsetInc));
        QFAIL("fail");
    }
}

void KisFilterWeightsApplicatorTest::testSpan_Scale_2_0_Aligned()
{
    testSpan(2.0, 0.0, 0, -1, 0.25, 1.0);
    testSpan(2.0, 0.0, 1, 0, 0.75, 1.0);
    testSpan(2.0, 0.0, -1, -1, 0.75, 1.0);
    testSpan(2.0, 0.0, -2, -2, 0.25, 1.0);
}

void KisFilterWeightsApplicatorTest::testSpan_Scale_2_0_Shift_0_5()
{
    testSpan(2.0, 0.5, 0, -1, 0.5, 1.0);
    testSpan(2.0, 0.5, 1, -1, 0.0, 1.0);
    testSpan(2.0, 0.5, -1, -2, 0.0, 1.0);
    testSpan(2.0, 0.5, -2, -2, 0.5, 1.0);
}

void KisFilterWeightsApplicatorTest::testSpan_Scale_2_0_Shift_0_75()
{
    testSpan(2.0, 0.75, 0, -1, 0.625, 1.0);
    testSpan(2.0, 0.75, 1, -1, 0.125, 1.0);
    testSpan(2.0, 0.75, -1, -2, 0.125, 1.0);
    testSpan(2.0, 0.75, -2, -2, 0.625, 1.0);
}

void KisFilterWeightsApplicatorTest::testSpan_Scale_0_5_Aligned()
{
    testSpan(0.5, 0.0, 0, -1, 0.25, 0.5);
    testSpan(0.5, 0.0, 1, 1, 0.25, 0.5);
    testSpan(0.5, 0.0, -1, -3, 0.25, 0.5);
}

void KisFilterWeightsApplicatorTest::testSpan_Scale_0_5_Shift_0_5()
{
    testSpan(0.5, 0.5, 0, -2, 0.25, 0.5);
    testSpan(0.5, 0.5, 1, 0, 0.25, 0.5);
    testSpan(0.5, 0.5, -1, -4, 0.25, 0.5);
}

void KisFilterWeightsApplicatorTest::testSpan_Scale_0_5_Shift_0_25()
{
    testSpan(0.5, 0.25, 0, -2, 0.0, 0.5);
    testSpan(0.5, 0.25, 1, 0, 0.0, 0.5);
    testSpan(0.5, 0.25, -1, -4, 0.0, 0.5);
}

void KisFilterWeightsApplicatorTest::testSpan_Scale_0_5_Shift_0_375()
{
    testSpan(0.5, 0.375, 0, -2, 0.125, 0.5);
    testSpan(0.5, 0.375, 1, 0, 0.125, 0.5);
    testSpan(0.5, 0.375, -1, -4, 0.125, 0.5);
}

void KisFilterWeightsApplicatorTest::testSpan_Scale_0_5_Shift_m0_5()
{
    testSpan(0.5, -0.5, 0, 0, 0.25, 0.5);
    testSpan(0.5, -0.5, 1, 2, 0.25, 0.5);
    testSpan(0.5, -0.5, -1, -2, 0.25, 0.5);
}

void KisFilterWeightsApplicatorTest::testSpan_Scale_0_5_Shift_m0_25()
{
    testSpan(0.5, -0.25, 0, -1, 0.0, 0.5);
    testSpan(0.5, -0.25, 1, 1, 0.0, 0.5);
    testSpan(0.5, -0.25, -1, -3, 0.0, 0.5);
}

void KisFilterWeightsApplicatorTest::testSpan_Scale_0_5_Shift_m0_375()
{
    testSpan(0.5, -0.375, 0, 0, 0.375, 0.5);
    testSpan(0.5, -0.375, 1, 2, 0.375, 0.5);
    testSpan(0.5, -0.375, -1, -2, 0.375, 0.5);
}

void KisFilterWeightsApplicatorTest::testSpan_Scale_1_0_Aligned_Mirrored()
{
    testSpan(-1.0, 0.0, 0, -2, 0.0, 1.0);
    testSpan(-1.0, 0.0, 1, -3, 0.0, 1.0);
    testSpan(-1.0, 0.0, -1, -1, 0.0, 1.0);
    testSpan(-1.0, 0.0, -2, 0, 0.0, 1.0);
}

void KisFilterWeightsApplicatorTest::testSpan_Scale_0_5_Aligned_Mirrored()
{
    testSpan(-0.5, 0.0, 0, -3, 0.25, 0.5);
    testSpan(-0.5, 0.0, 1, -5, 0.25, 0.5);
    testSpan(-0.5, 0.0, -1, -1, 0.25, 0.5);
    testSpan(-0.5, 0.0, -2, 1, 0.25, 0.5);
}

void KisFilterWeightsApplicatorTest::testSpan_Scale_0_5_Shift_0_125_Mirrored()
{
    testSpan(-0.5, 0.125, 0, -3, 0.125, 0.5);
    testSpan(-0.5, 0.125, 1, -5, 0.125, 0.5);
    testSpan(-0.5, 0.125, -1, -1, 0.125, 0.5);
    testSpan(-0.5, 0.125, -2, 1, 0.125, 0.5);
}

void printPixels(KisPaintDeviceSP dev, int x0, int len, bool horizontal, bool dense = false)
{
    std::stringstream ss;
    for (int i = x0; i < x0 + len; i++) {
        QColor c;

        int x = horizontal ? i : 0;
        int y = horizontal ? 0 : i;

        dev->pixel(x, y, &c);
        if (dense){
            ss << c.red() << " , " << c.alpha() << " | ";
        } else {
            qDebug() << "px" << x << y << "|" << c.red() << c.green() << c.blue() << c.alpha();
        }
    }

    if (dense) {
        qDebug() << ss.str().c_str();
    }
}

void checkRA(KisPaintDeviceSP dev, int x0, int len, quint8 r[], quint8 a[], bool horizontal)
{
    bool failed = false;
    for (int i = 0; i < len; i++) {
        QColor c;

        int x = horizontal ? x0 + i : 0;
        int y = horizontal ? 0 : x0 + i;

        dev->pixel(x, y, &c);

        if (c.red() != r[i] ||
            c.alpha() != a[i]) {

            qDebug() << "Failed to compare RA channels:" << ppVar(x0 + i);
            qDebug() << "Red:" << c.red() << "Expected:" << r[i];
            qDebug() << "Alpha:" << c.alpha() << "Expected:" << a[i];
            failed = true;
        }
    }

    if (failed) {
        QFAIL("failed");
    }
}

void testLineImpl(qreal scale, qreal dx, quint8 expR[], quint8 expA[], int x0, int len, bool clampToEdge, bool horizontal, KisFilterStrategy *filter = 0, KisPaintDeviceSP dev = 0)
{
    int startPos = 0;
    int endPos = 4;
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    if (!filter) {
        filter = new KisBilinearFilterStrategy();
    }
    if (!dev) {
        dev = new KisPaintDevice(cs);

        for (int i = 0; i < 4; i++) {
            int x = horizontal ? i : 0;
            int y = horizontal ? 0 : i;
            dev->setPixel(x,y,QColor(10 + i * 10, 20 + i * 10, 40 + i * 10));
        }

        {
            quint8 r[] = {  0, 10, 20, 30, 40,  0,  0};
            quint8 a[] = {  0,255,255,255,255,  0,  0};
            checkRA(dev, -1, 6, r, a, horizontal);
        }

        startPos = 0;
        endPos = 4;

    } else {
        QRect rc = dev->exactBounds();
        if (horizontal) {
            startPos = rc.left();
            endPos = rc.left() + rc.width();
        } else {
            startPos = rc.top();
            endPos = rc.top() + rc.height();
        }
    }

    KisFilterWeightsBuffer buf(filter, qAbs(scale));
    KisFilterWeightsApplicator applicator(dev, dev, scale, 0.0, dx, clampToEdge);


    KisFilterWeightsApplicator::LinePos srcPos(startPos, endPos - startPos);
    KisFilterWeightsApplicator::LinePos dstPos;

    if (horizontal) {
        dstPos = applicator.processLine<KisHLineIteratorSP>(srcPos,0,&buf, filter->support(buf.weightsPositionScale().toFloat()));
    } else {
        dstPos = applicator.processLine<KisVLineIteratorSP>(srcPos,0,&buf, filter->support(buf.weightsPositionScale().toFloat()));
    }

    QRect rc = dev->exactBounds();

    if (horizontal) {
        QVERIFY(rc.left() >= dstPos.start());
        QVERIFY(rc.left() + rc.width() <= dstPos.end());
    } else {
        QVERIFY(rc.top() >= dstPos.start());
        QVERIFY(rc.top() + rc.height() <= dstPos.end());
    }

    //printPixels(dev, x0, len, horizontal, true);
    checkRA(dev, x0, len, expR, expA, horizontal);
}

void testLine(qreal scale, qreal dx, quint8 expR[], quint8 expA[], int x0, int len, bool clampToEdge = false, KisFilterStrategy* filter = 0, KisPaintDeviceSP dev = 0)
{
    testLineImpl(scale, dx, expR, expA, x0, len, clampToEdge, true, filter, dev);
    testLineImpl(scale, dx, expR, expA, x0, len, clampToEdge, false, filter, dev);
}

void KisFilterWeightsApplicatorTest::testProcessLine_Scale_1_0_Aligned()
{
    qreal scale = 1.0;
    qreal dx = 0.0;

    quint8 r[] = {  0, 10, 20, 30, 40,  0,  0};
    quint8 a[] = {  0,255,255,255,255,  0,  0};

    testLine(scale, dx, r, a, -1, 7);
}

void KisFilterWeightsApplicatorTest::testProcessLine_Scale_1_0_Shift_0_5()
{
    qreal scale = 1.0;
    qreal dx = 0.5;

    quint8 r[] = {  0, 10, 15, 25, 35, 40,  0};
    quint8 a[] = {  0,128,255,255,255,127,  0};

    testLine(scale, dx, r, a, -1, 7);
}

void KisFilterWeightsApplicatorTest::testProcessLine_Scale_1_0_Shift_m0_5()
{
    qreal scale = 1.0;
    qreal dx = -0.5;

    quint8 r[] = { 10, 15, 25, 35, 40,  0,  0};
    quint8 a[] = {128,255,255,255,127,  0,  0};

    testLine(scale, dx, r, a, -1, 7);
}

void KisFilterWeightsApplicatorTest::testProcessLine_Scale_1_0_Shift_0_25()
{
    qreal scale = 1.0;
    qreal dx = 0.25;

    quint8 r[] = {  0, 10, 17, 27, 37, 40,  0};
    quint8 a[] = {  0,191,255,255,255, 64,  0};

    testLine(scale, dx, r, a, -1, 7);
}

void KisFilterWeightsApplicatorTest::testProcessLine_Scale_1_0_Shift_m0_25()
{
    qreal scale = 1.0;
    qreal dx = -0.25;

    quint8 r[] = { 10, 13, 23, 33, 40,  0,  0};
    quint8 a[] = { 64,255,255,255,191,  0,  0};

    testLine(scale, dx, r, a, -1, 7);
}

void KisFilterWeightsApplicatorTest::testProcessLine_Scale_0_5_Aligned()
{
    qreal scale = 0.5;
    qreal dx = 0.0;

    quint8 r[] = { 10, 17, 33, 40,  0,  0,  0};
    quint8 a[] = { 32,223,223, 32,  0,  0,  0};

    testLine(scale, dx, r, a, -1, 7);
}

void KisFilterWeightsApplicatorTest::testProcessLine_Scale_0_5_Shift_0_25()
{
    qreal scale = 0.5;
    qreal dx = 0.25;

    quint8 r[] = {  0, 13, 30, 40,  0,  0,  0};
    quint8 a[] = {  0,191,255, 64,  0,  0,  0};

    testLine(scale, dx, r, a, -1, 7);
}

void KisFilterWeightsApplicatorTest::testProcessLine_Scale_2_0_Aligned()
{
    qreal scale = 2.0;
    qreal dx = 0.0;

    quint8 r[] = {  0, 10, 10, 13, 17, 23, 27, 33, 37, 40, 40,  0};
    quint8 a[] = {  0, 64,191,255,255,255,255,255,255,191, 64,  0};

    testLine(scale, dx, r, a, -2, 12);
}

void KisFilterWeightsApplicatorTest::testProcessLine_Scale_2_0_Shift_0_25()
{
    qreal scale = 2.0;
    qreal dx = 0.25;

    quint8 r[] = {  0, 10, 10, 11, 16, 21, 26, 31, 36, 40, 40,  0};
    quint8 a[] = {  0, 32,159,255,255,255,255,255,255,223, 96,  0};

    testLine(scale, dx, r, a, -2, 12);
}

void KisFilterWeightsApplicatorTest::testProcessLine_Scale_2_0_Shift_0_5()
{
    qreal scale = 2.0;
    qreal dx = 0.5;

    quint8 r[] = {  0,  0, 10, 10, 15, 20, 25, 30, 35, 40, 40,  0};
    quint8 a[] = {  0,  0,128,255,255,255,255,255,255,255,127,  0};

    testLine(scale, dx, r, a, -2, 12);
}

void KisFilterWeightsApplicatorTest::testProcessLine_Scale_1_0_Aligned_Clamped()
{
    qreal scale = 1.0;
    qreal dx = 0.0;

    quint8 r[] = {  0, 10, 20, 30, 40,  0,  0};
    quint8 a[] = {  0,255,255,255,255,  0,  0};

    testLine(scale, dx, r, a, -1, 7, true);
}

void KisFilterWeightsApplicatorTest::testProcessLine_Scale_0_5_Aligned_Clamped()
{
    qreal scale = 0.5;
    qreal dx = 0.0;

    quint8 r[] = {  0, 16, 34,  0,  0,  0,  0};
    quint8 a[] = {  0,255,255,  0,  0,  0,  0};

    testLine(scale, dx, r, a, -1, 7, true);
}

void KisFilterWeightsApplicatorTest::testProcessLine_Scale_2_0_Aligned_Clamped()
{
    qreal scale = 2.0;
    qreal dx = 0.0;

    quint8 r[] = {  0,  0, 10, 13, 17, 23, 27, 33, 37, 40,  0,  0};
    quint8 a[] = {  0,  0,255,255,255,255,255,255,255,255,  0,  0};

    testLine(scale, dx, r, a, -2, 12, true);
}

void KisFilterWeightsApplicatorTest::testProcessLine_Scale_1_0_Aligned_Mirrored()
{
    qreal scale = -1.0;
    qreal dx = 0.0;

    quint8 r[] = {  0,  0, 40, 30, 20, 10,  0,  0,  0,  0};
    quint8 a[] = {  0,  0,255,255,255,255,  0,  0,  0,  0};

    testLine(scale, dx, r, a, -6, 10);
}

void KisFilterWeightsApplicatorTest::testProcessLine_Scale_1_0_Shift_0_25_Mirrored()
{
    qreal scale = -1.0;
    qreal dx = 0.25;

    quint8 r[] = {  0,  0, 40, 33, 23, 13, 10,  0,  0,  0};
    quint8 a[] = {  0,  0,191,255,255,255, 64,  0,  0,  0};

    testLine(scale, dx, r, a, -6, 10);
}

void KisFilterWeightsApplicatorTest::testProcessLine_Scale_0_5_Aligned_Mirrored_Clamped()
{
    qreal scale = -0.5;
    qreal dx = 0.0;

    quint8 r[] = {  0,  0,  0,  0, 34, 16,  0,  0,  0,  0};
    quint8 a[] = {  0,  0,  0,  0,255,255,  0,  0,  0,  0};

    testLine(scale, dx, r, a, -6, 10, true);
}

void KisFilterWeightsApplicatorTest::testProcessLine_Scale_0_5_Shift_0_125_Mirrored()
{
    qreal scale = -0.5;
    qreal dx = 0.125;

    quint8 r[] = {  0,  0,  0, 40, 35, 19, 10,  0,  0,  0};
    quint8 a[] = {  0,  0,  0, 16,207,239, 48,  0,  0,  0};

    testLine(scale, dx, r, a, -6, 10);
}

void KisFilterWeightsApplicatorTest::testProcessLine_NearestNeighbourFilter_2x()
{
    qreal scale = 2.0;
    qreal dx = 0;

    quint8 r[] = {0,  10, 10,   20, 20,   30,  30,    40, 40, 0, 0};
    quint8 a[] = {0,  255, 255, 255, 255, 255,  255,  255, 255, 0, 0};

    KisFilterStrategy* filter = new KisBoxFilterStrategy();
    testLine(scale, dx, r, a, -1, 11, true, filter);
}

void KisFilterWeightsApplicatorTest::testProcessLine_NearestNeighbourFilter_1x()
{

    qreal scale = 1.0;
    qreal dx = 0;

    quint8 r[] = {  0, 10, 20, 30, 40,  0,  0};
    quint8 a[] = {  0,255,255,255,255,  0,  0};

    KisFilterStrategy* filter = new KisBoxFilterStrategy();
    testLine(scale, dx, r, a, -1, 7, false, filter);
}

void KisFilterWeightsApplicatorTest::testProcessLine_NearestNeighbourFilter_05x()
{

    qreal scale = 0.5;
    qreal dx = 0;

    quint8 r[] = {  0, 10, 30, 0, 0,  0,  0};
    quint8 a[] = {  0,255,255, 0, 0,  0,  0};

    KisFilterStrategy* filter = new KisBoxFilterStrategy();
    testLine(scale, dx, r, a, -1, 7, false, filter);
}


void KisFilterWeightsApplicatorTest::testProcessLine_NearestNeighbourFilter_077x()
{

    qreal scale = 0.77;
    qreal dx = 0;

    quint8 r[] = {  0, 10, 20, 40, 0,  0,  0};
    quint8 a[] = {  0,255,255, 255, 0,  0,  0};

    KisFilterStrategy* filter = new KisBoxFilterStrategy();
    testLine(scale, dx, r, a, -1, 7, false, filter);
}

void KisFilterWeightsApplicatorTest::testProcessLine_NearestNeighbourFilter_074x()
{

    qreal scale = 0.74;
    qreal dx = 0;

    quint8 r[] = {  0, 10, 30, 40, 0,  0,  0};
    quint8 a[] = {  0,255,255, 255, 0,  0,  0};

    KisFilterStrategy* filter = new KisBoxFilterStrategy();
    testLine(scale, dx, r, a, -1, 7, false, filter);
}

void KisFilterWeightsApplicatorTest::testProcessLine_NearestNeighbourFilter_075x()
{

    qreal scale = 0.75;
    qreal dx = 0;

    quint8 r[] = {  0, 10, 20, 40, 0,  0,  0};
    quint8 a[] = {  0,255,255, 255, 0,  0,  0};

    KisFilterStrategy* filter = new KisBoxFilterStrategy();
    testLine(scale, dx, r, a, -1, 7, false, filter);
}

void KisFilterWeightsApplicatorTest::testProcessLine_NearestNeighbourFilter_051x()
{

    qreal scale = 0.51;
    qreal dx = 0;

    quint8 r[] = {  0, 10, 30, 0, 0,  0,  0};
    quint8 a[] = {  0,255,255, 0, 0,  0,  0};

    KisFilterStrategy* filter = new KisBoxFilterStrategy();
    testLine(scale, dx, r, a, -1, 7, false, filter);
}



void KisFilterWeightsApplicatorTest::testProcessLine_NearestNeighbourFilter_15x()
{

    qreal scale = 1.5;
    qreal dx = 0;

    quint8 r[] = {  0, 10, 10, 20, 30,  30,  40};
    quint8 a[] = {  0,255,255, 255, 255,  255,  255};

    KisFilterStrategy* filter = new KisBoxFilterStrategy();
    testLine(scale, dx, r, a, -1, 7, false, filter);
}


void preparePixelData(quint8* r, quint8* a, int i)
{
    for (int j = 0; j < 7; j ++) {
        r[j] = 0;
        a[j] = 0;
    }

    if (i < 13) {
        // nothing to do
    } else if (i < 17) {
        r[1] = 40;
        a[1] = 255;
    } else if (i < 25) {
        r[1] = 30;
        a[1] = 255;
    } else if (i < 38) {
        r[1] = 20;
        a[1] = 255;
    } else if (i < 50) {
        r[1] = 20;
        r[2] = 40;
        a[1] = 255;
        a[2] = 255;
    } else if (i < 63) {
        r[1] = 10;
        r[2] = 30;
        a[1] = 255;
        a[2] = 255;
    } else if (i < 75) {
        r[1] = 10;
        r[2] = 30;
        r[3] = 40;

        a[1] = 255;
        a[2] = 255;
        a[3] = 255;

    } else if (i < 84) {
        r[1] = 10;
        r[2] = 20;
        r[3] = 40;

        a[1] = 255;
        a[2] = 255;
        a[3] = 255;

    } else if (i < 88) {
        r[1] = 10;
        r[2] = 20;
        r[3] = 30;

        a[1] = 255;
        a[2] = 255;
        a[3] = 255;
    } else {

        r[1] = 10;
        r[2] = 20;
        r[3] = 30;
        r[4] = 40;

        a[1] = 255;
        a[2] = 255;
        a[3] = 255;
        a[4] = 255;
    }

}


void KisFilterWeightsApplicatorTest::testProcessLine_NearestNeighbourFilter_all()
{

    KisFilterStrategy* filter = new KisBoxFilterStrategy();

    for (int i = 1; i < 100; i++) {

        qreal scale = i/100.0;
        qreal dx = 0;

        quint8 r[7];
        quint8 a[7];

        preparePixelData(r, a, i);
        testLine(scale, dx, r, a, -1, 7, false, filter);

    }
}




KisPaintDeviceSP prepareUniformPaintDevice(int pixelsNumber, bool horizontal)
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    for (int i = 0; i < pixelsNumber; i++) {
        int x = horizontal ? i : 0;
        int y = horizontal ? 0 : i;

        QColor c = QColor(10, 0, 0, 255);
        dev->setPixel(x, y, c);
    }

    return dev;
}

void prepareUniformPixels(quint8 r[], quint8 a[], int pixelsNumber, bool /*horizontal*/)
{
    for (int i = 0; i < pixelsNumber; i++) {

        QColor c = QColor(10, 0, 0, 255);
        r[i] = c.red();
        a[i] = c.alpha();
    }

}



void KisFilterWeightsApplicatorTest::testProcessLine_NearestNeighbourFilter_0098x_horizontal()
{
    int before = 5075;
    int after = 500;

    qreal scale = before/after;
    qreal dx = 0;

    bool horizontal = true;

    KisPaintDeviceSP dev = prepareUniformPaintDevice(before, horizontal);

    quint8 *r = new quint8[after];
    quint8 *a = new quint8[after];

    prepareUniformPixels(r, a, after, horizontal);

    KisFilterStrategy* filter = new KisBoxFilterStrategy();
    testLineImpl(scale, dx, r, a, 0, after, false, horizontal, filter, dev);

}

void KisFilterWeightsApplicatorTest::testProcessLine_NearestNeighbourFilter_0098x_vertical()
{
    int before = 4725;
    int after = 466;

    qreal scale = before/after;
    qreal dx = 0;

    bool horizontal = false;

    KisPaintDeviceSP dev = prepareUniformPaintDevice(before, horizontal);

    quint8 *r = new quint8[after];
    quint8 *a = new quint8[after];

    prepareUniformPixels(r, a, after, horizontal);

    KisFilterStrategy* filter = new KisBoxFilterStrategy();
    testLineImpl(scale, dx, r, a, 0, after, false, horizontal, filter, dev);

}


void KisFilterWeightsApplicatorTest::benchmarkProcesssLine()
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    KisFilterStrategy *filter = new KisBilinearFilterStrategy();

    const qreal scale = 0.873;
    const qreal dx = 0.0387;

    KisFilterWeightsBuffer buf(filter, qAbs(scale));
    KisFilterWeightsApplicator applicator(dev, dev, scale, 0.0, dx, false);

    for (int i = 0; i < 32767; i++) {
        dev->setPixel(i,0,QColor(10 + i%240,20,40));
    }

    KisFilterWeightsApplicator::LinePos linePos(0,32767);

    QBENCHMARK {
        applicator.processLine<KisHLineIteratorSP>(linePos,0,&buf, filter->support(buf.weightsPositionScale().toFloat()));
    }
}

KISTEST_MAIN(KisFilterWeightsApplicatorTest)
