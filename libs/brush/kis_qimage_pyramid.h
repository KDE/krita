/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_QIMAGE_PYRAMID_H
#define __KIS_QIMAGE_PYRAMID_H

#include <QImage>
#include <QVector>
#include <kis_dab_shape.h>
#include <kritabrush_export.h>


class BRUSH_EXPORT KisQImagePyramid
{
public:
    KisQImagePyramid() = default;
    KisQImagePyramid(const QImage &baseImage);
    ~KisQImagePyramid();

    static QSize imageSize(const QSize &originalSize,
                           KisDabShape const&,
                           qreal subPixelX, qreal subPixelY);

    static QSizeF characteristicSize(const QSize &originalSize, KisDabShape const&);

    QImage createImage(KisDabShape const&,
                       qreal subPixelX, qreal subPixelY) const;

    QImage getClosest(QTransform transform, qreal *scale) const;

private:
    friend class KisGbrBrushTest;
    int findNearestLevel(qreal scale, qreal *baseScale) const;
    void appendPyramidLevel(const QImage &image);

    static void calculateParams(KisDabShape const& shape,
                                qreal subPixelX, qreal subPixelY,
                                const QSize &originalSize,
                                QTransform *outputTransform, QSize *outputSize);

    static void calculateParams(KisDabShape shape,
                                qreal subPixelX, qreal subPixelY,
                                const QSize &originalSize,
                                qreal baseScale, const QSize &baseSize,
                                QTransform *outputTransform, QSize *outputSize);

private:
    QSize m_originalSize;
    qreal m_baseScale;

    struct PyramidLevel {
        PyramidLevel() {}
        PyramidLevel(QImage _image, QSize _size) : image(_image), size(_size) {}

        QImage image;
        QSize size;
    };

    QVector<PyramidLevel> m_levels;
};

#endif /* __KIS_QIMAGE_PYRAMID_H */
