/*
 *  Copyright (c) 2004-2008 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_qimage_mask.h"

#include <cfloat>
#include <QImage>
#include <QVector>

#include <kis_debug.h>

#include <KoColorSpace.h>

#include "kis_global.h"


KisQImagemask::KisQImagemask(const QImage& image, bool hasColor)
{
    init(image);
    if (hasColor) {
        copyAlpha(image);
    } else {
        computeAlpha(image);
    }
}

KisQImagemask::KisQImagemask(const QImage& image)
{
    init(image);
    if (!image.allGray()) {
        copyAlpha(image);
    } else {
        computeAlpha(image);
    }
}

KisQImagemask::KisQImagemask(qint32 width, qint32 height) : m_width(width), 
                                                            m_height(height),
                                                            m_data(width, height, QImage::Format_Indexed8)
{
    m_data.fill(0);
}

inline void KisQImagemask::init(const QImage& image)
{
    m_width = image.width();
    m_height = image.height();
    m_data = QImage(m_width,m_height,QImage::Format_Indexed8);
}


KisQImagemask::~KisQImagemask()
{
}

qint32 KisQImagemask::width() const
{
    return m_width;
}

qint32 KisQImagemask::height() const
{
    return m_height;
}
void KisQImagemask::copyAlpha(const QImage& image)
{
    for (int y = 0; y < image.height(); y++) {
        const QRgb *scanline = reinterpret_cast<const QRgb *>(image.scanLine(y));
        for (int x = 0; x < image.width(); x++) {
            QRgb c = scanline[x];
            quint8 a = (qGray(c) * qAlpha(c)) / 255;
            m_data.scanLine(y)[x] = a;
        }
    }
}

void KisQImagemask::computeAlpha(const QImage& image)
{
    // The brushes are mostly grayscale on a white background,
    // although some do have a colors. The alpha channel is seldom
    // used, so we take the average gray value of this pixel of
    // the brush as the setting for the opacitiy. We need to
    // invert it, because 255, 255, 255 is white, which is
    // completely transparent, but 255 corresponds to
    // OPACITY_OPAQUE.
    for (int y = 0; y < image.height(); y++) {
        const QRgb *scanline = reinterpret_cast<const QRgb *>(image.scanLine(y));
        for (int x = 0; x < image.width(); x++) {
            m_data.scanLine(y)[x] = (255 - qRed(scanline[x]));
        }
    }
}

KisQImagemaskSP KisQImagemask::interpolate(KisQImagemaskSP mask1, KisQImagemaskSP mask2, double t)
{
    Q_ASSERT((mask1->width() == mask2->width()) && (mask1->height() == mask2->height()));
    Q_ASSERT(t > -DBL_EPSILON && t < 1 + DBL_EPSILON);

    int width = mask1->width();
    int height = mask1->height();
    KisQImagemaskSP outputMask = KisQImagemaskSP(new KisQImagemask(width, height));
    Q_CHECK_PTR(outputMask);

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            quint8 d = static_cast<quint8>((1 - t) * mask1->alphaAt(x, y) + t * mask2->alphaAt(x, y));
            outputMask->setAlphaAt(x, y, d);
        }
    }

    return outputMask;
}

void KisQImagemask::rotation(double angle)
{
    // For some reason rotating an Indexed8 image is broken so convert to RGB32
    // Would probably be faster to have a native own implementation
    QVector<QRgb> table;
    for (int i = 0; i < 256; ++i) table.append(qRgb(i, i, i));
    m_data.setColorTable(table);
    QImage tmp = m_data.convertToFormat(QImage::Format_RGB32);
    tmp = tmp.transformed(QTransform().rotate(-angle * 180 / M_PI));
    m_data = QImage(tmp.width(), tmp.height(), QImage::Format_Indexed8);
    // Do not use convertToFormat to go back to Indexed8, since it is quiet
    // a slow general operation, while we know that we are outputing a grayscale image
    for (int y = 0; y < tmp.height(); ++y) {
        for (int x = 0; x < tmp.width(); ++x) {
            m_data.scanLine(y)[x] = tmp.scanLine(y)[4 * x];
        }
    }
}
