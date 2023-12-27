/*
 *  SPDX-FileCopyrightText: 2013 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <kis_paint_device.h>
#include <kis_paint_layer.h>
#include <KoColorSpaceRegistry.h>
#include <kis_image.h>
#include <KoColorSpace.h>
#include <KoResourcePaths.h>
#include <KoColorModelStandardIds.h>
#include <KoColor.h>

#include <simpletest.h>
#include <QImage>

#include <kis_qmic_simple_convertor.h>
#include <kis_group_layer.h>
#include <kis_painter.h>
#include <kis_selection.h>
#include <commands/kis_set_global_selection_command.h>
#include <kis_processing_applicator.h>
#include <testutil.h>

#include "../kis_qmic_interface.h"
#include "kis_qmic_tests.h"

#ifndef FILES_DATA_DIR
#error "FILES_DATA_DIR not set. A directory with the data used for testing the importing of files in krita"
#endif

void KisQmicTests::initTestCase()
{
    m_qimage = QImage(QString(FILES_DATA_DIR) + "/" + "poster_rodents_bunnysize.jpg");
    m_qimage.convertTo(QImage::Format_ARGB32);
}

void KisQmicTests::cleanupTestCase()
{
}

void KisQmicTests::testConvertGrayScaleQmic()
{
    KisPaintDeviceSP resultDev = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
    KisPaintDeviceSP resultDevFast = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());

    KisQMicImage qmicImage({}, m_qimage.width(), m_qimage.height(), 1);

    KisQmicSimpleConvertor::convertFromQImage(m_qimage, qmicImage, 1.0);
    KisQmicSimpleConvertor::convertFromGmicImage(qmicImage, resultDev, 1.0);
    KisQmicSimpleConvertor::convertFromGmicFast(qmicImage, resultDevFast, 1.0);

    QImage slowQImage = resultDev->convertToQImage(nullptr,
                                                   0,
                                                   0,
                                                   qmicImage.m_width,
                                                   qmicImage.m_height);
    QImage fastQImage = resultDevFast->convertToQImage(nullptr,
                                                       0,
                                                       0,
                                                       qmicImage.m_width,
                                                       qmicImage.m_height);

    QPoint errpoint;
    if (!TestUtil::compareQImages(errpoint, slowQImage, fastQImage)) {
        QFAIL(QString("Slow method produces different result then fast to convert qmic grayscale pixel format, first different pixel: %1,%2 ").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
        slowQImage.save("grayscale.bmp");
        fastQImage.save("grayscale_fast.bmp");
    }
}

void KisQmicTests::testConvertGrayScaleAlphaQmic()
{
    KisPaintDeviceSP resultDev = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
    KisPaintDeviceSP resultDevFast = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());

    KisQMicImage qmicImage({}, m_qimage.width(), m_qimage.height(), 2);

    KisQmicSimpleConvertor::convertFromQImage(m_qimage, qmicImage, 1.0);
    KisQmicSimpleConvertor::convertFromGmicImage(qmicImage, resultDev, 1.0);
    KisQmicSimpleConvertor::convertFromGmicFast(qmicImage, resultDevFast, 1.0);

    QImage slowQImage = resultDev->convertToQImage(nullptr,
                                                   0,
                                                   0,
                                                   qmicImage.m_width,
                                                   qmicImage.m_height);
    QImage fastQImage = resultDevFast->convertToQImage(nullptr,
                                                       0,
                                                       0,
                                                       qmicImage.m_width,
                                                       qmicImage.m_height);

    QPoint errpoint;
    if (!TestUtil::compareQImages(errpoint, slowQImage, fastQImage)) {
        QFAIL(QString("Slow method produces different result then fast to convert qmic grayscale pixel format, first different pixel: %1,%2 ").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
        slowQImage.save("grayscale.bmp");
        fastQImage.save("grayscale_fast.bmp");
    }
}

void KisQmicTests::testConvertRGBqmic()
{
    KisPaintDeviceSP resultDev = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
    KisPaintDeviceSP resultDevFast = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());

    KisQMicImage qmicImage({}, m_qimage.width(), m_qimage.height(), 3);

    KisQmicSimpleConvertor::convertFromQImage(m_qimage, qmicImage, 1.0);
    KisQmicSimpleConvertor::convertFromGmicImage(qmicImage, resultDev, 1.0);
    KisQmicSimpleConvertor::convertFromGmicFast(qmicImage, resultDevFast, 1.0);

    QImage slowQImage = resultDev->convertToQImage(nullptr,
                                                   0,
                                                   0,
                                                   qmicImage.m_width,
                                                   qmicImage.m_height);
    QPoint errpoint;

    if (!TestUtil::compareQImages(errpoint, slowQImage, m_qimage)) {
        QFAIL(QString("Slow method failed to convert qmic RGB pixel format, first different pixel: %1,%2 ").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
        slowQImage.save("RGB.bmp");
    }

    QImage fastQImage = resultDevFast->convertToQImage(nullptr,
                                                       0,
                                                       0,
                                                       qmicImage.m_width,
                                                       qmicImage.m_height);

    if (!TestUtil::compareQImages(errpoint,fastQImage,m_qimage)) {
        QFAIL(QString("Fast method failed to convert qmic RGB pixel format, first different pixel: %1,%2 ").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
        fastQImage.save("RGB_fast.bmp");
    }
}


void KisQmicTests::testConvertRGBAqmic()
{
    KisPaintDeviceSP resultDev = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
    KisPaintDeviceSP resultDevFast = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());

    KisQMicImage qmicImage({}, m_qimage.width(), m_qimage.height(), 4);

    KisQmicSimpleConvertor::convertFromQImage(m_qimage, qmicImage, 1.0);
    KisQmicSimpleConvertor::convertFromGmicImage(qmicImage, resultDev, 1.0);
    KisQmicSimpleConvertor::convertFromGmicFast(qmicImage, resultDevFast, 1.0);

    QImage slowQImage = resultDev->convertToQImage(nullptr,
                                                   0,
                                                   0,
                                                   qmicImage.m_width,
                                                   qmicImage.m_height);
    QPoint errpoint;
    if (!TestUtil::compareQImages(errpoint,slowQImage,m_qimage)) {
        QFAIL(QString("Slow method failed to convert qmic RGBA pixel format, first different pixel: %1,%2 ").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
        slowQImage.save("RGBA.bmp");
    }

    QImage fastQImage = resultDevFast->convertToQImage(nullptr,
                                                       0,
                                                       0,
                                                       qmicImage.m_width,
                                                       qmicImage.m_height);

    if (!TestUtil::compareQImages(errpoint,fastQImage,m_qimage)) {
        QFAIL(QString("Fast method failed to convert qmic RGBA pixel format, first different pixel: %1,%2 ").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
        fastQImage.save("RGBA_fast.bmp");
    }
}

void KisQmicTests::testConvertToGmic()
{
    KisPaintDeviceSP srcDev = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
    srcDev->convertFromQImage(m_qimage, nullptr);

    KisQMicImage qmicImage({}, m_qimage.width(), m_qimage.height(), 4);

    KisQmicSimpleConvertor::convertToGmicImageFast(srcDev,
                                                   qmicImage,
                                                   srcDev->exactBounds());

    QPoint errpoint;
    QImage resultQImage = KisQmicSimpleConvertor::convertToQImage(qmicImage);
    if (!TestUtil::compareQImages(errpoint, resultQImage, m_qimage)) {
        QFAIL(QString("Failed to convert qmic RGBA pixel format to QImage, first different pixel: %1,%2 ").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
        resultQImage.save("testConvertToGmic_qimage.bmp");
    }

    KisPaintDeviceSP resultDevFast = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
    KisQmicSimpleConvertor::convertFromGmicFast(qmicImage, resultDevFast, 255.0f);
    QImage fastQImage = resultDevFast->convertToQImage(nullptr,
                                                       0,
                                                       0,
                                                       qmicImage.m_width,
                                                       qmicImage.m_height);

    if (!TestUtil::compareQImages(errpoint, fastQImage, m_qimage)) {
        QFAIL(QString("Fast method failed to convert qmic RGBA pixel format, first different pixel: %1,%2 ").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
        fastQImage.save("testConvertToGmic_fast.bmp");
    }

    KisPaintDeviceSP resultDev = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
    KisQmicSimpleConvertor::convertFromGmicImage(qmicImage, resultDev, 255.0f);
    QImage slowQImage = resultDev->convertToQImage(nullptr,
                                                   0,
                                                   0,
                                                   qmicImage.m_width,
                                                   qmicImage.m_height);
    if (!TestUtil::compareQImages(errpoint, slowQImage, m_qimage)) {
        QFAIL(QString("Slow method failed to convert qmic RGBA pixel format, first different pixel: %1,%2 ").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
        slowQImage.save("testConvertToGmic_slow.bmp");
    }


}


SIMPLE_TEST_MAIN(KisQmicTests)

