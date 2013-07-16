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

#include <QPainter>
#include <kis_debug.h>

#define MIPMAP_SIZE_THRESHOLD 512
#define MAX_MIPMAP_SCALE 8.0

KisQImagePyramid::KisQImagePyramid(const QImage &baseImage)
{
    Q_ASSERT(!baseImage.isNull());

    m_baseSize = baseImage.size();


    qreal scale = MAX_MIPMAP_SCALE;

    while (scale > 1.0) {
        QSize scaledSize = m_baseSize * scale;

        if (scaledSize.width() <= MIPMAP_SIZE_THRESHOLD ||
            scaledSize.height() <= MIPMAP_SIZE_THRESHOLD) {

            if (m_levels.isEmpty()) {
                m_baseScale = scale;
            }

            m_levels.append(baseImage.scaled(scaledSize,  Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        }

        scale *= 0.5;
    }

    if (m_levels.isEmpty()) {
        m_baseScale = 1.0;
    }
    m_levels.append(baseImage);

    scale = 0.5;
    while (true) {
        QSize scaledSize = m_baseSize * scale;

        if (scaledSize.width() == 0 ||
            scaledSize.height() == 0) break;

        m_levels.append(baseImage.scaled(scaledSize,  Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

        scale *= 0.5;
    }
}

KisQImagePyramid::~KisQImagePyramid()
{
}

int KisQImagePyramid::findNearestLevel(qreal scale, qreal *baseScale)
{
    qreal levelScale = m_baseScale;
    int level = 0;
    int lastLevel = m_levels.size() - 1;

    while (0.5 * levelScale > scale && level < lastLevel) {
        levelScale *= 0.5;
        level++;
    }

    *baseScale = levelScale;
    return level;
}

inline QRect roundRect(const QRectF &rc) {
    /**
     * This is an analog of toAlignedRect() with the only difference
     * that it rounds corner values instead of doing floor/ceil.
     *
     * Warning: be *very* careful with using bottom()/right() values
     *          of a pure QRect (we don't use it here for the dangers
     *          it can lead to).
     */

    int left = qRound(rc.left());
    int right = qRound(rc.right());

    int top = qRound(rc.top());
    int bottom = qRound(rc.bottom());

    return QRect(left, top, right - left, bottom - top);
}

void KisQImagePyramid::calculateParams(qreal scale, qreal rotation,
                                       qreal subPixelX, qreal subPixelY,
                                       qreal baseScale, const QSize &baseSize,
                                       QTransform *outputTransform, QSize *outputSize)
{
    QTransform transform;
    QRectF baseBounds = QRectF(QPointF(), baseSize);

    qreal scaleX = scale / baseScale;
    qreal scaleY = scale / baseScale;

    if (!qFuzzyCompare(rotation, 0)) {
        QTransform rotationTransform;
        rotationTransform.rotateRadians(rotation);

        QRectF rotatedBounds = rotationTransform.mapRect(baseBounds);
        transform = QTransform().rotateRadians(rotation) *
            QTransform::fromTranslate(-rotatedBounds.x(), -rotatedBounds.y());
    } else {
        QRectF scaledRect = QTransform::fromScale(scaleX, scaleY).mapRect(baseBounds).toAlignedRect();
        scaleX = scaledRect.width() / baseBounds.width();
        scaleY = scaledRect.height() / baseBounds.height();
    }


    transform *= QTransform::fromScale(scaleX, scaleY) *
        QTransform::fromTranslate(subPixelX, subPixelY);

    QRect dstRect = roundRect(transform.mapRect(baseBounds));
    Q_ASSERT(dstRect.x() >= 0);
    Q_ASSERT(dstRect.y() >= 0);

    int width = dstRect.x() + dstRect.width();
    int height = dstRect.y() + dstRect.height();

    // we should not return invalid image, so adjust the image to be
    // at least 1 px in size.
    width = qMax(1, width);
    height = qMax(1, height);

    *outputTransform = transform;
    *outputSize = QSize(width, height);
}

QSize KisQImagePyramid::imageSize(const QSize &baseSize,
                                  qreal scale, qreal rotation,
                                  qreal subPixelX, qreal subPixelY)
{
    QTransform transform;
    QSize dstSize;

    calculateParams(scale, rotation, subPixelX, subPixelY,
                    1.0, baseSize,
                    &transform, &dstSize);

    return dstSize;
}

QImage KisQImagePyramid::createImage(qreal scale, qreal rotation,
                                     qreal subPixelX, qreal subPixelY)
{
    qreal baseScale = -1.0;
    int level = findNearestLevel(scale, &baseScale);

    const QImage &srcImage = m_levels[level];

    QTransform transform;
    QSize dstSize;

    calculateParams(scale, rotation, subPixelX, subPixelY,
                    baseScale, srcImage.size(),
                    &transform, &dstSize);

    if (transform.isIdentity()) {
        return srcImage;
    }

    QImage dstImage(dstSize, QImage::Format_ARGB32);
    dstImage.fill(0);

    QPainter gc(&dstImage);
    gc.setTransform(transform);
    gc.setRenderHints(QPainter::SmoothPixmapTransform);
    gc.drawImage(QPointF(), srcImage);
    gc.end();

    return dstImage;
}

