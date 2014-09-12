/*
 *  kis_warptransform_worker.cc -- part of Krita
 *
 *  Copyright (c) 2010 Marc Pegon <pe.marc@free.fr>
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

#include "kis_warptransform_worker.h"
#include "kis_random_sub_accessor.h"
#include "kis_iterator_ng.h"
#include "kis_datamanager.h"

#include <algorithm>

#include <QTransform>
#include <QVector2D>
#include <QPainter>
#include <QVarLengthArray>

#include <KoProgressUpdater.h>
#include <KoColorSpace.h>
#include <KoColor.h>

#include <limits>
#include <math.h>

#include "kis_four_point_interpolator_backward.h"

QPointF KisWarpTransformWorker::affineTransformMath(QPointF v, QVector<QPointF> p, QVector<QPointF> q, qreal alpha)
{
    int nbPoints = p.size();
    QVarLengthArray<qreal> w(nbPoints);
    qreal sumWi = 0;
    QPointF pStar(0, 0), qStar(0, 0);
    QVarLengthArray<QPointF> pHat(nbPoints), qHat(nbPoints);

    for (int i = 0; i < nbPoints; ++i) {
        if (v == p[i])
            return q[i];

        QVector2D tmp(p[i] - v);
        w[i] = 1. / pow(tmp.lengthSquared(), alpha);
        pStar += w[i] * p[i];
        qStar += w[i] * q[i];
        sumWi += w[i];
    }
    pStar /= sumWi;
    qStar /= sumWi;

    qreal A_tmp[4] = {0, 0, 0, 0};
    for (int i = 0; i < nbPoints; ++i) {
        pHat[i] = p[i] - pStar;
        qHat[i] = q[i] - qStar;

        A_tmp[0] += w[i] * pow(pHat[i].x(), 2);
        A_tmp[3] += w[i] * pow(pHat[i].y(), 2);
        A_tmp[1] += w[i] * pHat[i].x() * pHat[i].y();
    }
    A_tmp[2] = A_tmp[1];
    qreal det_A_tmp = A_tmp[0] * A_tmp[3] - A_tmp[1] * A_tmp[2];

    qreal A_tmp_inv[4];

    if (det_A_tmp == 0)
        return v;

    A_tmp_inv[0] = A_tmp[3] / det_A_tmp;
    A_tmp_inv[1] = - A_tmp[1] / det_A_tmp;
    A_tmp_inv[2] = A_tmp_inv[1];
    A_tmp_inv[3] = A_tmp[0] / det_A_tmp;

    QPointF t = v - pStar;
    QPointF A_precalc(t.x() * A_tmp_inv[0] + t.y() * A_tmp_inv[1], t.x() * A_tmp_inv[2] + t.y() * A_tmp_inv[3]);
    qreal A_j;

    QPointF res = qStar;
    for (int j = 0; j < nbPoints; ++j) {
        A_j = A_precalc.x() * pHat[j].x() + A_precalc.y() * pHat[j].y();

        res += w[j] * A_j * qHat[j];
    }

    return res;
}

QPointF KisWarpTransformWorker::similitudeTransformMath(QPointF v, QVector<QPointF> p, QVector<QPointF> q, qreal alpha)
{
    int nbPoints = p.size();
    QVarLengthArray<qreal> w(nbPoints);
    qreal sumWi = 0;
    QPointF pStar(0, 0), qStar(0, 0);
    QVarLengthArray<QPointF> pHat(nbPoints), qHat(nbPoints);

    for (int i = 0; i < nbPoints; ++i) {
        if (v == p[i])
            return q[i];

        QVector2D tmp(p[i] - v);
        w[i] = 1. / pow(tmp.lengthSquared(), alpha);
        pStar += w[i] * p[i];
        qStar += w[i] * q[i];
        sumWi += w[i];
    }
    pStar /= sumWi;
    qStar /= sumWi;

    qreal mu_s = 0;
    QPointF res_tmp(0, 0);
    qreal qx, qy, px, py;
    for (int i = 0; i < nbPoints; ++i) {
        pHat[i] = p[i] - pStar;
        qHat[i] = q[i] - qStar;

        QVector2D tmp(pHat[i]);
        mu_s += w[i] * tmp.lengthSquared();

        qx = w[i] * qHat[i].x();
        qy = w[i] * qHat[i].y();
        px = pHat[i].x();
        py = pHat[i].y();

        res_tmp += QPointF(qx * px + qy * py, qx * py - qy * px);
    }

    res_tmp /= mu_s;
    QPointF v_m_pStar(v - pStar);
    QPointF res(res_tmp.x() * v_m_pStar.x() + res_tmp.y() * v_m_pStar.y(), res_tmp.x() * v_m_pStar.y() - res_tmp.y() * v_m_pStar.x());
    res += qStar;

    return res;
}

QPointF KisWarpTransformWorker::rigidTransformMath(QPointF v, QVector<QPointF> p, QVector<QPointF> q, qreal alpha)
{
    int nbPoints = p.size();
    QVarLengthArray<qreal> w(nbPoints);
    qreal sumWi = 0;
    QPointF pStar(0, 0), qStar(0, 0);
    QVarLengthArray<QPointF> pHat(nbPoints), qHat(nbPoints);

    for (int i = 0; i < nbPoints; ++i) {
        if (v == p[i])
            return q[i];

        QVector2D tmp(p[i] - v);
        w[i] = 1. / pow(tmp.lengthSquared(), alpha);
        pStar += w[i] * p[i];
        qStar += w[i] * q[i];
        sumWi += w[i];
    }
    pStar /= sumWi;
    qStar /= sumWi;

    QVector2D res_tmp(0, 0);
    qreal qx, qy, px, py;
    for (int i = 0; i < nbPoints; ++i) {
        pHat[i] = p[i] - pStar;
        qHat[i] = q[i] - qStar;

        qx = w[i] * qHat[i].x();
        qy = w[i] * qHat[i].y();
        px = pHat[i].x();
        py = pHat[i].y();

        res_tmp += QVector2D(qx * px + qy * py, qx * py - qy * px);
    }

    QPointF f_arrow(res_tmp.normalized().toPointF());
    QVector2D v_m_pStar(v - pStar);
    QPointF res(f_arrow.x() * v_m_pStar.x() + f_arrow.y() * v_m_pStar.y(), f_arrow.x() * v_m_pStar.y() - f_arrow.y() * v_m_pStar.x());
    res += qStar;

    return res;
}

KisWarpTransformWorker::KisWarpTransformWorker(WarpType warpType, KisPaintDeviceSP dev, QVector<QPointF> origPoint, QVector<QPointF> transfPoint, qreal alpha, KoUpdater *progress)
        : m_dev(dev), m_progress(progress)
{
    m_origPoint = origPoint;
    m_transfPoint = transfPoint;
    m_alpha = alpha;

    switch(warpType) {
    case AFFINE_TRANSFORM:
        m_warpMathFunction = &affineTransformMath;
        break;
    case SIMILITUDE_TRANSFORM:
        m_warpMathFunction = &similitudeTransformMath;
        break;
    case RIGID_TRANSFORM:
        m_warpMathFunction = &rigidTransformMath;
        break;
    default:
        m_warpMathFunction = NULL;
        break;
    }
}

KisWarpTransformWorker::~KisWarpTransformWorker()
{
}

template <class ProcessPolygon, class ForwardTransform>
void processPixels(ProcessPolygon &polygonOp, ForwardTransform &transformOp,
                   const QRect &srcBounds, const int pixelPrecision)
{
    if (srcBounds.isEmpty()) return;

    const int alignmentMask = ~(pixelPrecision - 1);

    QVector<QPointF> prevLinePoints;
    QVector<QPointF> currLinePoints;

    int prevRow = std::numeric_limits<int>::max();
    int prevCol = std::numeric_limits<int>::max();

    int rowIndex = 0;
    int colIndex = 0;

    for (int row = srcBounds.top(); row <= srcBounds.bottom();) {
        for (int col = srcBounds.left(); col <= srcBounds.right();) {

            QPointF dstPosF = transformOp(QPointF(col, row));
            currLinePoints << dstPosF;

            if (rowIndex >= 1 && colIndex >= 1) {

                QPolygonF srcPolygon;

                srcPolygon << QPointF(prevCol, prevRow);
                srcPolygon << QPointF(col, prevRow);
                srcPolygon << QPointF(col, row);
                srcPolygon << QPointF(prevCol, row);

                QPolygonF dstPolygon;

                dstPolygon << prevLinePoints.at(colIndex - 1);
                dstPolygon << prevLinePoints.at(colIndex);
                dstPolygon << currLinePoints.at(colIndex);
                dstPolygon << currLinePoints.at(colIndex - 1);

                polygonOp(srcPolygon, dstPolygon);
            }


            prevCol = col;
            col += pixelPrecision;
            colIndex++;

            if (col > srcBounds.right() &&
                col < srcBounds.right() + pixelPrecision - 1) {

                col = srcBounds.right();
            } else {
                col &= alignmentMask;
            }
        }

        std::swap(prevLinePoints, currLinePoints);

        // we are erasing elements for not free'ing the occupied
        // memory, which is more efficient since we are going to fill
        // the vector again
        currLinePoints.erase(currLinePoints.begin(), currLinePoints.end());
        colIndex = 0;

        prevRow = row;
        row += pixelPrecision;
        rowIndex++;

        if (row > srcBounds.bottom() &&
            row < srcBounds.bottom() + pixelPrecision - 1) {

            row = srcBounds.bottom();
        } else {
            row &= alignmentMask;
        }
    }
}

struct KisWarpTransformWorker::FunctionTransformOp
{
    FunctionTransformOp(KisWarpTransformWorker::WarpMathFunction function,
                        const QVector<QPointF> &p,
                        const QVector<QPointF> &q,
                        qreal alpha)
        : m_function(function),
          m_p(p),
          m_q(q),
          m_alpha(alpha)
    {
    }

    QPointF operator() (const QPointF &pt) const {
        return m_function(pt, m_p, m_q, m_alpha);
    }

    KisWarpTransformWorker::WarpMathFunction m_function;
    const QVector<QPointF> &m_p;
    const QVector<QPointF> &m_q;
    qreal m_alpha;
};

struct PaintDevicePolygonOp
{
    PaintDevicePolygonOp(KisPaintDeviceSP srcDev, KisPaintDeviceSP dstDev)
        : m_srcDev(srcDev), m_dstDev(dstDev) {}

    void operator() (const QPolygonF &srcPolygon, const QPolygonF &dstPolygon) {
        QRect boundRect = dstPolygon.boundingRect().toAlignedRect();
        KisSequentialIterator dstIt(m_dstDev, boundRect);
        KisRandomSubAccessorSP srcAcc = m_srcDev->createRandomSubAccessor();

        KisFourPointInterpolatorBackward interp(srcPolygon, dstPolygon);

        int y = boundRect.top();
        interp.setY(y);

        do {
            int newY = dstIt.y();

            if (y != newY) {
                y = newY;
                interp.setY(y);
            }

            QPointF srcPoint(dstIt.x(), y);

            if (dstPolygon.containsPoint(srcPoint, Qt::OddEvenFill)) {

                interp.setX(srcPoint.x());
                QPointF dstPoint = interp.getValue();

                // brain-blowing part:
                //
                // since the interpolator does the inverted
                // transfomation we read data from "dstPoint"
                // (which is non-transformed) and write it into
                // "srcPoint" (which is transformed position)

                srcAcc->moveTo(dstPoint);
                srcAcc->sampledOldRawData(dstIt.rawData());
            }

        } while (dstIt.nextPixel());

    }

    KisPaintDeviceSP m_srcDev;
    KisPaintDeviceSP m_dstDev;
};

struct QImagePolygonOp
{
    QImagePolygonOp(const QImage &srcImage, QImage &dstImage,
                    const QPointF &srcImageOffset,
                    const QPointF &dstImageOffset)
        : m_srcImage(srcImage), m_dstImage(dstImage),
          m_srcImageOffset(srcImageOffset),
          m_dstImageOffset(dstImageOffset),
          m_srcImageRect(m_srcImage.rect()),
          m_dstImageRect(m_dstImage.rect())
    {
    }

    void operator() (const QPolygonF &srcPolygon, const QPolygonF &dstPolygon) {
        QRect boundRect = dstPolygon.boundingRect().toAlignedRect();
        KisFourPointInterpolatorBackward interp(srcPolygon, dstPolygon);

        for (int y = boundRect.top(); y <= boundRect.bottom(); y++) {
            interp.setY(y);
            for (int x = boundRect.left(); x <= boundRect.right(); x++) {

                QPointF srcPoint(x, y);
                if (dstPolygon.containsPoint(srcPoint, Qt::OddEvenFill)) {

                    interp.setX(srcPoint.x());
                    QPointF dstPoint = interp.getValue();

                    // about srcPoint/dstPoint hell please see a
                    // comment in PaintDevicePolygonOp::operator() ()

                    srcPoint -= m_dstImageOffset;
                    dstPoint -= m_srcImageOffset;

                    QPoint srcPointI = srcPoint.toPoint();
                    QPoint dstPointI = dstPoint.toPoint();

                    srcPointI.rx() = qBound(m_dstImageRect.x(), srcPointI.x(), m_dstImageRect.right());
                    srcPointI.ry() = qBound(m_dstImageRect.y(), srcPointI.y(), m_dstImageRect.bottom());
                    dstPointI.rx() = qBound(m_srcImageRect.x(), dstPointI.x(), m_srcImageRect.right());
                    dstPointI.ry() = qBound(m_srcImageRect.y(), dstPointI.y(), m_srcImageRect.bottom());

                    m_dstImage.setPixel(srcPointI, m_srcImage.pixel(dstPointI));
                }
            }
        }
    }

    const QImage &m_srcImage;
    QImage &m_dstImage;
    QPointF m_srcImageOffset;
    QPointF m_dstImageOffset;

    QRect m_srcImageRect;
    QRect m_dstImageRect;
};

void KisWarpTransformWorker::run()
{

    if (!m_warpMathFunction ||
        m_origPoint.isEmpty() ||
        m_origPoint.size() != m_transfPoint.size()) {

        return;
    }

    KisPaintDeviceSP srcdev = new KisPaintDevice(*m_dev.data());

    if (m_origPoint.size() == 1) {
        QPointF translate(QPointF(m_dev->x(), m_dev->y()) + m_transfPoint[0] - m_origPoint[0]);
        m_dev->move(translate.toPoint());
        return;
    }

    const QRect srcBounds = srcdev->region().boundingRect();

    m_dev->clear();

    const int pixelPrecision = 8;

    FunctionTransformOp functionOp(m_warpMathFunction, m_origPoint, m_transfPoint, m_alpha);
    PaintDevicePolygonOp polygonOp(srcdev, m_dev);
    processPixels(polygonOp, functionOp,
                  srcBounds, pixelPrecision);
}

QImage KisWarpTransformWorker::transformQImage(WarpType warpType,
                                               const QVector<QPointF> &origPoint,
                                               const QVector<QPointF> &transfPoint,
                                               qreal alpha,
                                               const QImage& srcImage,
                                               const QPointF &srcQImageOffset,
                                               QPointF *newOffset)
{
    KIS_ASSERT_RECOVER(srcImage.format() == QImage::Format_ARGB32) {
        return QImage();
    }

    WarpMathFunction warpMathFunction = &rigidTransformMath;

    switch (warpType) {
    case AFFINE_TRANSFORM:
        warpMathFunction = &affineTransformMath;
        break;
    case SIMILITUDE_TRANSFORM:
        warpMathFunction = &similitudeTransformMath;
        break;
    case RIGID_TRANSFORM:
        warpMathFunction = &rigidTransformMath;
        break;
    }

    if (!warpMathFunction ||
        origPoint.isEmpty() ||
        origPoint.size() != transfPoint.size()) {

        return srcImage;
    }

    if (origPoint.size() == 1) {
        *newOffset = srcQImageOffset + (transfPoint[0] - origPoint[0]).toPoint();
        return srcImage;
    }

    FunctionTransformOp functionOp(warpMathFunction, origPoint, transfPoint, alpha);

    const QRect srcBounds = srcImage.rect();
    QRectF dstBounds;

    {
        QPolygonF testPoints;
        testPoints << srcBounds.topLeft();
        testPoints << srcBounds.topRight();
        testPoints << srcBounds.bottomRight();
        testPoints << srcBounds.bottomLeft();
        testPoints << srcBounds.topLeft();

        QPolygonF::iterator it = testPoints.begin() + 1;

        while (it != testPoints.end()) {
            it = testPoints.insert(it, 0.5 * (*it + *(it - 1)));
            it += 2;
        }

        it = testPoints.begin();

        while (it != testPoints.end()) {
            *it = functionOp(*it);
            ++it;
        }

        dstBounds = testPoints.boundingRect();
    }

    QPointF dstQImageOffset = dstBounds.topLeft();
    *newOffset = srcQImageOffset + dstQImageOffset;

    QRect dstBoundsI = dstBounds.toAlignedRect();
    QImage dstImage(dstBoundsI.size(), srcImage.format());
    dstImage.fill(128);

    QImagePolygonOp polygonOp(srcImage, dstImage, QPointF(), dstQImageOffset);

    const int pixelPrecision = 32;

    processPixels(polygonOp, functionOp,
                  srcBounds, pixelPrecision);

    return dstImage;
}
