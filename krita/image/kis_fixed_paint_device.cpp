/*
 *  Copyright (c) 2009 Boudewijn Rempt <boud@valdyas.org>
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
#include "kis_fixed_paint_device.h"

#include <KoColorSpaceRegistry.h>
#include <KoColor.h>
#include <KoColorModelStandardIds.h>

KisFixedPaintDevice::KisFixedPaintDevice(const KoColorSpace* colorSpace)
        : m_colorSpace(colorSpace)
{
}


KisFixedPaintDevice::~KisFixedPaintDevice()
{
}

KisFixedPaintDevice::KisFixedPaintDevice(const KisFixedPaintDevice& rhs)
        : KisShared(rhs)
{
    m_bounds = rhs.m_bounds;
    m_colorSpace = rhs.m_colorSpace;
    m_data = rhs.m_data;
}

void KisFixedPaintDevice::setRect(const QRect& rc)
{
    m_bounds = rc;
}


QRect KisFixedPaintDevice::bounds() const
{
    return m_bounds;
}

quint32 KisFixedPaintDevice::pixelSize() const
{
    return m_colorSpace->pixelSize();
}

bool KisFixedPaintDevice::initialize(quint8 defaultValue)
{
    m_data.fill(defaultValue, m_bounds.height() * m_bounds.width() * pixelSize());

    return true;
}

quint8* KisFixedPaintDevice::data()
{
    return m_data.data();
}

quint8* KisFixedPaintDevice::data() const
{
    return const_cast<quint8*>(m_data.data());
}

void KisFixedPaintDevice::convertTo(const KoColorSpace* dstColorSpace, KoColorConversionTransformation::Intent renderingIntent)
{
    if (*m_colorSpace == *dstColorSpace) {
        return;
    }
    quint32 size = m_bounds.width() * m_bounds.height();
    QVector<quint8> dstData(size * dstColorSpace->pixelSize());

    m_colorSpace->convertPixelsTo(data(), dstData.data(),
                                  dstColorSpace,
                                  size,
                                  renderingIntent);

    m_colorSpace = dstColorSpace;
    m_data = dstData;

}

void KisFixedPaintDevice::convertFromQImage(const QImage& _image, const QString &srcProfileName)
{
    QImage image = _image;

    if (image.format() != QImage::Format_ARGB32) {
        image = image.convertToFormat(QImage::Format_ARGB32);
    }
    setRect(image.rect());
    initialize();

    // Don't convert if not no profile is given and both paint dev and qimage are rgba.
    if (srcProfileName.isEmpty() && colorSpace()->id() == "RGBA") {
        memcpy(data(), image.bits(), image.width() * image.height() * pixelSize());
    } else {
        KoColorSpaceRegistry::instance()
        ->colorSpace( RGBAColorModelID.id(), Integer8BitsColorDepthID.id(), srcProfileName)
        ->convertPixelsTo(image.bits(), data(), colorSpace(), image.width() * image.height());
    }
}

QImage KisFixedPaintDevice::convertToQImage(const KoColorProfile *  dstProfile)
{
    qint32 x1;
    qint32 y1;
    qint32 w;
    qint32 h;

    x1 = m_bounds.x();
    y1 = m_bounds.y();
    w = m_bounds.width();
    h = m_bounds.height();

    return convertToQImage(dstProfile, x1, y1, w, h);
}

QImage KisFixedPaintDevice::convertToQImage(const KoColorProfile *  dstProfile, qint32 x1, qint32 y1, qint32 w, qint32 h)
{

    if (w < 0)
        return QImage();

    if (h < 0)
        return QImage();

    if (QRect(x1, y1, w, h) == m_bounds) {
        return colorSpace()->convertToQImage(data(), w, h, dstProfile,
                                             KoColorConversionTransformation::IntentPerceptual);
    } else {
        quint8* newData = new quint8[w * h * pixelSize()];
        quint8* srcPtr = data();
        quint8* dstPtr = newData;
        int pSize = pixelSize();
        // copy the right area out of the paint device into data
        for (int row = y1; row < h; row++) {
            memcpy(dstPtr, srcPtr, w * pSize);
            srcPtr += (row * w * pSize) + (y1 * pSize);
            dstPtr += w * pSize;
        }
        QImage image = colorSpace()->convertToQImage(newData, w, h, dstProfile,
                       KoColorConversionTransformation::IntentPerceptual);
        return image;
    }
}


void KisFixedPaintDevice::clear(const QRect & rc)
{
    KoColor c(Qt::black, m_colorSpace);
    quint8* black = m_colorSpace->allocPixelBuffer(1);
    memcpy(black, c.data(), m_colorSpace->pixelSize());
    m_colorSpace->setAlpha(black, OPACITY_TRANSPARENT, 1);
    fill(rc.x(), rc.y(), rc.width(), rc.height(), black);
    delete[] black;
}

void KisFixedPaintDevice::fill(qint32 x, qint32 y, qint32 w, qint32 h, const quint8 *fillPixel)
{
    if (m_data.isEmpty() || m_bounds.isEmpty()) {
        setRect(QRect(x, y, w, h));
        initialize();
    }

    QRect rc(x, y, w, h);
    if (!m_bounds.contains(rc)) {
        rc = m_bounds;
    }

    quint8 pixelSize = m_colorSpace->pixelSize();
    quint8* dabPointer = data();

    if (rc.contains(m_bounds)) {
        for (int i = 0; i < w * h ; ++i) {
            memcpy(dabPointer, fillPixel, pixelSize);
            dabPointer += pixelSize;
        }
    } else {
        int y1 = y;
        for (int row = y1; row < h; row++) {
            memcpy(dabPointer, fillPixel, w * pixelSize);
            dabPointer += (row * w * pixelSize) + (y1 * pixelSize);
        }
    }
}
