/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt boud @valdyas.org
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <compositeops/KoVcMultiArchBuildSupport.h> //MSVC requires that Vc come first
#include "kis_convolution_painter_test.h"

#include <simpletest.h>

#include <QBitArray>
#include <QElapsedTimer>

#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorSpaceTraits.h>

#include "kis_paint_device.h"
#include "kis_convolution_painter.h"
#include "kis_convolution_kernel.h"
#include <kis_gaussian_kernel.h>
#include <kis_mask_generator.h>
#include <kistest.h>
#include "testutil.h"
#include "testing_timed_default_bounds.h"

KisPaintDeviceSP initAsymTestDevice(QRect &imageRect, int &pixelSize, QByteArray &initialData)
{
    KisPaintDeviceSP dev = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
    pixelSize = dev->pixelSize();

    imageRect = QRect(0,0,5,5);
    initialData.resize(25 * pixelSize);

    quint8 *ptr = (quint8*) initialData.data();
    for(int i = 0; i < 25; i++) {
        KoColor pixel(QColor(i,i,i,255), dev->colorSpace());
        memcpy(ptr, pixel.data(), pixelSize);

        ptr += pixelSize;
    }

    dev->writeBytes((const quint8*)initialData.constData(), imageRect);

    return dev;
}

Eigen::Matrix<qreal, 3, 3> initSymmFilter(qreal &offset, qreal &factor)
{
    Eigen::Matrix<qreal, 3, 3> filter;
    filter(0,0) = 1.0 / 21;
    filter(0,1) = 3.0 / 21;
    filter(0,2) = 1.0 / 21;

    filter(1,0) = 3.0 / 21;
    filter(1,1) = 5.0 / 21;
    filter(1,2) = 3.0 / 21;

    filter(2,0) = 1.0 / 21;
    filter(2,1) = 3.0 / 21;
    filter(2,2) = 1.0 / 21;

    offset = 0.0;
    factor = 1.0;

    return filter;
}

Eigen::Matrix<qreal, 3, 3> initAsymmFilter(qreal &offset, qreal &factor)
{
    Eigen::Matrix<qreal, 3, 3> filter;
    filter(0,0) = 1.0;
    filter(1,0) = 2.0;
    filter(2,0) = 1.0;

    filter(0,1) = 0.0;
    filter(1,1) = 1.0;
    filter(2,1) = 0.0;

    filter(0,2) =-1.0;
    filter(1,2) =-2.0;
    filter(2,2) =-1.0;

    offset = 0.0;
    factor = 1.0;

    return filter;
}

void printPixel(QString prefix, int pixelSize, quint8 *data) {
    QString str = prefix;

    for(int i = 0; i < pixelSize; i++) {
        str += ' ';
        str += QString::number(data[i]);
    }

    dbgKrita << str;
}

void KisConvolutionPainterTest::testIdentityConvolution()
{
    QImage qimage(QString(FILES_DATA_DIR) + '/' + "hakonepa.png");

    KisDefaultBoundsBaseSP bounds = new TestUtil::TestingTimedDefaultBounds(qimage.rect());

    KisPaintDeviceSP dev = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
    dev->setDefaultBounds(bounds);
    dev->convertFromQImage(qimage, 0, 0, 0);

    KisConvolutionKernelSP kernel = new KisConvolutionKernel(3, 3, 0, 0);
    kernel->data()(0) = 0;
    kernel->data()(1) = 0;
    kernel->data()(2) = 0;
    kernel->data()(3) = 0;
    kernel->data()(4) = 1;
    kernel->data()(5) = 0;
    kernel->data()(6) = 0;
    kernel->data()(7) = 0;
    kernel->data()(8) = 0;
    KisConvolutionPainter gc(dev);
    gc.beginTransaction();
    gc.applyMatrix(kernel, dev, QPoint(0, 0), QPoint(0, 0), QSize(qimage.width(), qimage.height()));
    gc.deleteTransaction();

    QImage resultImage = dev->convertToQImage(0, 0, 0, qimage.width(), qimage.height());

    QPoint errpoint;
    if (!TestUtil::compareQImages(errpoint, qimage, resultImage)) {
        resultImage.save("identity_convolution.png");
        QFAIL(QString("Identity kernel did change image, first different pixel: %1,%2 ").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
    }
}

void KisConvolutionPainterTest::testSymmConvolution()
{
    qreal offset = 0.0;
    qreal factor = 1.0;
    Eigen::Matrix<qreal, 3, 3> filter = initSymmFilter(offset, factor);

    QRect imageRect;
    int pixelSize = 0;
    QByteArray initialData;
    KisPaintDeviceSP dev = initAsymTestDevice(imageRect, pixelSize, initialData);

    KisDefaultBoundsBaseSP bounds = new TestUtil::TestingTimedDefaultBounds(dev->exactBounds());
    dev->setDefaultBounds(bounds);

    KisConvolutionKernelSP kernel =
        KisConvolutionKernel::fromMatrix(filter, offset, factor);
    KisConvolutionPainter gc(dev);
    gc.beginTransaction();

    QRect filterRect = imageRect.adjusted(1,1,-1,-1);
    gc.applyMatrix(kernel, dev, filterRect.topLeft(), filterRect.topLeft(),
                   filterRect.size());
    gc.deleteTransaction();

    QByteArray resultData(initialData.size(), 0);
    dev->readBytes((quint8*)resultData.data(), imageRect);

    QCOMPARE(resultData, initialData);
}

void KisConvolutionPainterTest::testAsymmConvolutionImp(QBitArray channelFlags)
{
    qreal offset = 0.0;
    qreal factor = 1.0;
    Eigen::Matrix<qreal, 3, 3> filter = initAsymmFilter(offset, factor);

    QRect imageRect;
    int pixelSize = -1;
    QByteArray initialData;
    KisPaintDeviceSP dev = initAsymTestDevice(imageRect, pixelSize, initialData);

    KisDefaultBoundsBaseSP bounds = new TestUtil::TestingTimedDefaultBounds(dev->exactBounds());
    dev->setDefaultBounds(bounds);

    KisConvolutionKernelSP kernel =
        KisConvolutionKernel::fromMatrix(filter, offset, factor);
    KisConvolutionPainter gc(dev);
    gc.beginTransaction();
    gc.setChannelFlags(channelFlags);

    QRect filterRect = imageRect.adjusted(1,1,-1,-1);
    gc.applyMatrix(kernel, dev, filterRect.topLeft(), filterRect.topLeft(),
                   filterRect.size());
    gc.deleteTransaction();


    QByteArray resultData(initialData.size(), 0);
    dev->readBytes((quint8*)resultData.data(), imageRect);

    QRect filteredRect = imageRect.adjusted(1, 1, -1, -1);

    quint8 *srcPtr = (quint8*) initialData.data();
    quint8 *resPtr = (quint8*) resultData.data();

    for(int row = 0; row < imageRect.height(); row++) {
        for(int col = 0; col < imageRect.width(); col++) {

            bool isFiltered = filteredRect.contains(col, row);

            int pixelValue = 8 + row * imageRect.width() + col;
            KoColor filteredPixel(QColor(pixelValue, pixelValue, pixelValue, 255), dev->colorSpace());

            KoColor resultPixel(dev->colorSpace());
            for(int j = 0; j < pixelSize; j++) {
                resultPixel.data()[j] = isFiltered && channelFlags[j] ?
                    filteredPixel.data()[j] : srcPtr[j];
            }

            if(memcmp(resPtr, resultPixel.data(), pixelSize)) {
                printPixel("Actual:  ", pixelSize, resPtr);
                printPixel("Expected:", pixelSize, resultPixel.data());
                QFAIL("Failed to filter area");
            }

            srcPtr += pixelSize;
            resPtr += pixelSize;
        }
    }
}

void KisConvolutionPainterTest::testAsymmAllChannels()
{
    QBitArray channelFlags =
        KoColorSpaceRegistry::instance()->rgb8()->channelFlags(true, true);
    testAsymmConvolutionImp(channelFlags);
}

void KisConvolutionPainterTest::testAsymmSkipRed()
{
    QBitArray channelFlags =
        KoColorSpaceRegistry::instance()->rgb8()->channelFlags(true, true);
    channelFlags[2] = false;

    testAsymmConvolutionImp(channelFlags);
}

void KisConvolutionPainterTest::testAsymmSkipGreen()
{
    QBitArray channelFlags =
        KoColorSpaceRegistry::instance()->rgb8()->channelFlags(true, true);
    channelFlags[1] = false;

    testAsymmConvolutionImp(channelFlags);
}

void KisConvolutionPainterTest::testAsymmSkipBlue()
{
    QBitArray channelFlags =
        KoColorSpaceRegistry::instance()->rgb8()->channelFlags(true, true);
    channelFlags[0] = false;

    testAsymmConvolutionImp(channelFlags);
}

void KisConvolutionPainterTest::testAsymmSkipAlpha()
{
    QBitArray channelFlags =
        KoColorSpaceRegistry::instance()->rgb8()->channelFlags(true, true);
    channelFlags[3] = false;

    testAsymmConvolutionImp(channelFlags);
}


// #include <valgrind/callgrind.h>
void KisConvolutionPainterTest::benchmarkConvolution()
{
    QImage referenceImage(QString(FILES_DATA_DIR) + '/' + "hakonepa.png");
    QRect imageRect(QPoint(), referenceImage.size());

    KisPaintDeviceSP dev = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
    dev->convertFromQImage(referenceImage, 0, 0, 0);

    KisDefaultBoundsBaseSP bounds = new TestUtil::TestingTimedDefaultBounds(referenceImage.rect());
    dev->setDefaultBounds(bounds);


    int diameter = 1;

    for (int i = 0; i < 4; i++) {

        KisCircleMaskGenerator* kas = new KisCircleMaskGenerator(diameter, 1.0, 5, 5, 2, false);
        KisConvolutionKernelSP kernel = KisConvolutionKernel::fromMaskGenerator(kas);

        KisConvolutionPainter gc(dev);

        QElapsedTimer timer; timer.start();

        // CALLGRIND_START_INSTRUMENTATION;

        gc.beginTransaction();
        gc.applyMatrix(kernel, dev, imageRect.topLeft(), imageRect.topLeft(),
                       imageRect.size());
        gc.deleteTransaction();

        // CALLGRIND_STOP_INSTRUMENTATION;

        dbgKrita << "Diameter:" << diameter << "time:" << timer.elapsed();

        if(diameter < 4) {
            diameter += 2;
        } else {
            diameter += 8;
        }
    }
}

void KisConvolutionPainterTest::testGaussianBase(KisPaintDeviceSP dev, bool useFftw, const QString &prefix)
{
   QBitArray channelFlags =
       KoColorSpaceRegistry::instance()->rgb8()->channelFlags(true, true);

   KisPainter gc(dev);


   qreal horizontalRadius = 5, verticalRadius = 5;

   for(int i = 0; i < 3 ; i++, horizontalRadius+=5, verticalRadius+=5)
   {
       QElapsedTimer timer;
       timer.start();

       gc.beginTransaction();

       if (( horizontalRadius > 0 ) && ( verticalRadius > 0 )) {
           KisPaintDeviceSP interm = new KisPaintDevice(dev->colorSpace());
           interm->setDefaultBounds(dev->defaultBounds());

           KisConvolutionKernelSP kernelHoriz = KisGaussianKernel::createHorizontalKernel(horizontalRadius);
           KisConvolutionKernelSP kernelVertical = KisGaussianKernel::createVerticalKernel(verticalRadius);

           const QRect applyRect = dev->exactBounds();

           KisConvolutionPainter::EnginePreference enginePreference =
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


void KisConvolutionPainterTest::testGaussian(bool useFftw)
{
    QImage referenceImage(TestUtil::fetchDataFileLazy("kritaTransparent.png"));
    KisPaintDeviceSP dev = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
    dev->convertFromQImage(referenceImage, 0, 0, 0);

    // TODO: fix the bounding rect to the rect of the reference image
    KisDefaultBoundsBaseSP bounds = new TestUtil::TestingTimedDefaultBounds(dev->exactBounds());
    dev->setDefaultBounds(bounds);

    testGaussianBase(dev, useFftw, "");
}

void KisConvolutionPainterTest::testGaussianSpatial()
{
    testGaussian(false);
}

void KisConvolutionPainterTest::testGaussianFFTW()
{
    testGaussian(true);
}

void KisConvolutionPainterTest::testGaussianSmall(bool useFftw)
{
    KisPaintDeviceSP dev = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());

    KoColor c(Qt::yellow, dev->colorSpace());

    for (int i = 0; i < 50; i++) {
        quint8 baseOpacity = 75;
        KoColor c(Qt::magenta, dev->colorSpace());

        for (int j = 0; j <= 6; j++) {
            c.setOpacity(static_cast<quint8>(baseOpacity + 30 * j));
            dev->setPixel(i + j, i, c);
        }
    }

    KisDefaultBoundsBaseSP bounds = new TestUtil::TestingTimedDefaultBounds(dev->exactBounds());
    dev->setDefaultBounds(bounds);

    testGaussianBase(dev, useFftw, "reduced");
}

void KisConvolutionPainterTest::testGaussianSmallSpatial()
{
    testGaussianSmall(false);
}

void KisConvolutionPainterTest::testGaussianSmallFFTW()
{
    testGaussianSmall(true);
}

void KisConvolutionPainterTest::testGaussianDetails(bool useFftw)
{
    QImage referenceImage(TestUtil::fetchDataFileLazy("resolution_test.png"));
    KisPaintDeviceSP dev = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
    dev->convertFromQImage(referenceImage, 0, 0, 0);

    KisDefaultBoundsBaseSP bounds = new TestUtil::TestingTimedDefaultBounds(dev->exactBounds());
    dev->setDefaultBounds(bounds);

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

#include "kis_transaction.h"

void KisConvolutionPainterTest::testDilate()
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->alpha8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);

    const QRect imageRect(0,0,256,256);
    dev->fill(QRect(50,50,100,20), KoColor(Qt::white, cs));
    dev->fill(QRect(150,50,20,100), KoColor(Qt::white, cs));

    KisDefaultBoundsBaseSP bounds = new TestUtil::TestingTimedDefaultBounds(dev->exactBounds());
    dev->setDefaultBounds(bounds);

    TestUtil::checkQImage(dev->convertToQImage(0, imageRect), "convolution_painter_test", "dilate", "initial");

    KisGaussianKernel::applyDilate(dev, imageRect, 10, QBitArray(), 0);

    TestUtil::checkQImage(dev->convertToQImage(0, imageRect), "convolution_painter_test", "dilate", "dilate10");
}

void KisConvolutionPainterTest::testErode()
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->alpha8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);

    const QRect imageRect(0,0,256,256);
    dev->fill(QRect(50,50,100,20), KoColor(Qt::white, cs));
    dev->fill(QRect(150,50,20,100), KoColor(Qt::white, cs));

    KisDefaultBoundsBaseSP bounds = new TestUtil::TestingTimedDefaultBounds(dev->exactBounds());
    dev->setDefaultBounds(bounds);

    TestUtil::checkQImage(dev->convertToQImage(0, imageRect), "convolution_painter_test", "dilate", "initial");

    KisGaussianKernel::applyErodeU8(dev, imageRect, 5, QBitArray(), 0);

    TestUtil::checkQImage(dev->convertToQImage(0, imageRect), "convolution_painter_test", "dilate", "erode5");
}

#include "kis_edge_detection_kernel.h"

void KisConvolutionPainterTest::testNormalMap(KisPaintDeviceSP dev, bool useFftw, const QString &prefix)
{
   QBitArray channelFlags =
       KoColorSpaceRegistry::instance()->rgb8()->channelFlags(true, true);

   KisPainter gc(dev);


   qreal horizontalRadius = 0.5, verticalRadius = 0.5;

   for(int i = 0; i < 3 ; i++, horizontalRadius+=0.5, verticalRadius+=0.5)
   {
       QElapsedTimer timer;
       timer.start();

       gc.beginTransaction();

       if (( horizontalRadius > 0 ) && ( verticalRadius > 0 )) {

           const QRect applyRect = dev->exactBounds();

           //KisGaussianKernel::applyLoG(dev, applyRect, horizontalRadius, 1.0, channelFlags, 0);

           QVector<int> channelOrder(3);
           QVector<bool> channelFlip(3);
           channelFlip.fill(false);

           channelOrder[0] = 0;
           channelOrder[1] = 1;
           channelOrder[2] = 2;

           KisEdgeDetectionKernel::convertToNormalMap(dev, applyRect,
                                                      5.67, 5.67,
                                                      KisEdgeDetectionKernel::Simple,
                                                      0,
                                                      channelOrder,
                                                      channelFlip,
                                                      channelFlags,
                                                      0,
                                                      useFftw);

           QImage result = dev->convertToQImage(0, applyRect.x(), applyRect.y(), applyRect.width(), applyRect.height());

           QString engine = useFftw ? "fftw" : "spatial";
           QString testCaseName = QString("test_%1_%2_%3.png").arg(horizontalRadius).arg(verticalRadius).arg(engine);

           QVERIFY(TestUtil::checkQImage(result,
                                         "convolution_painter_test",
                                         QString("normalmap_") + prefix,
                                         testCaseName, 1));

           gc.revertTransaction();
       }
       qDebug() << "Elapsed time:" << timer.elapsed() << "ms";
    }
}

void KisConvolutionPainterTest::testNormalMap(bool useFftw)
{
    KisPaintDeviceSP dev = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());

    KoColor c(Qt::yellow, dev->colorSpace());

    for (int i = 0; i < 50; i++) {
        quint8 baseOpacity = 75;
        KoColor c(Qt::magenta, dev->colorSpace());

        for (int j = 0; j <= 6; j++) {
            c.setOpacity(static_cast<quint8>(baseOpacity + 30 * j));
            dev->setPixel(i + j, i, c);
        }
    }

    KisDefaultBoundsBaseSP bounds = new TestUtil::TestingTimedDefaultBounds(dev->exactBounds());
    dev->setDefaultBounds(bounds);

    testNormalMap(dev, useFftw, "reduced");
}

void KisConvolutionPainterTest::testNormalMapSpatial()
{
    testNormalMap(false);
}

void KisConvolutionPainterTest::testNormalMapFFTW()
{
    testNormalMap(true);
}

KISTEST_MAIN(KisConvolutionPainterTest)
