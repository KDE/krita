/*
 *  Copyright (c) 2013 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */




#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorModelStandardIds.h>
#include <KoColor.h>

#include <qtest_kde.h>

#include <QImage>
#include <QColor>

#include "kis_gmic_benchmarks.h"
#include "kis_gmic_simple_convertor.h"
// #define SAVE_OUTPUT

void KisGmicBenchmarks::initTestCase()
{
    // qimage
    m_darkOrange = QColor(255,127,0,127);

    m_qImage = QImage(QString(FILES_DATA_DIR)+"/"+"poster_rodents_bunnysize.jpg");
    m_qImage = m_qImage.convertToFormat(QImage::Format_ARGB32);


    //m_qImage = QImage(4096, 4096, QImage::Format_ARGB32);
    //m_qImage.fill(m_darkOrange);

    m_rect = QRect(0,0, m_qImage.width(), m_qImage.height());

    // gmic image
    m_gmicImage.assign(m_rect.width(),m_rect.height(), 1,4);

    // kisPaintDevice
    const KoColorSpace * colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    m_device = new KisPaintDevice(colorSpace);
    m_device->convertFromQImage(m_qImage, 0);
}

void KisGmicBenchmarks::cleanupTestCase()
{

}


void KisGmicBenchmarks::testQImageConversion()
{
    gmic_image<float> gmicImage;
    gmicImage.assign(m_qImage.width(),m_qImage.height(), 1,4);

    // convert QImage > gmic layer -> QImage
    QImage result;
    QBENCHMARK
    {
            KisGmicSimpleConvertor::convertFromQImage(m_qImage, gmicImage, 1.0);
            result = KisGmicSimpleConvertor::convertToQImage(gmicImage, 1.0);
    }

    // TODO: compare images
#ifdef SAVE_OUTPUT
    m_qImage.save("original.bmp");
    result.save("original-converted.bmp");
#endif
}


void KisGmicBenchmarks::testKisPaintDeviceConversion()
{
    gmic_image<float> gmicImage;
    gmicImage.assign(m_qImage.width(),m_qImage.height(), 1,4);

    // benchmark rgba2rgba
    KisPaintDeviceSP result = new KisPaintDevice(m_device->colorSpace());
    result->fill(QRect(0,0,m_qImage.width(),m_qImage.height()), KoColor(m_darkOrange, m_device->colorSpace()));

    QBENCHMARK
    {
        KisGmicSimpleConvertor::convertToGmicImage(m_device, gmicImage, m_rect);
        KisGmicSimpleConvertor::convertFromGmicImage(gmicImage, result, 1.0);
    }

    #ifdef SAVE_OUTPUT
        QImage qResult = result->convertToQImage(0, 0,0, m_qImage.width(),m_qImage.height());
        qResult.save("Device-Gmic-Device-new.bmp");
    #endif
}


void KisGmicBenchmarks::testConvertToGmic()
{
    gmic_image<float> gmicImage;
    gmicImage.assign(m_qImage.width(),m_qImage.height(), 1,4);

    QBENCHMARK
    {
        KisGmicSimpleConvertor::convertToGmicImage(m_device, gmicImage, m_rect);
    }

    #ifdef SAVE_OUTPUT
        QImage qResult = KisGmicSimpleConvertor::convertToQImage(gmicImage, 1.0);
        qResult.save("001_testConvertToGmic.bmp");
    #endif
}

void KisGmicBenchmarks::testConvertFromGmic()
{
    gmic_image<float> gmicImage;
    gmicImage.assign(m_qImage.width(),m_qImage.height(), 1,4);

    KisGmicSimpleConvertor::convertFromQImage(m_qImage, gmicImage, 1.0);

    KisPaintDeviceSP result = new KisPaintDevice(m_device->colorSpace());
    result->fill(QRect(0,0,m_qImage.width(),m_qImage.height()), KoColor(m_darkOrange, m_device->colorSpace()));

    QBENCHMARK
    {
        KisGmicSimpleConvertor::convertFromGmicImage(gmicImage, result, 1.0);
    }

    #ifdef SAVE_OUTPUT
        QImage qResult = result->convertToQImage(0, 0,0, m_qImage.width(),m_qImage.height());
        qResult.save("002_testConvertFromGmic.bmp");
    #endif
}


void KisGmicBenchmarks::testConvertToGmicFast()
{
    gmic_image<float> gmicImage;
    gmicImage.assign(m_qImage.width(),m_qImage.height(), 1,4);
    memset(gmicImage._data, 0, m_qImage.width() * m_qImage.height() * sizeof(float));

    QBENCHMARK
    {
        KisGmicSimpleConvertor::convertToGmicImageFast(m_device, gmicImage, m_rect);
    }

    #ifdef SAVE_OUTPUT
        QImage qResult = KisGmicSimpleConvertor::convertToQImage(gmicImage, 1.0);
        qResult.save("003_testConvertToGmic_fast.bmp");
    #endif
}


void KisGmicBenchmarks::testConvertFromGmicFast()
{
    gmic_image<float> gmicImage;
    gmicImage.assign(m_qImage.width(),m_qImage.height(), 1,4);

    KisGmicSimpleConvertor::convertFromQImage(m_qImage, gmicImage, 1.0);

    KisPaintDeviceSP result = new KisPaintDevice(m_device->colorSpace());
    result->fill(QRect(0,0,m_qImage.width(),m_qImage.height()), KoColor(m_darkOrange, m_device->colorSpace()));

    QBENCHMARK
    {
        KisGmicSimpleConvertor::convertFromGmicFast(gmicImage, result, 1.0);
    }

    #ifdef SAVE_OUTPUT
        QImage qResult = result->convertToQImage(0, 0,0, m_qImage.width(),m_qImage.height());
        qResult.save("002_testConvertFromGmic_fast.bmp");
    #endif
}




void KisGmicBenchmarks::testConversion()
{
    const KoColorSpace *rgbaFloat32bitcolorSpace = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(),
                                                                                                Float32BitsColorDepthID.id(),
                                                                                                KoColorSpaceRegistry::instance()->rgb8()->profile());

    Q_CHECK_PTR(rgbaFloat32bitcolorSpace);
    QCOMPARE(m_device->colorSpace()->name(), KoColorSpaceRegistry::instance()->rgb8()->name());
    QBENCHMARK
    {
        m_device->convertTo(rgbaFloat32bitcolorSpace);
        m_device->convertTo(KoColorSpaceRegistry::instance()->rgb8());
    }
}

QTEST_KDEMAIN(KisGmicBenchmarks, GUI)

