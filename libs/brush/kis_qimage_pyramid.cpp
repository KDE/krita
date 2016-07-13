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

#include "kis_qimage_pyramid.h"

#include <limits>
#include <QPainter>
#include <kis_debug.h>

inline QRect roundRect(const QRectF &rc)
{
    /**
     * This is an analog of toAlignedRect() with the only difference
     * that it ensures the rect position will never be below zero.
     *
     * Warning: be *very* careful with using bottom()/right() values
     *          of a pure QRect (we don't use it here for the dangers
     *          it can lead to).
     */

    QRectF rect(rc);

    KIS_ASSERT_RECOVER_NOOP(rect.x() > -1e-6);
    KIS_ASSERT_RECOVER_NOOP(rect.y() > -1e-6);

    if (rect.x() < 0.0) {
        rect.setLeft(0.0);
    }

    if (rect.y() < 0.0) {
        rect.setTop(0.0);
    }

    return rect.toAlignedRect();
}

QTransform baseBrushTransform(qreal scaleX, qreal scaleY,
                              qreal rotation,
                              qreal subPixelX, qreal subPixelY,
                              const QRectF &baseBounds)
{
    QTransform transform;
    if (!qFuzzyCompare(rotation, 0)) {
        QTransform rotationTransform;
        rotationTransform.rotateRadians(rotation);

        QRectF rotatedBounds = rotationTransform.mapRect(baseBounds);
        transform = rotationTransform *
                    QTransform::fromTranslate(-rotatedBounds.x(), -rotatedBounds.y());
    }

    return transform *
           QTransform::fromScale(scaleX, scaleY) *
           QTransform::fromTranslate(subPixelX, subPixelY);
}

void KisQImagePyramid::calculateParams(qreal scale, qreal rotation,
                                       qreal subPixelX, qreal subPixelY,
                                       const QSize &originalSize,
                                       QTransform *outputTransform, QSize *outputSize)
{
    calculateParams(scale, rotation,
                    subPixelX, subPixelY,
                    originalSize, 1.0, originalSize,
                    outputTransform, outputSize);
}

void KisQImagePyramid::calculateParams(qreal scale, qreal rotation,
                                       qreal subPixelX, qreal subPixelY,
                                       const QSize &originalSize,
                                       qreal baseScale, const QSize &baseSize,
                                       QTransform *outputTransform, QSize *outputSize)
{
    Q_UNUSED(baseScale);

    QRectF originalBounds = QRectF(QPointF(), originalSize);
    QTransform originalTransform =
        baseBrushTransform(scale, scale,
                           rotation,
                           subPixelX, subPixelY,
                           originalBounds);

    qreal realBaseScaleX = qreal(baseSize.width()) / originalSize.width();
    qreal realBaseScaleY = qreal(baseSize.height()) / originalSize.height();

    qreal scaleX = scale / realBaseScaleX;
    qreal scaleY = scale / realBaseScaleY;

    QRectF baseBounds = QRectF(QPointF(), baseSize);

    QTransform transform =
        baseBrushTransform(scaleX, scaleY,
                           rotation,
                           subPixelX, subPixelY,
                           baseBounds);

    QRect expectedDstRect = roundRect(originalTransform.mapRect(originalBounds));
#if 0 // Only enable when debugging; users shouldn't see this warning
    {
        QRect testingRect = roundRect(transform.mapRect(baseBounds));
        if (testingRect != expectedDstRect) {
            warnKrita << "WARNING: expected and real dab rects do not coincide!";
            warnKrita << "         expected rect:" << expectedDstRect;
            warnKrita << "         real rect:    " << testingRect;
        }
    }
#endif
    KIS_ASSERT_RECOVER_NOOP(expectedDstRect.x() >= 0);
    KIS_ASSERT_RECOVER_NOOP(expectedDstRect.y() >= 0);

    int width = expectedDstRect.x() + expectedDstRect.width();
    int height = expectedDstRect.y() + expectedDstRect.height();

    // we should not return invalid image, so adjust the image to be
    // at least 1 px in size.
    width = qMax(1, width);
    height = qMax(1, height);

    *outputTransform = transform;
    *outputSize = QSize(width, height);
}

QSize KisQImagePyramid::imageSize(const QSize &originalSize,
                                  qreal scale, qreal rotation,
                                  qreal subPixelX, qreal subPixelY)
{
    QTransform transform;
    QSize dstSize;

    calculateParams(scale, rotation, subPixelX, subPixelY,
                    originalSize,
                    &transform, &dstSize);

    return dstSize;
}

QSizeF KisQImagePyramid::characteristicSize(const QSize &originalSize,
                                            qreal scale, qreal rotation)
{
    QRectF originalRect(QPointF(), originalSize);
    QTransform transform = baseBrushTransform(scale, scale,
                                              rotation,
                                              0.0, 0.0,
                                              originalRect);

    return transform.mapRect(originalRect).size();
}
