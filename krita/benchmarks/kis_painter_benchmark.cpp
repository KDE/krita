/*
 *  Copyright (c) 2010 Lukáš Tvrdý lukast.dev@gmail.com
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#if defined(_WIN32) || defined(_WIN64)
#include <stdlib.h>
#define srand48 srand
inline double drand48()
{
    return double(rand()) / RAND_MAX;
}
#endif

#include <qtest_kde.h>

#include <QImage>
#include <QDebug>

#include "kis_painter_benchmark.h"
#include "kis_benchmark_values.h"

#include "kis_paint_device.h"
#include "kis_fixed_paint_device.h"

#include "kis_selection.h"
#include "kis_pixel_selection.h"

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoCompositeOp.h>
#include <KoColor.h>

#include <kis_image.h>
#include <kis_painter.h>
#include <kis_types.h>

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
    selection->getOrCreatePixelSelection()->select(QRect(0, 0, TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT));
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
    selection->getOrCreatePixelSelection()->select(QRect(0, 0, TEST_IMAGE_WIDTH , TEST_IMAGE_HEIGHT));
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

QTEST_KDEMAIN(KisPainterBenchmark, NoGUI)
#include "kis_painter_benchmark.moc"
