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
#include <klocale.h>

#include <QTransform>

#include <KoProgressUpdater.h>
#include <KoColorSpace.h>
#include <KoCompositeOp.h>
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
    dev->move(dev->x() - 1, dev->y());
    return QRect(- r.top() - r.height(), r.x(), r.height(), r.width());
}

QRect KisTransformWorker::rotateLeft90(KisPaintDeviceSP dev,
                                       QRect boundRect,
                                       KoUpdaterPtr progressUpdater,
                                       int portion)
{
    QRect r = rotateWithTf(270, dev, boundRect, progressUpdater, portion);
    dev->move(dev->x(), dev->y() - 1);
    return QRect(r.top(), - r.x() - r.width(), r.height(), r.width());
}

QRect KisTransformWorker::rotate180(KisPaintDeviceSP dev,
                                    QRect boundRect,
                                    KoUpdaterPtr progressUpdater,
                                    int portion)
{
    QRect r = rotateWithTf(180, dev, boundRect, progressUpdater, portion);
    dev->move(dev->x() - 1, dev->y() -1);
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
    /* Check for nonsense and let the user know, this helps debugging.
    Otherwise the program will crash at a later point, in a very obscure way, probably by division by zero */
    Q_ASSERT_X(m_xscale != 0, "KisTransformer::run() validation step", "xscale == 0");
    Q_ASSERT_X(m_yscale != 0, "KisTransformer::run() validation step", "yscale == 0");
    // Fallback safety line in case Krita is compiled without ASSERTS
    if (m_xscale == 0 || m_yscale == 0) return false;

    m_boundRect = m_dev->exactBounds();

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
            m_dev->move(m_dev->x(), m_dev->y() + dy);
        } else if (yShearPresent) {
            transformPass <KisVLineIteratorSP>(m_dev.data(), m_dev.data(), yscale, m_yshear, dy, m_filter, portion);
            m_boundRect.translate(dx, 0);
            m_dev->move(m_dev->x() + dx, m_dev->y());
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
        m_dev->move(m_dev->x() + xtranslate, m_dev->y() + ytranslate);
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

    return true;
}

QRect KisTransformWorker::mirrorX(KisPaintDeviceSP dev, qreal axis, const KisSelection* selection)
{

    int pixelSize = dev->pixelSize();
    KisPaintDeviceSP dst = new KisPaintDevice(dev->colorSpace());

    QRect r;

    if (selection) {
        r = selection->selectedExactRect();
    } else {
        r = dev->exactBounds();

        if (axis > 0 && !r.isEmpty()) {
            // Extend rect so it has the same width on both sides of the axis
            qreal distanceFromAxis = qMax(fabs((qreal)r.left() - axis), fabs((qreal)r.right() - axis));
            QRect newRect(floor(axis - distanceFromAxis), r.y(), ceil(2*distanceFromAxis), r.height());
            r = newRect;
        }
    }

    if (r.width() <= 1) return r;

    {
        quint8 *dstPixels = new quint8[r.width() * pixelSize];

        KisHLineConstIteratorSP srcIt = dev->createHLineConstIteratorNG(r.x(), r.top(), r.width());

        KisHLineConstIteratorSP selIt;
        if (selection) {
            selIt = selection->projection()->createHLineConstIteratorNG(r.x(), r.top(), r.width());
        }

        for (qint32 y = r.top(); y <= r.bottom(); ++y) {

            quint8 *dstIt = dstPixels + (r.width() * pixelSize) - pixelSize;

            do {
                if (selIt) {
                    if (*selIt->oldRawData() > SELECTION_THRESHOLD) {
                        memcpy(dstIt, srcIt->oldRawData(), pixelSize);
                        selIt->nextPixel();
                    }
                }
                else {
                    memcpy(dstIt, srcIt->oldRawData(), pixelSize);
                }
                dstIt -= pixelSize;

            } while (srcIt->nextPixel());

            dst->writeBytes(dstPixels, QRect(r.left(), y, r.width(), 1));
            srcIt->nextRow();
        }

        delete[] dstPixels;
    }
    KisPainter gc(dev);

    if (selection) {
        dev->clearSelection(const_cast<KisSelection*>(selection));
    }
    else {
        dev->clear(r);
    }
    gc.setCompositeOp(COMPOSITE_OVER);
    gc.bitBlt(r.topLeft(), dst, r);

    return r;
}

QRect KisTransformWorker::mirrorY(KisPaintDeviceSP dev, qreal axis, const KisSelection* selection)
{
    int pixelSize = dev->pixelSize();
    KisPaintDeviceSP dst = new KisPaintDevice(dev->colorSpace());

    /* Read a line from bottom to top and and from top to bottom and write their values to each other */
    QRect r;
    if (selection) {
        r = selection->selectedExactRect();
    } else {
        r = dev->exactBounds();

        if (axis > 0) {
            // Extend rect so it has the same height on both sides of the axis
            qreal distanceFromAxis = qMax(fabs((qreal)r.top() - axis), fabs((qreal)r.bottom() - axis));
            QRect newRect(r.x(), floor(axis - distanceFromAxis), r.width(), ceil(2*distanceFromAxis));
            r = newRect;
        }
    }
    {
        qint32 y1, y2;
        for (y1 = r.top(), y2 = r.bottom(); y1 <= r.bottom(); ++y1, --y2) {

            KisHLineConstIteratorSP itTop = dev->createHLineConstIteratorNG(r.x(), y1, r.width());
            KisHLineIteratorSP itBottom = dst->createHLineIteratorNG(r.x(), y2, r.width());
            KisHLineConstIteratorSP selIt;
            if (selection) {
                selIt = selection->projection()->createHLineConstIteratorNG(r.x(), r.top(), r.width());
            }
            do {
                if (selIt) {
                    if (*selIt->oldRawData() > SELECTION_THRESHOLD) {
                        memcpy(itBottom->rawData(), itTop->oldRawData(), pixelSize);
                        selIt->nextPixel();
                    }
                }
                else {
                    memcpy(itBottom->rawData(), itTop->oldRawData(), pixelSize);
                }
            } while (itTop->nextPixel() && itBottom->nextPixel());
        }
    }
    KisPainter gc(dev);

    if (selection) {
        dev->clearSelection(const_cast<KisSelection*>(selection));
    }
    else {
        dev->clear(r);
    }
    gc.setCompositeOp(COMPOSITE_OVER);
    gc.bitBlt(r.topLeft(), dst, r);

    return r;

}

void KisTransformWorker::offset(KisPaintDeviceSP device, const QPoint& offsetPosition, const QSize& wrapSize)
{
    // inspired by gimp offset code, only wrap mode supported
    int width = wrapSize.width();
    int height = wrapSize.height();

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
    KisPainter gc(offsetDevice);
    gc.setCompositeOp(COMPOSITE_COPY);

    int srcX = 0;
    int srcY = 0;
    int destX = offsetX;
    int destY = offsetY;

    width = qBound<int>(0, width - offsetX, width);
    height = qBound<int>(0, height - offsetY, height);


    if ((width != 0) && (height != 0))
    {
        gc.bitBlt(destX, destY, device, srcX, srcY, width, height);
    }

    srcX = wrapSize.width() - offsetX;
    srcY = wrapSize.height() - offsetY;

    destX = (srcX + offsetX) % wrapSize.width();
    destY = (srcY + offsetY) % wrapSize.height();

    if (offsetX != 0 && offsetY != 0)
    {
          gc.bitBlt(destX, destY, device, srcX, srcY, offsetX, offsetY);
    }

    if (offsetX != 0)
    {
        gc.bitBlt(destX, destY + offsetY, device, srcX, 0, offsetX, wrapSize.height() - offsetY);
    }

    if (offsetY != 0)
    {
        gc.bitBlt(destX + offsetX, destY, device, 0, srcY, wrapSize.width() - offsetX, offsetY);
    }

    gc.end();

    // bitblt the result back
    KisPainter gc2(device);
    gc2.setCompositeOp(COMPOSITE_COPY);
    gc2.bitBlt(0,0,offsetDevice, 0, 0, wrapSize.width(), wrapSize.height());
    gc2.end();
}


