/*
 * Copyright (c) 2013 Lukáš Tvrdý <lukast.dev@gmail.com
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

#include <QRect>

#include <kis_debug.h>
#include <kis_random_accessor_ng.h>

#include <kis_gmic_simple_convertor.h>

#include <KoColorSpaceRegistry.h>
#include <KoColorSpace.h>
#include <KoColorModelStandardIds.h>
#include <KoColorSpaceTraits.h>

using namespace cimg_library;

// gmic assumes float rgba in 0.0 - 255.0, thus default value
KisGmicSimpleConvertor::KisGmicSimpleConvertor() : m_multiplier(255.0)
{

}


KisGmicSimpleConvertor::~KisGmicSimpleConvertor()
{
}

void KisGmicSimpleConvertor::convertToGmicImage(KisPaintDeviceSP dev, CImg< float >& gmicImage, QRect rc)
{
    if (rc.isEmpty())
    {
        rc = dev->exactBounds();
    }

    const KoColorSpace *rgbaFloat32bitcolorSpace = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(),
                                                                                                Float32BitsColorDepthID.id(),
                                                                                                KoColorSpaceRegistry::instance()->rgb8()->profile());

    Q_CHECK_PTR(rgbaFloat32bitcolorSpace);
    dev->convertTo(rgbaFloat32bitcolorSpace);

    Q_ASSERT(gmicImage._spectrum == 4); // rgba

    int greenOffset = gmicImage._width * gmicImage._height;
    int blueOffset = greenOffset * 2;
    int alphaOffset = greenOffset * 3;


    KisRandomConstAccessorSP it = dev->createRandomConstAccessorNG(0,0);
    int pos;
    for (int y = 0; y < rc.height(); y++)
    {
        for (int x = 0; x < rc.width(); x++)
        {
            pos = y * gmicImage._width + x;
            it->moveTo(x, y);
            const quint8 * floatRGBApixel = it->rawDataConst(); // we are interpreting pixels here

            memcpy(gmicImage._data + pos                  ,floatRGBApixel    , 4);
            memcpy(gmicImage._data + pos + greenOffset    ,floatRGBApixel + 4, 4);
            memcpy(gmicImage._data + pos + blueOffset     ,floatRGBApixel + 8, 4);
            memcpy(gmicImage._data + pos + alphaOffset    ,floatRGBApixel + 12, 4);
        }
    }
}


KisPaintDeviceSP KisGmicSimpleConvertor::convertFromGmicImage(CImg< float >& gmicImage)
{
    const KoColorSpace *rgbaFloat32bitcolorSpace = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(),
                                                                                                Float32BitsColorDepthID.id(),
                                                                                                KoColorSpaceRegistry::instance()->rgb8()->profile());

    KisPaintDeviceSP dev = new KisPaintDevice(rgbaFloat32bitcolorSpace);
    int greenOffset = gmicImage._width * gmicImage._height;
    int blueOffset = greenOffset * 2;
    int alphaOffset = greenOffset * 3;
    QRect rc(0,0,gmicImage._width, gmicImage._height);

    KisRandomAccessorSP it = dev->createRandomAccessorNG(0,0);
    int pos;
    float r,g,b,a;

    switch (gmicImage._spectrum)
    {
        case 1:
        {
            // convert grayscale to rgba
            for (int y = 0; y < rc.height(); y++)
            {
                for (int x = 0; x < rc.width(); x++)
                {
                    pos = y * gmicImage._width + x;
                    it->moveTo(x, y);
                    quint8 * floatRGBApixel = it->rawData(); // we are interpreting pixels here

                    r = g = b = gmicImage._data[pos] / m_multiplier;
                    a = 1.0;

                    memcpy(floatRGBApixel,      &r,4);
                    memcpy(floatRGBApixel + 4,  &g,4);
                    memcpy(floatRGBApixel + 8,  &b,4);
                    memcpy(floatRGBApixel + 12, &a,4);
                }
            }
            break;
        }
        case 2:
        {
            // convert grayscale alpha to rgba
            for (int y = 0; y < rc.height(); y++)
            {
                for (int x = 0; x < rc.width(); x++)
                {
                    pos = y * gmicImage._width + x;
                    it->moveTo(x, y);
                    quint8 * floatRGBApixel = it->rawData(); // we are interpreting pixels here

                    r = g = b = gmicImage._data[pos] / m_multiplier;
                    a = gmicImage._data[pos + greenOffset] / m_multiplier;

                    memcpy(floatRGBApixel,      &r,4);
                    memcpy(floatRGBApixel + 4,  &g,4);
                    memcpy(floatRGBApixel + 8,  &b,4);
                    memcpy(floatRGBApixel + 12, &a,4);
                }
            }
            break;
        }
        case 3:
        {
            // convert rgb -> rgba
            for (int y = 0; y < rc.height(); y++)
            {
                for (int x = 0; x < rc.width(); x++)
                {
                    pos = y * gmicImage._width + x;
                    it->moveTo(x, y);
                    quint8 * floatRGBApixel = it->rawData(); // we are interpreting pixels here

                    r = gmicImage._data[pos] / m_multiplier;
                    g = gmicImage._data[pos + greenOffset] / m_multiplier;
                    b = gmicImage._data[pos + blueOffset] / m_multiplier;
                    a = 1.0;

                    memcpy(floatRGBApixel,      &r,4);
                    memcpy(floatRGBApixel + 4,  &g,4);
                    memcpy(floatRGBApixel + 8,  &b,4);
                    memcpy(floatRGBApixel + 12, &a,4);
                }
            }
            break;
        }
        case 4:
        {
            for (int y = 0; y < rc.height(); y++)
            {
                for (int x = 0; x < rc.width(); x++)
                {
                    pos = y * gmicImage._width + x;
                    it->moveTo(x, y);
                    quint8 * floatRGBApixel = it->rawData(); // we are interpreting pixels here

                    r = gmicImage._data[pos] / m_multiplier;
                    g = gmicImage._data[pos + greenOffset] / m_multiplier;
                    b = gmicImage._data[pos + blueOffset] / m_multiplier;
                    a = gmicImage._data[pos + alphaOffset] / m_multiplier;

                    memcpy(floatRGBApixel,      &r,4);
                    memcpy(floatRGBApixel + 4,  &g,4);
                    memcpy(floatRGBApixel + 8,  &b,4);
                    memcpy(floatRGBApixel + 12, &a,4);
                }
            }
            break;
        }

        default:
        {
            dbgPlugins << "Unsupported gmic output format : " <<  gmicImage._width << gmicImage._height << gmicImage._depth << gmicImage._spectrum;
        }
    }

    return dev;
}


QImage KisGmicSimpleConvertor::convertToQImage(gmic_image<float>& gmicImage)
{

    QImage image = QImage(gmicImage._width, gmicImage._height, QImage::Format_ARGB32);

    dbgPlugins << image.format() <<"first pixel:"<< gmicImage._data[0] << gmicImage._width << gmicImage._height << gmicImage._spectrum;

    int greenOffset = gmicImage._width * gmicImage._height;
    int blueOffset = greenOffset * 2;
    int pos = 0;

    for (unsigned int y = 0; y < gmicImage._height; y++)
    {
        QRgb *pixel = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (unsigned int x = 0; x < gmicImage._width; x++)
        {
            pos = y * gmicImage._width + x;
            float r = gmicImage._data[pos];
            float g = gmicImage._data[pos + greenOffset];
            float b = gmicImage._data[pos + blueOffset];
            pixel[x] = qRgb(int(r),int(g), int(b));
        }
    }
    return image;
}


void KisGmicSimpleConvertor::convertFromQImage(const QImage& image, CImg< float >& gmicImage, qreal multiplied)
{
    int greenOffset = gmicImage._width * gmicImage._height;
    int blueOffset = greenOffset * 2;
    int alphaOffset = greenOffset * 3;
    int pos = 0;

    Q_ASSERT(image.width() == int(gmicImage._width));
    Q_ASSERT(image.height() == int(gmicImage._height));
    Q_ASSERT(image.format() == QImage::Format_ARGB32);
    Q_ASSERT(gmicImage._spectrum == 4);

    for (int y = 0; y < image.height(); y++)
    {
        const QRgb *pixel = reinterpret_cast<const QRgb *>(image.scanLine(y));
        for (int x = 0; x < image.width(); x++)
        {
            pos = y * gmicImage._width + x;
            gmicImage._data[pos]                = qRed(pixel[x]) / multiplied;
            gmicImage._data[pos + greenOffset]  = qGreen(pixel[x]) / multiplied;
            gmicImage._data[pos + blueOffset]   = qBlue(pixel[x]) / multiplied;
            gmicImage._data[pos + alphaOffset]   = qAlpha(pixel[x]) / multiplied;
        }
    }

}

