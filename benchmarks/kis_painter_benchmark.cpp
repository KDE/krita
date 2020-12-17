/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý lukast.dev @gmail.com
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#if defined(_WIN32) || defined(_WIN64)
#include <stdlib.h>
#define srand48 srand
inline double drand48()
{
    return double(rand()) / RAND_MAX;
}
#endif

#include <QTest>

#include <QImage>
#include <kis_debug.h>

#include "kis_painter_benchmark.h"
#include "kis_benchmark_values.h"

#include "kis_paint_device.h"
#include "kis_fixed_paint_device.h"

#include "kis_selection.h"
#include "kis_pixel_selection.h"

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoCompositeOpRegistry.h>
#include <KoColor.h>

#include <kis_image.h>
#include <kis_painter.h>
#include <kis_types.h>
#include "kis_paintop_utils.h"
#include "kis_algebra_2d.h"
#include "kis_paint_device_debug_utils.h"
#include "KisRenderedDab.h"


#define SAVE_OUTPUT

#define CYCLES 20
static const int LINE_COUNT = 100;
static const int LINE_WIDTH = 1;

void KisPainterBenchmark::initTestCase()
{
    m_colorSpace = KoColorSpaceRegistry::instance()->rgb8();    
    
    m_color = KoColor(m_colorSpace);
    m_color.fromQColor(Qt::red);
    
    
    srand48(0);
    for (int i = 0; i < LINE_COUNT ;i++){
        m_points.append( QPointF(drand48() * TEST_IMAGE_WIDTH, drand48() * TEST_IMAGE_HEIGHT) );
        m_points.append( QPointF(drand48() * TEST_IMAGE_WIDTH, drand48() * TEST_IMAGE_HEIGHT) );
    }
}

void KisPainterBenchmark::cleanupTestCase()
{
}

void KisPainterBenchmark::benchmarkBitBlt()
{
    KisPaintDeviceSP src = new KisPaintDevice(m_colorSpace);
    KisPaintDeviceSP dst = new KisPaintDevice(m_colorSpace);
    src->fill(0,0,TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT, m_color.data());
    dst->fill(0,0,TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT, m_color.data());
    
    KisPainter gc(dst);
    
    QPoint pos(0,0);
    QRect rc(0,0,TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT);
    
    QBENCHMARK{
        for (int i = 0; i < CYCLES ; i++){
            gc.bitBlt(pos,src,rc);
        }
    }

}

void KisPainterBenchmark::benchmarkFastBitBlt()
{
    KisPaintDeviceSP src = new KisPaintDevice(m_colorSpace);
    KisPaintDeviceSP dst = new KisPaintDevice(m_colorSpace);
    src->fill(0,0,TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT, m_color.data());
    dst->fill(0,0,TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT, m_color.data());

    KisPainter gc(dst);
    gc.setCompositeOp(m_colorSpace->compositeOp(COMPOSITE_COPY));

    QPoint pos(0,0);
    QRect rc(0,0,TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT);

    QBENCHMARK{
        for (int i = 0; i < CYCLES ; i++){
            gc.bitBlt(pos,src,rc);
        }
    }

}

void KisPainterBenchmark::benchmarkBitBltSelection()
{
    KisPaintDeviceSP src = new KisPaintDevice(m_colorSpace);
    KisPaintDeviceSP dst = new KisPaintDevice(m_colorSpace);
    src->fill(0,0,TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT, m_color.data());
    dst->fill(0,0,TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT, m_color.data());

    KisSelectionSP selection = new KisSelection();
    selection->pixelSelection()->select(QRect(0, 0, TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT));
    selection->updateProjection();

    
    KisPainter gc(dst);
    gc.setSelection(selection);
    
    QPoint pos(0,0);
    QRect rc(0,0,TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT);
    
    QBENCHMARK{
        for (int i = 0; i < CYCLES ; i++){
            gc.bitBlt(pos,src,rc);
        }
    }

    
}


void KisPainterBenchmark::benchmarkFixedBitBlt()
{
    QImage img(TEST_IMAGE_WIDTH,TEST_IMAGE_HEIGHT,QImage::Format_ARGB32);
    img.fill(255);

    KisFixedPaintDeviceSP fdev = new KisFixedPaintDevice(m_colorSpace);
    fdev->convertFromQImage(img, 0);

    KisPaintDeviceSP dst = new KisPaintDevice(m_colorSpace);
    KisPainter gc(dst);
    QPoint pos(0, 0);
    QRect rc = img.rect();

    QBENCHMARK{
        for (int i = 0; i < CYCLES ; i++){
            gc.bltFixed(pos,fdev,rc);
        }
    }
}


void KisPainterBenchmark::benchmarkFixedBitBltSelection()
{
    QImage img(TEST_IMAGE_WIDTH,TEST_IMAGE_HEIGHT,QImage::Format_ARGB32);
    img.fill(128);

    KisFixedPaintDeviceSP fdev = new KisFixedPaintDevice(m_colorSpace);
    fdev->convertFromQImage(img, 0);

    KisPaintDeviceSP dst = new KisPaintDevice(m_colorSpace);

    KisSelectionSP selection = new KisSelection();
    selection->pixelSelection()->select(QRect(0, 0, TEST_IMAGE_WIDTH , TEST_IMAGE_HEIGHT));
    selection->updateProjection();

    KisPainter gc(dst);
    gc.setSelection(selection);

    QPoint pos(0, 0);
    QRect rc = img.rect();

    QBENCHMARK{
        for (int i = 0; i < CYCLES ; i++){
            gc.bltFixed(pos,fdev,rc);
        }
    }

}

void KisPainterBenchmark::benchmarkDrawThickLine()
{
    KisPaintDeviceSP dev = new KisPaintDevice(m_colorSpace);
    KoColor color(m_colorSpace);
    color.fromQColor(Qt::white);
    
    dev->clear();
    dev->fill(0,0,TEST_IMAGE_WIDTH,TEST_IMAGE_HEIGHT,color.data());
    
    color.fromQColor(Qt::black);
    
    KisPainter painter(dev);
    painter.setPaintColor(color);
    
    QBENCHMARK{
        for (int i = 0; i < LINE_COUNT; i++){
            painter.drawThickLine(m_points[i*2],m_points[i*2+1],LINE_WIDTH,LINE_WIDTH);
        }
    }
#ifdef SAVE_OUTPUT    
    dev->convertToQImage(m_colorSpace->profile()).save("drawThickLine.png");
#endif
}


void KisPainterBenchmark::benchmarkDrawQtLine()
{
    KisPaintDeviceSP dev = new KisPaintDevice(m_colorSpace);
    KoColor color(m_colorSpace);
    color.fromQColor(Qt::white);
    
    dev->clear();
    dev->fill(0,0,TEST_IMAGE_WIDTH,TEST_IMAGE_HEIGHT,color.data());
    
    color.fromQColor(Qt::black);
    
    KisPainter painter(dev);
    painter.setPaintColor(color);
    painter.setFillStyle(KisPainter::FillStyleForegroundColor);
    
    QPen pen;
    pen.setWidth(LINE_WIDTH);
    pen.setColor(Qt::white);
    pen.setCapStyle(Qt::RoundCap);
    
    QBENCHMARK{
        for (int i = 0; i < LINE_COUNT; i++){
            QPainterPath path;
            path.moveTo(m_points[i*2]);
            path.lineTo(m_points[i*2 + 1]);
            painter.drawPainterPath(path, pen);
        }
    }
#ifdef SAVE_OUTPUT        
    dev->convertToQImage(m_colorSpace->profile(),0,0,TEST_IMAGE_WIDTH,TEST_IMAGE_HEIGHT).save("drawQtLine.png");
#endif
}

void KisPainterBenchmark::benchmarkDrawScanLine()
{
    KisPaintDeviceSP dev = new KisPaintDevice(m_colorSpace);
    KoColor color(m_colorSpace);
    color.fromQColor(Qt::white);
    
    dev->clear();
    dev->fill(0,0,TEST_IMAGE_WIDTH,TEST_IMAGE_HEIGHT,color.data());
    
    color.fromQColor(Qt::black);
    
    KisPainter painter(dev);
    painter.setPaintColor(color);
    painter.setFillStyle(KisPainter::FillStyleForegroundColor);
    
    
    QBENCHMARK{
        for (int i = 0; i < LINE_COUNT; i++){
            painter.drawLine(m_points[i*2],m_points[i*2+1],LINE_WIDTH,true);
        }
    }
#ifdef SAVE_OUTPUT    
    dev->convertToQImage(m_colorSpace->profile(),0,0,TEST_IMAGE_WIDTH,TEST_IMAGE_HEIGHT).save("drawScanLine.png");
#endif
}

void KisPainterBenchmark::benchmarkBitBlt2()
{
    quint8 p = 128;
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->alpha8();

    KisPaintDeviceSP src = new KisPaintDevice(cs);
    KisPaintDeviceSP dst = new KisPaintDevice(cs);

    KoColor color(&p, cs);
    QRect fillRect(0,0,5000,5000);

    src->fill(fillRect, color);

    QBENCHMARK {
        KisPainter gc(dst);
        gc.bitBlt(QPoint(), src, fillRect);
    }
}

void KisPainterBenchmark::benchmarkBitBltOldData()
{
    quint8 p = 128;
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->alpha8();

    KisPaintDeviceSP src = new KisPaintDevice(cs);
    KisPaintDeviceSP dst = new KisPaintDevice(cs);

    KoColor color(&p, cs);
    QRect fillRect(0,0,5000,5000);

    src->fill(fillRect, color);

    QBENCHMARK {
        KisPainter gc(dst);
        gc.bitBltOldData(QPoint(), src, fillRect);
    }
}


void benchmarkMassiveBltFixedImpl(int numDabs, int size, qreal spacing, int idealNumPatches, Qt::Orientations direction)
{
    const KoColorSpace* cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dst = new KisPaintDevice(cs);

    QList<QColor> colors;
    colors << QColor(255, 0, 0, 200);
    colors << QColor(0, 255, 0, 200);
    colors << QColor(0, 0, 255, 200);

    QRect devicesRect;
    QList<KisRenderedDab> devices;

    const int step = spacing * size;

    for (int i = 0; i < numDabs; i++) {
        const QRect rc =
            direction == Qt::Horizontal ? QRect(10 + i * step, 0, size, size) :
            direction == Qt::Vertical ? QRect(0, 10 + i * step, size, size) :
            QRect(10 + i * step, 10 + i * step, size, size);

        KisFixedPaintDeviceSP dev = new KisFixedPaintDevice(cs);
        dev->setRect(rc);
        dev->initialize();
        dev->fill(rc, KoColor(colors[i % 3], cs));
        dev->fill(kisGrowRect(rc, -5), KoColor(Qt::white, cs));

        KisRenderedDab dab;
        dab.device = dev;
        dab.offset = dev->bounds().topLeft();
        dab.opacity = 1.0;
        dab.flow = 1.0;

        devices << dab;
        devicesRect |= rc;
    }

    const QRect fullRect = kisGrowRect(devicesRect, 10);

    {
        KisPainter painter(dst);
        painter.bltFixed(fullRect, devices);
        painter.end();
        //QVERIFY(TestUtil::checkQImage(dst->convertToQImage(0, fullRect),
        //                              "kispainter_test",
        //                              "massive_bitblt_benchmark",
        //                              "initial"));
        dst->clear();
    }


    QVector<QRect> dabRects;
    Q_FOREACH (const KisRenderedDab &dab, devices) {
        dabRects.append(dab.realBounds());
    }

    QElapsedTimer t;

    qint64 massiveTime = 0;
    int massiveTries = 0;
    int numRects = 0;
    int avgPatchSize = 0;

    for (int i = 0; i < 50 || massiveTime > 5000000; i++) {
        QVector<QRect> rects = KisPaintOpUtils::splitDabsIntoRects(dabRects, idealNumPatches, size, spacing);
        numRects = rects.size();

        // HACK: please calculate real *average*!
        avgPatchSize = KisAlgebra2D::maxDimension(rects.first());

        t.start();

        KisPainter painter(dst);
        Q_FOREACH (const QRect &rc, rects) {
            painter.bltFixed(rc, devices);
        }
        painter.end();

        massiveTime += t.nsecsElapsed() / 1000;
        massiveTries++;
        dst->clear();
    }

    qint64 linearTime = 0;
    int linearTries = 0;

    for (int i = 0; i < 50 || linearTime > 5000000; i++) {
        t.start();

        KisPainter painter(dst);
        Q_FOREACH (const KisRenderedDab &dab, devices) {
            painter.setOpacity(255 * dab.opacity);
            painter.setFlow(255 * dab.flow);
            painter.bltFixed(dab.offset, dab.device, dab.device->bounds());
        }
        painter.end();

        linearTime += t.nsecsElapsed() / 1000;
        linearTries++;
        dst->clear();
    }

    const qreal avgMassive = qreal(massiveTime) / massiveTries;
    const qreal avgLinear = qreal(linearTime) / linearTries;

    const QString directionMark =
        direction == Qt::Horizontal ? "H" :
        direction == Qt::Vertical ? "V" : "D";

    qDebug()
            << "D:" << size
            << "S:" << spacing
            << "N:" << numDabs
            << "P (px):" << avgPatchSize
            << "R:" << numRects
            << "Dir:" << directionMark
            << "\t"
            << qPrintable(QString("Massive (usec): %1").arg(QString::number(avgMassive, 'f', 2), 8))
            << "\t"
            << qPrintable(QString("Linear (usec): %1").arg(QString::number(avgLinear, 'f', 2), 8))
            << (avgMassive < avgLinear ? "*" : " ")
            << qPrintable(QString("%1")
                   .arg(QString::number((avgMassive - avgLinear) / avgLinear * 100.0, 'f', 2), 8))
            << qRound(size + size * spacing * (numDabs - 1));
}


void KisPainterBenchmark::benchmarkMassiveBltFixed()
{
    const qreal sp = 0.14;
    const int idealThreadCount = 8;

    for (int d = 50; d < 301; d += 50) {
        for (int n = 1; n < 150; n = qCeil(n * 1.5)) {
            benchmarkMassiveBltFixedImpl(n, d, sp, idealThreadCount, Qt::Horizontal);
            benchmarkMassiveBltFixedImpl(n, d, sp, idealThreadCount, Qt::Vertical);
            benchmarkMassiveBltFixedImpl(n, d, sp, idealThreadCount, Qt::Vertical | Qt::Horizontal);
        }
    }
}



QTEST_MAIN(KisPainterBenchmark)
