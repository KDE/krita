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

#include <kis_gmic_simple_convertor.h>

#include <kis_debug.h>
#include <kis_random_accessor_ng.h>

#include <KoColorSpaceRegistry.h>
#include <KoColorSpace.h>
#include <KoColorModelStandardIds.h>
#include <KoColorSpaceTraits.h>

using namespace cimg_library;

// gmic assumes float rgba in 0.0 - 255.0, thus default value
void KisGmicSimpleConvertor::convertToGmicImage(KisPaintDeviceSP dev, gmic_image<float>& gmicImage, QRect rc)
{
    Q_ASSERT(!dev.isNull());
    Q_ASSERT(gmicImage._spectrum == 4); // rgba

    if (rc.isEmpty())
    {
        rc = QRect(0,0,gmicImage._width, gmicImage._height);
    }

    const KoColorSpace *rgbaFloat32bitcolorSpace = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(),
                                                                                                Float32BitsColorDepthID.id(),
                                                                                                KoColorSpaceRegistry::instance()->rgb8()->profile());
    Q_CHECK_PTR(rgbaFloat32bitcolorSpace);

    int greenOffset = gmicImage._width * gmicImage._height;
    int blueOffset = greenOffset * 2;
    int alphaOffset = greenOffset * 3;

    KoColorConversionTransformation::Intent renderingIntent = KoColorConversionTransformation::InternalRenderingIntent;
    KoColorConversionTransformation::ConversionFlags conversionFlags = KoColorConversionTransformation::InternalConversionFlags;

    const KoColorSpace * colorSpace = dev->colorSpace();
    KisRandomConstAccessorSP it = dev->createRandomConstAccessorNG(0,0);

    int optimalBufferSize = 64; // most common numContiguousColumns, tile size?
    quint8 * floatRGBApixel = rgbaFloat32bitcolorSpace->allocPixelBuffer(optimalBufferSize);
    quint32 pixelSize = rgbaFloat32bitcolorSpace->pixelSize();
    int pos = 0;
    for (int y = 0; y < rc.height(); y++)
    {
        int x = 0;
        while (x < rc.width())
        {
            it->moveTo(x, y);
            qint32 numContiguousColumns = qMin(it->numContiguousColumns(x), optimalBufferSize);
            numContiguousColumns = qMin(numContiguousColumns, rc.width() - x);

            colorSpace->convertPixelsTo(it->rawDataConst(), floatRGBApixel, rgbaFloat32bitcolorSpace, numContiguousColumns, renderingIntent, conversionFlags);

            pos = y * gmicImage._width + x;
            for (qint32 bx = 0; bx < numContiguousColumns; bx++)
            {
                memcpy(gmicImage._data + pos                  ,floatRGBApixel + bx * pixelSize   , 4);
                memcpy(gmicImage._data + pos + greenOffset    ,floatRGBApixel + bx * pixelSize + 4, 4);
                memcpy(gmicImage._data + pos + blueOffset     ,floatRGBApixel + bx * pixelSize + 8, 4);
                memcpy(gmicImage._data + pos + alphaOffset    ,floatRGBApixel + bx * pixelSize + 12, 4);
                pos++;
            }

            x += numContiguousColumns;
        }
    }
    delete [] floatRGBApixel;
}

void KisGmicSimpleConvertor::convertFromGmicImage(gmic_image<float>& gmicImage, KisPaintDeviceSP dst, float gmicMaxChannelValue)
{
    Q_ASSERT(!dst.isNull());
    const KoColorSpace *rgbaFloat32bitcolorSpace = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(),
                                                                                                Float32BitsColorDepthID.id(),
                                                                                                KoColorSpaceRegistry::instance()->rgb8()->profile());
    const KoColorSpace *dstColorSpace = dst->colorSpace();
    if (dstColorSpace == 0)
    {
        dstColorSpace = rgbaFloat32bitcolorSpace;
    }

    KisPaintDeviceSP dev = dst;
    int greenOffset = gmicImage._width * gmicImage._height;
    int blueOffset = greenOffset * 2;
    int alphaOffset = greenOffset * 3;
    QRect rc(0,0,gmicImage._width, gmicImage._height);

    KisRandomAccessorSP it = dev->createRandomAccessorNG(0,0);
    int pos;
    float r,g,b,a;

    int optimalBufferSize = 64; // most common numContiguousColumns, tile size?
    quint8 * floatRGBApixel = rgbaFloat32bitcolorSpace->allocPixelBuffer(optimalBufferSize);
    quint32 pixelSize = rgbaFloat32bitcolorSpace->pixelSize();

    KoColorConversionTransformation::Intent renderingIntent = KoColorConversionTransformation::InternalRenderingIntent;
    KoColorConversionTransformation::ConversionFlags conversionFlags = KoColorConversionTransformation::InternalConversionFlags;

    // Krita needs rgba in 0.0...1.0
    float multiplied = 1.0 / gmicMaxChannelValue;

    switch (gmicImage._spectrum)
    {
        case 1:
        {
            // convert grayscale to rgba
            for (int y = 0; y < rc.height(); y++)
            {
                int x = 0;
                while (x < rc.width())
                {
                    it->moveTo(x, y);
                    qint32 numContiguousColumns = qMin(it->numContiguousColumns(x), optimalBufferSize);
                    numContiguousColumns = qMin(numContiguousColumns, rc.width() - x);

                    pos = y * gmicImage._width + x;
                    for (qint32 bx = 0; bx < numContiguousColumns; bx++)
                    {
                            r = g = b = gmicImage._data[pos] * multiplied;
                            a = gmicMaxChannelValue * multiplied;

                            memcpy(floatRGBApixel + bx * pixelSize,      &r,4);
                            memcpy(floatRGBApixel + bx * pixelSize + 4,  &g,4);
                            memcpy(floatRGBApixel + bx * pixelSize + 8,  &b,4);
                            memcpy(floatRGBApixel + bx * pixelSize + 12, &a,4);
                            pos++;
                    }
                    rgbaFloat32bitcolorSpace->convertPixelsTo(floatRGBApixel, it->rawData(), dstColorSpace, numContiguousColumns,renderingIntent, conversionFlags);
                    x += numContiguousColumns;
                }
            }
            break;
        }
        case 2:
        {
            // convert grayscale alpha to rgba
            for (int y = 0; y < rc.height(); y++)
            {
                int x = 0;
                while (x < rc.width())
                {
                    it->moveTo(x, y);
                    qint32 numContiguousColumns = qMin(it->numContiguousColumns(x), optimalBufferSize);
                    numContiguousColumns = qMin(numContiguousColumns, rc.width() - x);

                    pos = y * gmicImage._width + x;
                    for (qint32 bx = 0; bx < numContiguousColumns; bx++)
                    {
                            r = g = b = gmicImage._data[pos] * multiplied;
                            a = gmicImage._data[pos + greenOffset] * multiplied;

                            memcpy(floatRGBApixel + bx * pixelSize,      &r,4);
                            memcpy(floatRGBApixel + bx * pixelSize + 4,  &g,4);
                            memcpy(floatRGBApixel + bx * pixelSize + 8,  &b,4);
                            memcpy(floatRGBApixel + bx * pixelSize + 12, &a,4);
                            pos++;
                    }
                    rgbaFloat32bitcolorSpace->convertPixelsTo(floatRGBApixel, it->rawData(), dstColorSpace, numContiguousColumns,renderingIntent, conversionFlags);
                    x += numContiguousColumns;
                }
            }
            break;
        }
        case 3:
        {
            // convert rgb -> rgba
            for (int y = 0; y < rc.height(); y++)
            {
                int x = 0;
                while (x < rc.width())
                {
                    it->moveTo(x, y);
                    qint32 numContiguousColumns = qMin(it->numContiguousColumns(x), optimalBufferSize);
                    numContiguousColumns = qMin(numContiguousColumns, rc.width() - x);

                    pos = y * gmicImage._width + x;
                    for (qint32 bx = 0; bx < numContiguousColumns; bx++)
                    {
                            r = gmicImage._data[pos] * multiplied;
                            g = gmicImage._data[pos + greenOffset] * multiplied;
                            b = gmicImage._data[pos + blueOffset ] * multiplied;
                            a = gmicMaxChannelValue * multiplied;

                            memcpy(floatRGBApixel + bx * pixelSize,      &r,4);
                            memcpy(floatRGBApixel + bx * pixelSize + 4,  &g,4);
                            memcpy(floatRGBApixel + bx * pixelSize + 8,  &b,4);
                            memcpy(floatRGBApixel + bx * pixelSize + 12, &a,4);
                            pos++;
                    }
                    rgbaFloat32bitcolorSpace->convertPixelsTo(floatRGBApixel, it->rawData(), dstColorSpace, numContiguousColumns,renderingIntent, conversionFlags);
                    x += numContiguousColumns;
                }
            }
            break;
        }
        case 4:
        {
            for (int y = 0; y < rc.height(); y++)
            {
                int x = 0;
                while (x < rc.width())
                {
                    it->moveTo(x, y);
                    qint32 numContiguousColumns = qMin(it->numContiguousColumns(x), optimalBufferSize);
                    numContiguousColumns = qMin(numContiguousColumns, rc.width() - x);

                    pos = y * gmicImage._width + x;
                    for (qint32 bx = 0; bx < numContiguousColumns; bx++)
                    {
                            r = gmicImage._data[pos] * multiplied;
                            g = gmicImage._data[pos + greenOffset] * multiplied;
                            b = gmicImage._data[pos + blueOffset ] * multiplied;
                            a = gmicImage._data[pos + alphaOffset] * multiplied;

                            memcpy(floatRGBApixel + bx * pixelSize,      &r,4);
                            memcpy(floatRGBApixel + bx * pixelSize + 4,  &g,4);
                            memcpy(floatRGBApixel + bx * pixelSize + 8,  &b,4);
                            memcpy(floatRGBApixel + bx * pixelSize + 12, &a,4);
                            pos++;
                    }
                    rgbaFloat32bitcolorSpace->convertPixelsTo(floatRGBApixel, it->rawData(), dstColorSpace, numContiguousColumns,renderingIntent, conversionFlags);
                    x += numContiguousColumns;
                }
            }
            break;
        }

        default:
        {
            dbgPlugins << "Unsupported gmic output format : " <<  gmicImage._width << gmicImage._height << gmicImage._depth << gmicImage._spectrum;
        }
    }
}


QImage KisGmicSimpleConvertor::convertToQImage(gmic_image<float>& gmicImage, float gmicActualMaxChannelValue)
{

    QImage image = QImage(gmicImage._width, gmicImage._height, QImage::Format_ARGB32);

    dbgPlugins << image.format() <<"first pixel:"<< gmicImage._data[0] << gmicImage._width << gmicImage._height << gmicImage._spectrum;

    int greenOffset = gmicImage._width * gmicImage._height;
    int blueOffset = greenOffset * 2;
    int pos = 0;

    // always put 255 to qimage
    float multiplied = 255.0f / gmicActualMaxChannelValue;

    for (unsigned int y = 0; y < gmicImage._height; y++)
    {
        QRgb *pixel = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (unsigned int x = 0; x < gmicImage._width; x++)
        {
            pos = y * gmicImage._width + x;
            float r = gmicImage._data[pos] * multiplied;
            float g = gmicImage._data[pos + greenOffset]  * multiplied;
            float b = gmicImage._data[pos + blueOffset]  * multiplied;
            pixel[x] = qRgb(int(r),int(g), int(b));
        }
    }
    return image;
}


void KisGmicSimpleConvertor::convertFromQImage(const QImage& image, CImg< float >& gmicImage, float gmicMaxChannelValue)
{
    int greenOffset = gmicImage._width * gmicImage._height;
    int blueOffset = greenOffset * 2;
    int alphaOffset = greenOffset * 3;
    int pos = 0;

    // QImage has 0..255
    float multiplied = gmicMaxChannelValue / 255.0;


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
            gmicImage._data[pos]                = qRed(pixel[x]) * multiplied;
            gmicImage._data[pos + greenOffset]  = qGreen(pixel[x]) * multiplied;
            gmicImage._data[pos + blueOffset]   = qBlue(pixel[x]) * multiplied;
            gmicImage._data[pos + alphaOffset]   = qAlpha(pixel[x]) * multiplied;
        }
    }

}

