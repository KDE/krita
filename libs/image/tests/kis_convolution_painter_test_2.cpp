/*
 *  Copyright (c) 2007 Boudewijn Rempt boud@valdyas.org
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

#include <compositeops/KoVcMultiArchBuildSupport.h> //MSVC requires that Vc come first
#include "kis_convolution_painter_test_2.h"

#include <QTest>

#include <QBitArray>

#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorSpaceTraits.h>

#include "kis_paint_device.h"
#include "kis_convolution_painter.h"
#include "kis_convolution_kernel.h"
#include <kis_gaussian_kernel.h>
#include <kis_mask_generator.h>
#include "testutil.h"


void KisConvolutionPainterTest::testGaussianBase(KisPaintDeviceSP dev, bool useFftw, const QString &prefix)
{
   QBitArray channelFlags =
       KoColorSpaceRegistry::instance()->rgb8()->channelFlags(true, true);

   KisPainter gc(dev);


   qreal horizontalRadius = 5, verticalRadius = 5;
   
   for(int i = 0; i < 3 ; i++, horizontalRadius+=5, verticalRadius+=5)
   {
       QTime timer;
       timer.start();

       gc.beginTransaction();

       if (( horizontalRadius > 0 ) && ( verticalRadius > 0 )) {
           KisPaintDeviceSP interm = new KisPaintDevice(dev->colorSpace());

           KisConvolutionKernelSP kernelHoriz = KisGaussianKernel::createHorizontalKernel(horizontalRadius);
           KisConvolutionKernelSP kernelVertical = KisGaussianKernel::createVerticalKernel(verticalRadius);

           const QRect applyRect = dev->exactBounds();

           KisConvolutionPainter::TestingEnginePreference enginePreference =
               useFftw ?
               KisConvolutionPainter::FFTW :
               KisConvolutionPainter::SPATIAL;

           KisConvolutionPainter horizPainter(interm, enginePreference);
           horizPainter.setChannelFlags(channelFlags);
           horizPainter.applyMatrix(kernelHoriz, dev,
                                    applyRect.topLeft() - QPoint(0, verticalRadius),
                                    applyRect.topLeft() - QPoint(0, verticalRadius),
                                    applyRect.size() + QSize(0, 2 * verticalRadius),
                                    BORDER_REPEAT);

           KisConvolutionPainter verticalPainter(dev, enginePreference);
           verticalPainter.setChannelFlags(channelFlags);
           verticalPainter.applyMatrix(kernelVertical, interm,
                                       applyRect.topLeft(),
                                       applyRect.topLeft(),
                                       applyRect.size(), BORDER_REPEAT);

           QImage result = dev->convertToQImage(0, applyRect.x(), applyRect.y(), applyRect.width(), applyRect.height());

           QString engine = useFftw ? "fftw" : "spatial";
           QString testCaseName = QString("test_gaussian_%1_%2_%3.png").arg(horizontalRadius).arg(verticalRadius).arg(engine);

           TestUtil::checkQImage(result,
                                 "convolution_painter_test",
                                 QString("gaussian_") + prefix,
                                 testCaseName);

           gc.revertTransaction();
       }
       dbgKrita << "Elapsed time:" << timer.elapsed() << "ms";
    }
}

void KisConvolutionPainterTest::testGaussianDetails(bool useFftw)
{
    QImage referenceImage(TestUtil::fetchDataFileLazy("resolution_test.png"));
    KisPaintDeviceSP dev = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
    dev->convertFromQImage(referenceImage, 0, 0, 0);

    testGaussianBase(dev, useFftw, "details");
}

void KisConvolutionPainterTest::testGaussianDetailsSpatial()
{
    testGaussianDetails(false);
}

void KisConvolutionPainterTest::testGaussianDetailsFFTW()
{
    testGaussianDetails(true);
}

QTEST_MAIN(KisConvolutionPainterTest)
