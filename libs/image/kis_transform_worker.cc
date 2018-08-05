/*
 *  Copyright (c) 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de> filters
 *  Copyright (c) 2005-2007 C. Boemann <cbo@boemann.dk>
 *  Copyright (c) 2005, 2010 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2010 Marc Pegon <pe.marc@free.fr>
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

#include "kis_transform_worker.h"

#include <qmath.h>
#include <klocalizedstring.h>

#include <QTransform>

#include <KoColorSpace.h>
#include <KoCompositeOpRegistry.h>
#include <KoColor.h>

#include "kis_paint_device.h"
#include "kis_debug.h"
#include "kis_selection.h"
#include "kis_iterator_ng.h"
#include "kis_random_accessor_ng.h"
#include "kis_filter_strategy.h"
#include "kis_painter.h"
#include "kis_filter_weights_applicator.h"
#include "kis_progress_update_helper.h"
#include "kis_pixel_selection.h"
#include "kis_image.h"


KisTransformWorker::KisTransformWorker(KisPaintDeviceSP dev,
                                       double xscale, double yscale,
                                       double xshear, double yshear,
                                       double xshearOrigin, double yshearOrigin,
                                       double rotation,
                                       qint32 xtranslate, qint32 ytranslate,
                                       KoUpdaterPtr progress,
                                       KisFilterStrategy *filter)
{
    m_dev = dev;
    m_xscale = xscale;
    m_yscale = yscale;
    m_xshear = xshear;
    m_yshear = yshear;
    m_xshearOrigin = xshearOrigin;
    m_yshearOrigin = yshearOrigin;
    m_rotation = rotation,
    m_xtranslate = xtranslate;
    m_ytranslate = ytranslate;
    m_progressUpdater = progress;
    m_filter = filter;
}

KisTransformWorker::~KisTransformWorker()
{
}

QTransform KisTransformWorker::transform() const
{
    QTransform TS = QTransform::fromTranslate(m_xshearOrigin, m_yshearOrigin);
    QTransform S; S.shear(0, m_yshear); S.shear(m_xshear, 0);
    QTransform SC = QTransform::fromScale(m_xscale, m_yscale);
    QTransform R; R.rotateRadians(m_rotation);
    QTransform T = QTransform::fromTranslate(m_xtranslate, m_ytranslate);

    return TS.inverted() * S * TS * SC * R * T;
}

void KisTransformWorker::transformPixelSelectionOutline(KisPixelSelectionSP pixelSelection) const
{
    if (pixelSelection->outlineCacheValid()) {
        QPainterPath outlineCache = pixelSelection->outlineCache();
        pixelSelection->setOutlineCache(transform().map(outlineCache));
    }
}

QRect rotateWithTf(int rotation, KisPaintDeviceSP dev,
                   QRect boundRect,
                   KoUpdaterPtr progressUpdater,
                   int portion)
{
    qint32 pixelSize = dev->pixelSize();
    QRect r(boundRect);

    KisPaintDeviceSP tmp = new KisPaintDevice(dev->colorSpace());
    tmp->prepareClone(dev);

    KisRandomAccessorSP devAcc = dev->createRandomAccessorNG(0, 0);
    KisRandomAccessorSP tmpAcc = tmp->createRandomAccessorNG(0, 0);
    KisProgressUpdateHelper progressHelper(progressUpdater, portion, r.height());

    QTransform tf;
    tf = tf.rotate(rotation);

    int ty = 0;
    int tx = 0;

    for (qint32 y = r.y(); y <= r.height() + r.y(); ++y) {
        for (qint32 x = r.x(); x <= r.width() + r.x(); ++x) {
            tf.map(x, y, &tx, &ty);
            devAcc->moveTo(x, y);
            tmpAcc->moveTo(tx, ty);

            memcpy(tmpAcc->rawData(), devAcc->rawData(), pixelSize);
        }
        progressHelper.step();
    }

    dev->makeCloneFrom(tmp, tmp->region().boundingRect());
    return r;
}

QRect KisTransformWorker::rotateRight90(KisPaintDeviceSP dev,
                                        QRect boundRect,
                                        KoUpdaterPtr progressUpdater,
                                        int portion)
{
    QRect r = rotateWithTf(90, dev, boundRect, progressUpdater, portion);
    dev->moveTo(dev->x() - 1, dev->y());
    return QRect(- r.top() - r.height(), r.x(), r.height(), r.width());
}

QRect KisTransformWorker::rotateLeft90(KisPaintDeviceSP dev,
                                       QRect boundRect,
                                       KoUpdaterPtr progressUpdater,
                                       int portion)
{
    QRect r = rotateWithTf(270, dev, boundRect, progressUpdater, portion);
    dev->moveTo(dev->x(), dev->y() - 1);
    return QRect(r.top(), - r.x() - r.width(), r.height(), r.width());
}

QRect KisTransformWorker::rotate180(KisPaintDeviceSP dev,
                                    QRect boundRect,
                                    KoUpdaterPtr progressUpdater,
                                    int portion)
{
    QRect r = rotateWithTf(180, dev, boundRect, progressUpdater, portion);
    dev->moveTo(dev->x() - 1, dev->y() -1);
    return QRect(- r.x() - r.width(), - r.top() - r.height(), r.width(), r.height());
}

template <class iter> void calcDimensions(QRect rc, qint32 &srcStart, qint32 &srcLen, qint32 &firstLine, qint32 &numLines);

template <> void calcDimensions <KisHLineIteratorSP>
(QRect rc, qint32 &srcStart, qint32 &srcLen, qint32 &firstLine, qint32 &numLines)
{
    srcStart = rc.x();
    srcLen = rc.width();
    firstLine = rc.y();
    numLines = rc.height();
}

template <> void calcDimensions <KisVLineIteratorSP>
(QRect rc, qint32 &srcStart, qint32 &srcLen, qint32 &firstLine, qint32 &numLines)
{
    srcStart = rc.y();
    srcLen = rc.height();
    firstLine = rc.x();
    numLines = rc.width();

}

template <class iter>
void updateBounds(QRect &boundRect,
                  const KisFilterWeightsApplicator::LinePos &newBounds);

template <>
void updateBounds<KisHLineIteratorSP>(QRect &boundRect, const KisFilterWeightsApplicator::LinePos &newBounds)
{
    boundRect.setLeft(newBounds.start());
    boundRect.setWidth(newBounds.size());
}

template <>
void updateBounds<KisVLineIteratorSP>(QRect &boundRect, const KisFilterWeightsApplicator::LinePos &newBounds)
{
    boundRect.setTop(newBounds.start());
    boundRect.setHeight(newBounds.size());
}

template <class T>
void KisTransformWorker::transformPass(KisPaintDevice *src, KisPaintDevice *dst,
                                       double floatscale, double shear, double dx,
                                       KisFilterStrategy *filterStrategy,
                                       int portion)
{
    bool clampToEdge = shear == 0.0;

    qint32 srcStart, srcLen, firstLine, numLines;
    calcDimensions<T>(m_boundRect, srcStart, srcLen, firstLine, numLines);

    KisProgressUpdateHelper progressHelper(m_progressUpdater, portion, numLines);
    KisFilterWeightsBuffer buf(filterStrategy, qAbs(floatscale));
    KisFilterWeightsApplicator applicator(src, dst, floatscale, shear, dx, clampToEdge);

    KisFilterWeightsApplicator::LinePos dstBounds;

    for (int i = firstLine; i < firstLine + numLines; i++) {
        KisFilterWeightsApplicator::LinePos dstPos;
        KisFilterWeightsApplicator::LinePos srcPos(srcStart, srcLen);

        dstPos = applicator.processLine<T>(srcPos, i, &buf, filterStrategy->support());
        dstBounds.unite(dstPos);

        progressHelper.step();
    }

    updateBounds<T>(m_boundRect, dstBounds);
}

template<typename T>
void swapValues(T *a, T *b) {
    T c = *a;
    *a = *b;
    *b = c;
}

bool KisTransformWorker::run()
{
    return runPartial(m_dev->exactBounds());
}

bool KisTransformWorker::runPartial(const QRect &processRect)
{
    /* Check for nonsense and let the user know, this helps debugging.
    Otherwise the program will crash at a later point, in a very obscure way, probably by division by zero */
    Q_ASSERT_X(m_xscale != 0, "KisTransformer::run() validation step", "xscale == 0");
    Q_ASSERT_X(m_yscale != 0, "KisTransformer::run() validation step", "yscale == 0");
    // Fallback safety line in case Krita is compiled without ASSERTS
    if (m_xscale == 0 || m_yscale == 0) return false;

    m_boundRect = processRect;

    if (m_boundRect.isNull()) {
        if (!m_progressUpdater.isNull()) {
            m_progressUpdater->setProgress(100);
        }
        return true;
    }

    double xscale = m_xscale;
    double yscale = m_yscale;
    double rotation = m_rotation;
    qint32 xtranslate = m_xtranslate;
    qint32 ytranslate = m_ytranslate;

    // Apply shearX/Y separately. In Krita it is demanded separately
    // most of the times.
    if (m_xshear != 0 || m_yshear != 0) {
        int portion = 50;

        int dx = - qRound(m_yshearOrigin * yscale * m_xshear);
        int dy = - qRound(m_xshearOrigin * xscale * m_yshear);

        bool scalePresent = !(qFuzzyCompare(xscale, 1.0) && qFuzzyCompare(yscale, 1.0));
        bool xShearPresent = !qFuzzyCompare(m_xshear, 0.0);
        bool yShearPresent = !qFuzzyCompare(m_yshear, 0.0);

        if (scalePresent || (xShearPresent && yShearPresent)) {
            transformPass <KisHLineIteratorSP>(m_dev.data(), m_dev.data(), xscale, yscale *  m_xshear, dx, m_filter, portion);
            transformPass <KisVLineIteratorSP>(m_dev.data(), m_dev.data(), yscale, m_yshear, dy, m_filter, portion);
        } else if (xShearPresent) {
            transformPass <KisHLineIteratorSP>(m_dev.data(), m_dev.data(), xscale, m_xshear, dx, m_filter, portion);
            m_boundRect.translate(0, dy);
            m_dev->moveTo(m_dev->x(), m_dev->y() + dy);
        } else if (yShearPresent) {
            transformPass <KisVLineIteratorSP>(m_dev.data(), m_dev.data(), yscale, m_yshear, dy, m_filter, portion);
            m_boundRect.translate(dx, 0);
            m_dev->moveTo(m_dev->x() + dx, m_dev->y());
        }

        yscale = 1.;
        xscale = 1.;
    }

    if (rotation < 0.0) {
        rotation = -fmod(-rotation, 2 * M_PI) + 2 * M_PI;
    } else {
        rotation = fmod(rotation, 2 * M_PI);
    }

    int rotQuadrant = int(rotation / (M_PI / 2) + 0.5) & 3;
    rotation -= rotQuadrant * M_PI / 2;

    bool simpleTranslation =
        qFuzzyCompare(rotation, 0.0) &&
        qFuzzyCompare(xscale, 1.0) &&
        qFuzzyCompare(yscale, 1.0);


    int progressTotalSteps = qMax(1, 2 * (!simpleTranslation) + (rotQuadrant != 0));
    int progressPortion = 100 / progressTotalSteps;

    /**
     * Pre-rotate the image to ensure the actual resampling is done
     * for an angle -pi/4...pi/4. This is faster and produces better
     * quality.
     */
    switch (rotQuadrant) {
    case 1:
        swapValues(&xscale, &yscale);
        m_boundRect = rotateRight90(m_dev, m_boundRect, m_progressUpdater, progressPortion);
        break;
    case 2:
        m_boundRect = rotate180(m_dev, m_boundRect, m_progressUpdater, progressPortion);
        break;
    case 3:
        swapValues(&xscale, &yscale);
        m_boundRect = rotateLeft90(m_dev, m_boundRect, m_progressUpdater, progressPortion);
        break;
    default:
        /* do nothing */
        break;
    }

    if (simpleTranslation) {
        m_boundRect.translate(xtranslate, ytranslate);
        m_dev->moveTo(m_dev->x() + xtranslate, m_dev->y() + ytranslate);
    } else {
        QTransform SC = QTransform::fromScale(xscale, yscale);
        QTransform R; R.rotateRadians(rotation);
        QTransform T = QTransform::fromTranslate(xtranslate, ytranslate);
        QTransform m = SC * R * T;

        /**
         * First X-pass, then Y-pass
         *     | a 0 0 |   | 1 d 0 |
         * m = | b 1 0 | x | 0 e 0 | (matrices are in Qt's notation)
         *     | c 0 1 |   | 0 f 1 |
         */
        qreal a = m.m11();
        qreal b = m.m21();
        qreal c = m.m31();
        qreal d = m.m12() / m.m11();
        qreal e = m.m22() - m.m21() * m.m12() / m.m11();
        qreal f = m.m32() - m.m31() * m.m12() / m.m11();

        // First Pass (X)
        transformPass <KisHLineIteratorSP>(m_dev.data(), m_dev.data(), a, b, c, m_filter, progressPortion);

        // Second Pass (Y)
        transformPass <KisVLineIteratorSP>(m_dev.data(), m_dev.data(), e, d, f, m_filter, progressPortion);

#if 0
        /************************************************************/
        /**
         * First Y-pass, then X-pass (for testing purposes)
         *     | 1 d 0 |   | a 0 0 |
         * m = | 0 e 0 | x | b 1 0 | (matrices are in Qt's notation)
         *     | 0 f 1 |   | c 0 1 |
         */
        qreal a = m.m11() - m.m21() * m.m12() / m.m22();
        qreal b = m.m21() / m.m22();
        qreal c = m.m31() - m.m21() * m.m32() / m.m22();
        qreal d = m.m12();
        qreal e = m.m22();
        qreal f = m.m32();
        // First Pass (X)
        transformPass <KisHLineIteratorSP>(m_dev.data(), m_dev.data(), a, b, c, m_filter, progressPortion);
        // Second Pass (Y)
        transformPass <KisVLineIteratorSP>(m_dev.data(), m_dev.data(), e, d, f, m_filter, progressPortion);
        /************************************************************/
#endif /* 0 */

#if 0
        /************************************************************/
        // Old three-pass implementation (for testing purposes)
        yshear = sin(rotation);
        xshear = -tan(rotation / 2);
        xtranslate -= int(xshear * ytranslate);

        transformPass <KisHLineIteratorSP>(m_dev.data(), m_dev.data(), xscale, yscale*xshear, 0, m_filter, 0);
        transformPass <KisVLineIteratorSP>(m_dev.data(), m_dev.data(), yscale, yshear, ytranslate, m_filter, 0);
        if (xshear != 0.0) {
            transformPass <KisHLineIteratorSP>(m_dev.data(), m_dev.data(), 1.0, xshear, xtranslate, m_filter, 0);
        } else {
            m_dev->move(m_dev->x() + xtranslate, m_dev->y());
            updateBounds <KisHLineIteratorSP>(m_boundRect, 1.0, 0, xtranslate);
        }
        /************************************************************/
#endif /* 0 */

    }

    if (!m_progressUpdater.isNull()) {
        m_progressUpdater->setProgress(100);
    }

    /**
     * Purge the tiles which might be left after scaling down the
     * image
     */
    m_dev->purgeDefaultPixels();

    return true;
}

void mirror_impl(KisPaintDeviceSP dev, qreal axis, bool isHorizontal)
{
    KIS_ASSERT_RECOVER_RETURN(qFloor(axis) == axis || (axis - qFloor(axis) == 0.5));

    QRect mirrorRect = dev->exactBounds();
    if (mirrorRect.width() <= 1) return;

    /**
     * We split the total mirror rect into two halves, which lay to
     * the 'left' and 'right' from the axis. Effectively, these halves
     * should be swapped, but there is a bit of optimization: some
     * parts of these portions overlap and some don't. So former ones
     * should be really swapped, but the latter ones just moved to the
     * other side.
     *
     * So the algorithm consists of two stages:
     *
     * 1) Move the non-overlapping portion of the mirror rect to the
     *    other side of the axis. The move may be either left-to-right or
     *    right-to-left.
     *
     * 2) Use slow 'swap' operation for the remaining portion of the
     *    mirrorRect.
     *
     * NOTE: the algorithm works with (column, row) coordinates which
     *       are mapped to the real (x, y) depending on the value of
     *       'isHorizontal' parameter.
     */

    int leftStart;
    int rightEnd;

    if (isHorizontal) {
        leftStart = mirrorRect.x();
        rightEnd = mirrorRect.x() + mirrorRect.width();
    } else {
        leftStart = mirrorRect.y();
        rightEnd = mirrorRect.y() + mirrorRect.height();
    }

    /**
     * If the axis is not aligned, that is crosses some pixel cell, we should just skip this
     * column and not process it. Actually, how can we mirror the central single-pixel column?
     */
    const bool axisNonAligned = qFloor(axis) < axis;

    int leftCenterPoint = qFloor(axis);
    int leftEnd = qMin(leftCenterPoint, rightEnd);

    int rightCenterPoint = axisNonAligned ? qCeil(axis) : qFloor(axis);
    int rightStart = qMax(rightCenterPoint, leftStart);

    int leftSize = qMax(0, leftEnd - leftStart);
    int rightSize = qMax(0, rightEnd - rightStart);

    int maxDistanceToAxis = qMax(leftCenterPoint - leftStart,
                           rightEnd - rightCenterPoint);


    // Main variables for controlling the stages of the algorithm
    bool moveLeftToRight = leftSize > rightSize;
    int moveAmount = qAbs(leftSize - rightSize);
    int swapAmount = qMin(leftSize, rightSize);

    // Initial position of 'left' and 'right' block iterators
    int initialLeftCol = leftCenterPoint - maxDistanceToAxis;
    int initialRightCol = rightCenterPoint + maxDistanceToAxis - 1;


    KisRandomAccessorSP leftIt = dev->createRandomAccessorNG(mirrorRect.x(), mirrorRect.y());
    KisRandomAccessorSP rightIt = dev->createRandomAccessorNG(mirrorRect.x(), mirrorRect.y());
    const KoColor defaultPixelObject = dev->defaultPixel();
    const quint8 *defaultPixel = defaultPixelObject.data();

    const int pixelSize = dev->pixelSize();
    QByteArray buf(pixelSize, 0);

    // Map (column, row) -> (x, y)
    int rowsRemaining;
    int row;

    if (isHorizontal) {
        rowsRemaining = mirrorRect.height();
        row = mirrorRect.y();
    } else {
        rowsRemaining = mirrorRect.width();
        row = mirrorRect.x();
    }

    int leftColPos = 0;
    int rightColPos = 0;

    const int &leftX = isHorizontal ? leftColPos : row;
    const int &leftY = isHorizontal ? row : leftColPos;

    const int &rightX = isHorizontal ? rightColPos : row;
    const int &rightY = isHorizontal ? row : rightColPos;

    while (rowsRemaining) {
        leftColPos = initialLeftCol;
        rightColPos = initialRightCol;

        int rows = qMin(rowsRemaining, isHorizontal ? leftIt->numContiguousRows(leftY) : leftIt->numContiguousColumns(leftX));
        int rowStride = isHorizontal ? leftIt->rowStride(leftX, leftY) : pixelSize;

        if (moveLeftToRight) {
            for (int i = 0; i < moveAmount; i++) {
                leftIt->moveTo(leftX, leftY);
                rightIt->moveTo(rightX, rightY);

                quint8 *leftPtr = leftIt->rawData();
                quint8 *rightPtr = rightIt->rawData();

                for (int j = 0; j < rows; j++) {
                    // left-to-right move
                    memcpy(rightPtr, leftPtr, pixelSize);
                    memcpy(leftPtr, defaultPixel, pixelSize);

                    leftPtr += rowStride;
                    rightPtr += rowStride;
                }

                leftColPos++;
                rightColPos--;
            }
        } else {
            for (int i = 0; i < moveAmount; i++) {
                leftIt->moveTo(leftX, leftY);
                rightIt->moveTo(rightX, rightY);

                quint8 *leftPtr = leftIt->rawData();
                quint8 *rightPtr = rightIt->rawData();

                for (int j = 0; j < rows; j++) {
                    // right-to-left move
                    memcpy(leftPtr, rightPtr, pixelSize);
                    memcpy(rightPtr, defaultPixel, pixelSize);

                    leftPtr += rowStride;
                    rightPtr += rowStride;
                }

                leftColPos++;
                rightColPos--;
            }
        }

        for (int i = 0; i < swapAmount; i++) {
            leftIt->moveTo(leftX, leftY);
            rightIt->moveTo(rightX, rightY);

            quint8 *leftPtr = leftIt->rawData();
            quint8 *rightPtr = rightIt->rawData();

            for (int j = 0; j < rows; j++) {
                // swap operation
                memcpy(buf.data(), leftPtr, pixelSize);
                memcpy(leftPtr, rightPtr, pixelSize);
                memcpy(rightPtr, buf.data(), pixelSize);

                leftPtr += rowStride;
                rightPtr += rowStride;
            }

            leftColPos++;
            rightColPos--;
        }

        rowsRemaining -= rows;
        row += rows;
    }
}

void KisTransformWorker::mirrorX(KisPaintDeviceSP dev, qreal axis)
{
    mirror_impl(dev, axis, true);
}

void KisTransformWorker::mirrorY(KisPaintDeviceSP dev, qreal axis)
{
    mirror_impl(dev, axis, false);
}

void KisTransformWorker::mirrorX(KisPaintDeviceSP dev)
{
    QRect bounds = dev->exactBounds();
    mirrorX(dev, bounds.x() + 0.5 * bounds.width());
}

void KisTransformWorker::mirrorY(KisPaintDeviceSP dev)
{
    QRect bounds = dev->exactBounds();
    mirrorY(dev, bounds.y() + 0.5 * bounds.height());
}

void KisTransformWorker::mirror(KisPaintDeviceSP dev, qreal axis, Qt::Orientation orientation)
{
    mirror_impl(dev, axis, orientation == Qt::Horizontal);
}

void KisTransformWorker::offset(KisPaintDeviceSP device, const QPoint& offsetPosition, const QRect& wrapRect)
{
    Q_ASSERT(wrapRect == wrapRect.normalized());

    // inspired by gimp offset code, only wrap mode supported
    int sx = wrapRect.x();
    int sy = wrapRect.y();

    int width = wrapRect.width();
    int height = wrapRect.height();

    // offset coords are relative to space wrapRect
    int offsetX = offsetPosition.x();
    int offsetY = offsetPosition.y();

    while (offsetX < 0)
    {
        offsetX += width;
    }

    while (offsetY < 0)
    {
        offsetY += height;
    }

    if ((offsetX == 0) && (offsetY == 0))
    {
        return;
    }

    KisPaintDeviceSP offsetDevice = new KisPaintDevice(device->colorSpace());

    int srcX = 0;
    int srcY = 0;

    int destX = offsetX;
    int destY = offsetY;

    width = qBound<int>(0, width - offsetX, width);
    height = qBound<int>(0, height - offsetY, height);

    if ((width != 0) && (height != 0)) {
        // convert back to paint device space
        KisPainter::copyAreaOptimized(QPoint(destX + sx, destY + sy), device, offsetDevice, QRect(srcX + sx, srcY + sy, width, height));
    }

    srcX = wrapRect.width() - offsetX;
    srcY = wrapRect.height() - offsetY;

    destX = (srcX + offsetX) % wrapRect.width();
    destY = (srcY + offsetY) % wrapRect.height();

    if (offsetX != 0 && offsetY != 0) {
          KisPainter::copyAreaOptimized(QPoint(destX + sx, destY + sy), device, offsetDevice, QRect(srcX + sx, srcY + sy, offsetX, offsetY));
    }

    if (offsetX != 0) {
        KisPainter::copyAreaOptimized(QPoint(destX + sx, (destY + offsetY) + sy), device, offsetDevice, QRect(srcX + sx, 0 + sy, offsetX, wrapRect.height() - offsetY));
    }

    if (offsetY != 0) {
        KisPainter::copyAreaOptimized(QPoint((destX + offsetX) + sx, destY + sy), device, offsetDevice, QRect(0 + sx, srcY + sy, wrapRect.width() - offsetX, offsetY));
    }

    // bitblt the result back
    QRect resultRect(sx, sy, wrapRect.width(), wrapRect.height());
    KisPainter::copyAreaOptimized(resultRect.topLeft(), offsetDevice, device, resultRect);
}


