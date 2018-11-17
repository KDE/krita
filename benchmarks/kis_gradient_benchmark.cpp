/*
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
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

#include <QTest>
#include <kundo2command.h>

#include "kis_benchmark_values.h"

#include "kis_random_accessor_ng.h"

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColor.h>

#include <kis_image.h>

#include "kis_gradient_benchmark.h"

#include <kis_gradient_painter.h>

#include <KoCompositeOps.h>
#include <resources/KoStopGradient.h>

void KisGradientBenchmark::initTestCase()
{
    m_colorSpace = KoColorSpaceRegistry::instance()->rgb8();    
    m_device = new KisPaintDevice(m_colorSpace);
    m_color = KoColor(m_colorSpace);
    
    m_color.fromQColor(QColor(0,0,0,0)); // default pixel
    m_device->fill( 0,0,GMP_IMAGE_WIDTH, GMP_IMAGE_HEIGHT,m_color.data() );
}

void KisGradientBenchmark::benchmarkGradient()
{
    KoColor fg(m_colorSpace);
    KoColor bg(m_colorSpace);
    fg.fromQColor(Qt::blue);
    bg.fromQColor(Qt::black);
    
    QBENCHMARK
    {
        QLinearGradient grad;
        grad.setColorAt(0, Qt::white);
        grad.setColorAt(1.0, Qt::red);
        KoAbstractGradientSP kograd(KoStopGradient::fromQGradient(&grad));
        Q_ASSERT(kograd);
        KisGradientPainter fillPainter(m_device);
        //setupPainter(&fillPainter);
        fillPainter.setGradient(kograd);

        fillPainter.beginTransaction(kundo2_noi18n("Gradient Fill"));

        //fillPainter.setProgress(updater->startSubtask());
        fillPainter.setOpacity(OPACITY_OPAQUE_U8);
        // default
        fillPainter.setCompositeOp(COMPOSITE_OVER);
        fillPainter.setGradientShape(KisGradientPainter::GradientShapeBiLinear);
        fillPainter.paintGradient(QPointF(0,0), QPointF(3000,3000), KisGradientPainter::GradientRepeatNone, 1.0, false, 0, 0, GMP_IMAGE_WIDTH,GMP_IMAGE_HEIGHT);

        fillPainter.deleteTransaction();
    }
    
    // uncomment this to see the output
    QImage out = m_device->convertToQImage(m_colorSpace->profile(),0,0,GMP_IMAGE_WIDTH,GMP_IMAGE_HEIGHT);
    out.save("fill_output.png");
}


void KisGradientBenchmark::cleanupTestCase()
{

}

QTEST_MAIN(KisGradientBenchmark)
