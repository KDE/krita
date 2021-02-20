/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
    KisQImagePyramid(const QImage &baseImage, bool useSmoothingForEnlarging = true);
    ~KisQImagePyramid();

    static QSize imageSize(const QSize &originalSize,
                           KisDabShape const&,
                           qreal subPixelX, qreal subPixelY);

    static QSizeF characteristicSize(const QSize &originalSize, KisDabShape const&);

    QImage createImage(KisDabShape const&,
                       qreal subPixelX, qreal subPixelY) const;

    QImage getClosest(QTransform transform, qreal *scale) const;

    QImage getClosestWithoutWorkaroundBorder(QTransform transform, qreal *scale) const;

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
    qreal m_baseScale {0.0};

    struct PyramidLevel {
        PyramidLevel() {}
        PyramidLevel(QImage _image, QSize _size) : image(_image), size(_size) {}

        QImage image;
        QSize size;
    };

    QVector<PyramidLevel> m_levels;
};

#endif /* __KIS_QIMAGE_PYRAMID_H */
