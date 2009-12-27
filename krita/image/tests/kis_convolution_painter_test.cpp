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

#include "kis_convolution_painter_test.h"

#include <qtest_kde.h>

#include <QBitArray>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include "kis_types.h"
#include "kis_paint_device.h"
#include "kis_convolution_painter.h"
#include "kis_convolution_kernel.h"
#include "kis_paint_device.h"
#include <kis_mask_generator.h>
#include "testutil.h"

void KisConvolutionPainterTest::testCreation()
{
    KisConvolutionPainter test;
}

void KisConvolutionPainterTest::testIdentityConvolution()
{
    QImage qimage(QString(FILES_DATA_DIR) + QDir::separator() + "hakonepa.png");

    KisPaintDeviceSP dev = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
    dev->convertFromQImage(qimage, "", 0, 0);

    KisConvolutionKernelSP kernel = new KisConvolutionKernel(3, 3, 0, 0);
    kernel->data()[0] = 0;
    kernel->data()[1] = 0;
    kernel->data()[2] = 0;
    kernel->data()[3] = 0;
    kernel->data()[4] = 1;
    kernel->data()[5] = 0;
    kernel->data()[6] = 0;
    kernel->data()[7] = 0;
    kernel->data()[8] = 0;
    KisConvolutionPainter gc(dev);
    gc.applyMatrix(kernel, dev, QPoint(0, 0), QPoint(0, 0), QSize(qimage.width(), qimage.height()));

    QPoint errpoint;
    if (!TestUtil::compareQImages(errpoint, qimage, dev->convertToQImage(0, 0, 0, qimage.width(), qimage.height()))) {
        dev->convertToQImage(0, 0, 0, qimage.width(), qimage.height()).save("identity_convolution.png");
        QFAIL(QString("Identity kernel did change image, first different pixel: %1,%2 ").arg(errpoint.x()).arg(errpoint.y()).toAscii());
    }
}

void KisConvolutionPainterTest::testIdentityConvolutionOnColorChannels()
{
    QImage qimage(QString(FILES_DATA_DIR) + QDir::separator() + "hakonepa.png");

    KisPaintDeviceSP dev = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
    dev->convertFromQImage(qimage, "", 0, 0);


    KisConvolutionKernelSP kernel = new KisConvolutionKernel(3, 3, 0, 0);
    kernel->data()[0] = 0;
    kernel->data()[1] = 0;
    kernel->data()[2] = 0;
    kernel->data()[3] = 0;
    kernel->data()[4] = 1;
    kernel->data()[5] = 0;
    kernel->data()[6] = 0;
    kernel->data()[7] = 0;
    kernel->data()[8] = 0;
    KisConvolutionPainter gc(dev);

    gc.setChannelFlags(dev->colorSpace()->channelFlags());
    gc.applyMatrix(kernel, dev, QPoint(0, 0), QPoint(0, 0), QSize(qimage.width(), qimage.height()));

    QPoint errpoint;
    if (!TestUtil::compareQImages(errpoint, qimage, dev->convertToQImage(0, 0, 0, qimage.width(), qimage.height()))) {
        dev->convertToQImage(0, 0, 0, qimage.width(), qimage.height()).save("identity_convolution_identity_on_colorchannels.png");
        QFAIL(QString("Identity kernel did change image, first different pixel: %1,%2 ").arg(errpoint.x()).arg(errpoint.y()).toAscii());
    }
}

void KisConvolutionPainterTest::testMaskConvolution()
{
    QImage qimage(QString(FILES_DATA_DIR) + QDir::separator() + "hakonepa.png");
    QImage result(QString(FILES_DATA_DIR) + QDir::separator() + "mask_conv.png");
    KisPaintDeviceSP dev = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
    dev->convertFromQImage(qimage, "", 0, 0);

    KisCircleMaskGenerator* kas = new KisCircleMaskGenerator(11, 11, 5, 5);
    KisConvolutionKernelSP kernel = KisConvolutionKernel::fromMaskGenerator(kas);

    KisConvolutionPainter gc(dev);
    gc.applyMatrix(kernel, dev, QPoint(0, 0), QPoint(0, 0), QSize(qimage.width(), qimage.height()));

    QPoint errpoint;
    if (!TestUtil::compareQImages(errpoint, result, dev->convertToQImage(0, 0, 0, qimage.width(), qimage.height()))) {
        dev->convertToQImage(0, 0, 0, qimage.width(), qimage.height()).save("mask_conv.png");
        QFAIL(QString("Mask kernel failed, first different pixel: %1,%2 ").arg(errpoint.x()).arg(errpoint.y()).toAscii());
    }
}

void KisConvolutionPainterTest::testMaskConvolutionOnColorChannels()
{
    QImage qimage(QString(FILES_DATA_DIR) + QDir::separator() + "hakonepa.png");
    QImage result(QString(FILES_DATA_DIR) + QDir::separator() + "mask_conv_channelflags.png");
    KisPaintDeviceSP dev = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
    dev->convertFromQImage(qimage, "", 0, 0);

    KisCircleMaskGenerator* kas = new KisCircleMaskGenerator(11, 11, 5, 5);
    KisConvolutionKernelSP kernel = KisConvolutionKernel::fromMaskGenerator(kas);

    KisConvolutionPainter gc(dev);
    gc.setChannelFlags(dev->colorSpace()->channelFlags());
    gc.applyMatrix(kernel, dev, QPoint(0, 0), QPoint(0, 0), QSize(qimage.width(), qimage.height()));

    QPoint errpoint;
    if (!TestUtil::compareQImages(errpoint, result, dev->convertToQImage(0, 0, 0, qimage.width(), qimage.height()))) {
        dev->convertToQImage(0, 0, 0, qimage.width(), qimage.height()).save("mask_conv_channelflags.png");
        QFAIL(QString("Mask on color channels failed, first different pixel: %1,%2 ").arg(errpoint.x()).arg(errpoint.y()).toAscii());
    }
}

void KisConvolutionPainterTest::testMaskConvolutionOnRedChannel()
{
    QImage qimage(QString(FILES_DATA_DIR) + QDir::separator() + "hakonepa.png");
    QImage result(QString(FILES_DATA_DIR) + QDir::separator() + "mask_conv_red.png");
    KisPaintDeviceSP dev = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
    dev->convertFromQImage(qimage, "", 0, 0);

    KisCircleMaskGenerator* kas = new KisCircleMaskGenerator(11, 11, 5, 5);
    KisConvolutionKernelSP kernel = KisConvolutionKernel::fromMaskGenerator(kas);

    KisConvolutionPainter gc(dev);
    QBitArray channelFlags = dev->colorSpace()->channelFlags();
    channelFlags.setBit(0, false);
    channelFlags.setBit(1, false);
    channelFlags.setBit(2, true);
    channelFlags.setBit(3, false);

    gc.setChannelFlags(channelFlags);
    gc.applyMatrix(kernel, dev, QPoint(0, 0), QPoint(0, 0), QSize(qimage.width(), qimage.height()));

    QPoint errpoint;
    if (!TestUtil::compareQImages(errpoint, result, dev->convertToQImage(0, 0, 0, qimage.width(), qimage.height()))) {
        dev->convertToQImage(0, 0, 0, qimage.width(), qimage.height()).save("mask_conv_red.png");
        QFAIL(QString("Mask on red channel, first different pixel: %1,%2 ").arg(errpoint.x()).arg(errpoint.y()).toAscii());
    }
}
QTEST_KDEMAIN(KisConvolutionPainterTest, GUI)
#include "kis_convolution_painter_test.moc"
