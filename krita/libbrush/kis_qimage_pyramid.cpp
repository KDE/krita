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

#define MIPMAP_SIZE_THRESHOLD 512
#define MAX_MIPMAP_SCALE 8.0

#define QPAINTER_WORKAROUND_BORDER 1


KisQImagePyramid::KisQImagePyramid(const QImage &baseImage)
{
    Q_ASSERT(!baseImage.isNull());

    m_originalSize = baseImage.size();


    qreal scale = MAX_MIPMAP_SCALE;

    while (scale > 1.0) {
        QSize scaledSize = m_originalSize * scale;

        if (scaledSize.width() <= MIPMAP_SIZE_THRESHOLD ||
                scaledSize.height() <= MIPMAP_SIZE_THRESHOLD) {

            if (m_levels.isEmpty()) {
                m_baseScale = scale;
            }

            appendPyramidLevel(baseImage.scaled(scaledSize,  Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        }

        scale *= 0.5;
    }

    if (m_levels.isEmpty()) {
        m_baseScale = 1.0;
    }
    appendPyramidLevel(baseImage);

    scale = 0.5;
    while (true) {
        QSize scaledSize = m_originalSize * scale;

        if (scaledSize.width() == 0 ||
                scaledSize.height() == 0) break;

        appendPyramidLevel(baseImage.scaled(scaledSize,  Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

        scale *= 0.5;
    }
}

KisQImagePyramid::~KisQImagePyramid()
{
}

int KisQImagePyramid::findNearestLevel(qreal scale, qreal *baseScale)
{
    const qreal scale_epsilon = 1e-6;

    qreal levelScale = m_baseScale;
    int level = 0;
    int lastLevel = m_levels.size() - 1;


    while ((0.5 * levelScale > scale ||
            qAbs(0.5 * levelScale - scale) < scale_epsilon) &&
            level < lastLevel) {

        levelScale *= 0.5;
        level++;
    }

    *baseScale = levelScale;
    return level;
}

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

    {
        QRect testingRect = roundRect(transform.mapRect(baseBounds));
        if (testingRect != expectedDstRect) {
            qWarning() << "WARNING: expected and real dab rects do not coincide!";
            qWarning() << "         expected rect:" << expectedDstRect;
            qWarning() << "         real rect:    " << testingRect;
        }
    }

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

void KisQImagePyramid::appendPyramidLevel(const QImage &image)
{
    /**
     * QPainter has a bug: when doing a transformation it decides that
     * all the pixels outside of the image (source rect) are equal to
     * the border pixels (CLAMP in terms of openGL). This means that
     * there will be no smooth scaling on the border of the image when
     * it is rotated.  To workaround this bug we need to add one pixel
     * wide border to the image, so that it transforms smoothly.
     *
     * See a unittest in: KisBrushTest::testQPainterTransformationBorder
     */
    QSize levelSize = image.size();
    QImage tmp = image.convertToFormat(QImage::Format_ARGB32);
    tmp = tmp.copy(-QPAINTER_WORKAROUND_BORDER,
                   -QPAINTER_WORKAROUND_BORDER,
                   image.width() + 2 * QPAINTER_WORKAROUND_BORDER,
                   image.height() + 2 * QPAINTER_WORKAROUND_BORDER);
    m_levels.append(PyramidLevel(tmp, levelSize));
}

QImage KisQImagePyramid::createImage(qreal scale, qreal rotation,
                                     qreal subPixelX, qreal subPixelY)
{
    qreal baseScale = -1.0;
    int level = findNearestLevel(scale, &baseScale);

    const QImage &srcImage = m_levels[level].image;

    QTransform transform;
    QSize dstSize;

    calculateParams(scale, rotation, subPixelX, subPixelY,
                    m_originalSize, baseScale, m_levels[level].size,
                    &transform, &dstSize);

    if (transform.isIdentity() &&
            srcImage.format() == QImage::Format_ARGB32) {

        return srcImage.copy(QPAINTER_WORKAROUND_BORDER,
                             QPAINTER_WORKAROUND_BORDER,
                             srcImage.width() - 2 * QPAINTER_WORKAROUND_BORDER,
                             srcImage.height() - 2 * QPAINTER_WORKAROUND_BORDER);
    }

    QImage dstImage(dstSize, QImage::Format_ARGB32);
    dstImage.fill(0);


    /**
     * QPainter has one more bug: when a QTransform is TxTranslate, it
     * does wrong sampling (probably, Nearest Neighbour) even though
     * we tell it directly that we need SmoothPixmapTransform.
     *
     * So here is a workaround: we set a negligible scale to convince
     * Qt we use a non-only-translating transform.
     */
    while (transform.type() == QTransform::TxTranslate) {
        const qreal scale = transform.m11();
        const qreal fakeScale = scale - 10 * std::numeric_limits<qreal>::epsilon();
        transform *= QTransform::fromScale(fakeScale, fakeScale);
    }

    QPainter gc(&dstImage);
    gc.setTransform(
        QTransform::fromTranslate(-QPAINTER_WORKAROUND_BORDER,
                                  -QPAINTER_WORKAROUND_BORDER) * transform);
    gc.setRenderHints(QPainter::SmoothPixmapTransform);
    gc.drawImage(QPointF(), srcImage);
    gc.end();

    return dstImage;
}

