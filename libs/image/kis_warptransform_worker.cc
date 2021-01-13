/*
 *  kis_warptransform_worker.cc -- part of Krita
 *
 *  SPDX-FileCopyrightText: 2010 Marc Pegon <pe.marc@free.fr>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_warptransform_worker.h"
#include "kis_random_sub_accessor.h"
#include "kis_iterator_ng.h"
#include "kis_datamanager.h"

#include <QTransform>
#include <QVector2D>
#include <QPainter>
#include <QVarLengthArray>

#include <KoColorSpace.h>
#include <KoColor.h>

#include <math.h>

#include "kis_grid_interpolation_tools.h"

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

KisWarpTransformWorker::KisWarpTransformWorker(WarpType warpType, QVector<QPointF> origPoint, QVector<QPointF> transfPoint, qreal alpha, KoUpdater *progress)
        : m_progress(progress)
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
        m_warpMathFunction = 0;
        break;
    }
}

KisWarpTransformWorker::~KisWarpTransformWorker()
{
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

void KisWarpTransformWorker::run(KisPaintDeviceSP srcDev, KisPaintDeviceSP dstDev)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(*srcDev->colorSpace() == *dstDev->colorSpace());

    if (!m_warpMathFunction ||
        m_origPoint.isEmpty() ||
        m_origPoint.size() != m_transfPoint.size()) {

        return;
    }

    if (m_origPoint.size() == 1) {
        dstDev->makeCloneFromRough(srcDev, srcDev->extent());
        QPointF translate(QPointF(srcDev->x(), srcDev->y()) + m_transfPoint[0] - m_origPoint[0]);
        dstDev->moveTo(translate.toPoint());
        return;
    }

    const QRect srcBounds = srcDev->region().boundingRect();

    dstDev->clear();

    const int pixelPrecision = 8;

    FunctionTransformOp functionOp(m_warpMathFunction, m_origPoint, m_transfPoint, m_alpha);
    GridIterationTools::PaintDevicePolygonOp polygonOp(srcDev, dstDev);
    GridIterationTools::processGrid(polygonOp, functionOp,
                                    srcBounds, pixelPrecision);
}

#include "krita_utils.h"

QRect KisWarpTransformWorker::approxChangeRect(const QRect &rc)
{
    const qreal margin = 0.05;

    FunctionTransformOp functionOp(m_warpMathFunction, m_origPoint, m_transfPoint, m_alpha);
    QRect resultRect = KisAlgebra2D::approximateRectWithPointTransform(rc, functionOp);

    return KisAlgebra2D::blowRect(resultRect, margin);
}

QRect KisWarpTransformWorker::approxNeedRect(const QRect &rc, const QRect &fullBounds)
{
    Q_UNUSED(rc);
    return fullBounds;
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
    default:
        KIS_ASSERT_RECOVER(0 && "Unknown warp mode") { return QImage(); }
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

    const QRectF srcBounds = QRectF(srcQImageOffset, srcImage.size());
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
    *newOffset = dstQImageOffset;

    QRect dstBoundsI = dstBounds.toAlignedRect();
    QImage dstImage(dstBoundsI.size(), srcImage.format());
    dstImage.fill(0);

    const int pixelPrecision = 32;
    GridIterationTools::QImagePolygonOp polygonOp(srcImage, dstImage, srcQImageOffset, dstQImageOffset);
    GridIterationTools::processGrid(polygonOp, functionOp, srcBounds.toAlignedRect(), pixelPrecision);

    return dstImage;
}
