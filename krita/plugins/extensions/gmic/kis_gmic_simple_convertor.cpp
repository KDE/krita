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

#include <kis_gmic_simple_convertor.h>

#include <kis_paint_device.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorSpace.h>
#include <KoColorModelStandardIds.h>
#include <KoColorSpaceTraits.h>

using namespace cimg_library;


KisGmicSimpleConvertor::KisGmicSimpleConvertor():m_channelSize(0)
{

}


KisGmicSimpleConvertor::~KisGmicSimpleConvertor()
{
    deletePlanes();
}


void KisGmicSimpleConvertor::convertToGmicImage(KisPaintDeviceSP dev, gmic_image< float >& gmicImage)
{
    const KoColorSpace *rgbaFloat32bitcolorSpace = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(),
                                                                                                Float32BitsColorDepthID.id(),
                                                                                                KoColorSpaceRegistry::instance()->rgb8()->profile());

    Q_CHECK_PTR(rgbaFloat32bitcolorSpace);
    dev->convertTo(rgbaFloat32bitcolorSpace);

    QRect rc = dev->exactBounds();
    m_planarBytes = dev->readPlanarBytes(rc.x(), rc.y(), rc.width(), rc.height());
    setChannelSize(rc.width() * rc.height());

    int greenOffset = gmicImage._width * gmicImage._height;
    int blueOffset = greenOffset * 2;
    int alphaOffset = greenOffset * 3;
    quint8 * redChannelBytes   = m_planarBytes.at(KoRgbF32Traits::red_pos);
    quint8 * greenChannelBytes = m_planarBytes.at(KoRgbF32Traits::green_pos);
    quint8 * blueChannelBytes  = m_planarBytes.at(KoRgbF32Traits::blue_pos);
    quint8 * alphaChannelBytes = m_planarBytes.at(KoRgbF32Traits::alpha_pos);

    unsigned int channelSize = sizeof(float);

    memcpy(gmicImage._data                  ,redChannelBytes    ,gmicImage._width * gmicImage._height * channelSize);
    memcpy(gmicImage._data + greenOffset    ,greenChannelBytes  ,gmicImage._width * gmicImage._height * channelSize);
    memcpy(gmicImage._data + blueOffset     ,blueChannelBytes   ,gmicImage._width * gmicImage._height * channelSize);
    memcpy(gmicImage._data + alphaOffset    ,alphaChannelBytes  ,gmicImage._width * gmicImage._height * channelSize);
}


KisPaintDeviceSP KisGmicSimpleConvertor::convertFromGmicImage(CImg< float >& gmicImage, bool &preserveAlpha)
{
    const KoColorSpace *rgbaFloat32bitcolorSpace = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(),
                                                                                                Float32BitsColorDepthID.id(),
                                                                                                KoColorSpaceRegistry::instance()->rgb8()->profile());

    KisPaintDeviceSP dev = new KisPaintDevice(rgbaFloat32bitcolorSpace);


    unsigned int channelBytes = gmicImage._width * gmicImage._height * sizeof(float);
    if (channelBytes == channelSize() * sizeof(float))
    {
        // ok, we can reuse read plannar bytes here
        qDebug() << "[krita] Re-using read plannar bytes";
        if ((gmicImage._spectrum == 1) || (gmicImage._spectrum == 3))
        {
            qDebug() << "[krita] Releasing alpha channel";
            // we can delete alpha channel
            releaseAlphaChannel();
        }

    }
    else
    {
        // re-accumullate buffers, output image has different dimension..not sure if this ever happens
        deletePlanes();
        bool alphaChannelEnabled = ((gmicImage._spectrum == 2) || (gmicImage._spectrum == 4));
        qDebug() << "Accumulating...!";
        accumulate(gmicImage._width * gmicImage._height, alphaChannelEnabled);
    }

    switch (gmicImage._spectrum)
    {
        case 1:
        {
            grayscale2rgb(gmicImage, m_planarBytes);
            preserveAlpha = true;
            break;
        }
        case 2:
        {
            grayscaleAlpha2rgba(gmicImage, m_planarBytes);
            break;
        }
        case 3:
        {
            rgb2rgb(gmicImage, m_planarBytes);
            preserveAlpha = true;
            break;
        }
        case 4:
            rgba2rgba(gmicImage, m_planarBytes);
            break;
        default:
        {
            qDebug() << "Unsupported gmic output format : " <<  gmicImage._width << gmicImage._height << gmicImage._depth << gmicImage._spectrum;
        }
    }

    dev->writePlanarBytes(m_planarBytes, 0, 0, gmicImage._width, gmicImage._height);

    // release planes
    deletePlanes();
    return dev;
}

void KisGmicSimpleConvertor::grayscale2rgb(CImg< float >& gmicImage, QVector< quint8 * > &planes)
{
    quint8 * redChannelBytes = planes[0];
    quint8 * greenChannelBytes = planes[1];
    quint8 * blueChannelBytes = planes[2];
    // alphaChannel will be preserved

    int pos = 0;
    float r,g,b;

    // iterate over gmic image and fill plane buffers
    for (unsigned int y = 0; y < gmicImage._height; y++)
    {
        for (unsigned int x = 0; x < gmicImage._width; x++)
        {
            pos = y * gmicImage._width + x;
            // gmic assumes 0.0 - 255.0, Krita stores 0.0 - 1.0
            r = g = b = gmicImage._data[pos]                / 255.0;
            memcpy(redChannelBytes,     &r, 4); redChannelBytes     += 4;
            memcpy(greenChannelBytes,   &g, 4); greenChannelBytes   += 4;
            memcpy(blueChannelBytes,    &b, 4); blueChannelBytes    += 4;
        }
    }
    // TODO: check performance if memcpy whole channel is faster
}


void KisGmicSimpleConvertor::grayscaleAlpha2rgba(CImg< float >& gmicImage, QVector< quint8 * > &planes)
{
    quint8 * redChannelBytes = planes[0];
    quint8 * greenChannelBytes = planes[1];
    quint8 * blueChannelBytes = planes[2];
    quint8 * alphaChannelBytes = planes[3];

    int pos = 0;
    int alphaOffset = gmicImage._width * gmicImage._height;
    float r,g,b,a;

    // iterate over gmic image and fill plane buffers
    for (unsigned int y = 0; y < gmicImage._height; y++)
    {
        for (unsigned int x = 0; x < gmicImage._width; x++){
            pos = y * gmicImage._width + x;

            // gmic assumes 0.0 - 255.0, Krita stores 0.0 - 1.0
            r = g = b = gmicImage._data[pos]                / 255.0;
            a = gmicImage._data[pos + alphaOffset]   / 255.0;

            memcpy(redChannelBytes,     &r, 4); redChannelBytes     += 4;
            memcpy(greenChannelBytes,   &g, 4); greenChannelBytes   += 4;
            memcpy(blueChannelBytes,    &b, 4); blueChannelBytes    += 4;
            memcpy(alphaChannelBytes,   &a, 4); alphaChannelBytes   += 4;
        }
    }
}


void KisGmicSimpleConvertor::rgb2rgb(CImg< float >& gmicImage, QVector< quint8 * > &planes)
{
    quint8 * redChannelBytes = planes[0];
    quint8 * greenChannelBytes = planes[1];
    quint8 * blueChannelBytes = planes[2];
    // alphaChannel will be preserved

    int pos = 0;
    int greenOffset = gmicImage._width * gmicImage._height;
    int blueOffset = greenOffset * 2;

    float r,g,b;

    // iterate over gmic image and fill plane buffers
    for (unsigned int y = 0; y < gmicImage._height; y++)
    {
        for (unsigned int x = 0; x < gmicImage._width; x++)
        {
            pos = y * gmicImage._width + x;

            // gmic assumes 0.0 - 255.0, Krita stores 0.0 - 1.0
            r = gmicImage._data[pos]                / 255.0;
            g = gmicImage._data[pos + greenOffset]  / 255.0;
            b = gmicImage._data[pos + blueOffset]   / 255.0;

            memcpy(redChannelBytes,     &r, 4); redChannelBytes     += 4;
            memcpy(greenChannelBytes,   &g, 4); greenChannelBytes   += 4;
            memcpy(blueChannelBytes,    &b, 4); blueChannelBytes    += 4;
        }
    }
}

void KisGmicSimpleConvertor::rgba2rgba(CImg< float >& gmicImage, QVector< quint8 * > &planes)
{
    qDebug() <<"planes-size"<< planes.size();

    quint8 * redChannelBytes = planes[0];
    quint8 * greenChannelBytes = planes[1];
    quint8 * blueChannelBytes = planes[2];
    quint8 * alphaChannelBytes = planes[3];

    int pos = 0;
    int greenOffset = gmicImage._width * gmicImage._height;
    int blueOffset = greenOffset * 2;
    int alphaOffset = greenOffset * 3;

    float r,g,b,a;

    // iterate over gmic image and fill plane buffers
    for (unsigned int y = 0; y < gmicImage._height; y++)
    {
        for (unsigned int x = 0; x < gmicImage._width; x++)
        {
            pos = y * gmicImage._width + x;

            // gmic assumes 0.0 - 255.0, Krita stores 0.0 - 1.0
            r = gmicImage._data[pos]                / 255.0;
            g = gmicImage._data[pos + greenOffset]  / 255.0;
            b = gmicImage._data[pos + blueOffset]   / 255.0;
            a = gmicImage._data[pos + alphaOffset]  / 255.0;

            memcpy(redChannelBytes,     &r, 4); redChannelBytes     += 4;
            memcpy(greenChannelBytes,   &g, 4); greenChannelBytes   += 4;
            memcpy(blueChannelBytes,    &b, 4); blueChannelBytes    += 4;
            memcpy(alphaChannelBytes,   &a, 4); alphaChannelBytes   += 4;
        }
    }
}
