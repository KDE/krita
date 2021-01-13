/*
 *  kis_warptransform_worker.h - part of Krita
 *
 *  SPDX-FileCopyrightText: 2010 Marc Pegon <pe.marc@free.fr>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_WARPTRANSFORM_WORKER_H
#define KIS_WARPTRANSFORM_WORKER_H

#include "kis_types.h"
#include "kritaimage_export.h"

#include <QImage>
#include <QPolygon>
#include <QPoint>
#include <QPointF>
#include <QRect>

#include <KoUpdater.h>

/**
 * Class to apply a transformation (affine, similitude, MLS) to a paintDevice
 * or a QImage according an original set of points p, a new set of points q,
 * and the constant alpha.
 * The algorithms are based a paper entitled "Image Deformation Using
 * Moving Least Squares", by Scott Schaefer (Texas A&M University), Travis
 * McPhail (Rice University) and Joe Warren (Rice University)
 */

class KRITAIMAGE_EXPORT KisWarpTransformWorker : public QObject
{
    Q_OBJECT

public:
    typedef enum WarpType_ {AFFINE_TRANSFORM = 0, SIMILITUDE_TRANSFORM, RIGID_TRANSFORM, N_MODES} WarpType;
    typedef enum WarpCalculation_ {GRID = 0, DRAW} WarpCalculation;

    static QPointF affineTransformMath(QPointF v, QVector<QPointF> p, QVector<QPointF> q, qreal alpha);
    static QPointF similitudeTransformMath(QPointF v, QVector<QPointF> p, QVector<QPointF> q, qreal alpha);
    static QPointF rigidTransformMath(QPointF v, QVector<QPointF> p, QVector<QPointF> q, qreal alpha);

    static QImage transformQImage(WarpType warpType,
                                  const QVector<QPointF> &origPoint,
                                  const QVector<QPointF> &transfPoint,
                                  qreal alpha,
                                  const QImage& srcImage,
                                  const QPointF &srcQImageOffset,
                                  QPointF *newOffset);

    // Prepare the transformation on dev
    KisWarpTransformWorker(WarpType warpType, QVector<QPointF> origPoint, QVector<QPointF> transfPoint, qreal alpha, KoUpdater *progress);
    ~KisWarpTransformWorker() override;
    // Perform the prepared transformation
    void run(KisPaintDeviceSP srcDev, KisPaintDeviceSP dstDev);

    QRect approxChangeRect(const QRect &rc);
    QRect approxNeedRect(const QRect &rc, const QRect &fullBounds);

private:
    struct FunctionTransformOp;
    typedef QPointF (*WarpMathFunction)(QPointF, QVector<QPointF>, QVector<QPointF>, qreal);

private:
    WarpMathFunction m_warpMathFunction;
    WarpCalculation m_warpCalc;
    QVector<QPointF> m_origPoint;
    QVector<QPointF> m_transfPoint;
    qreal m_alpha;
    KoUpdater *m_progress;
};

#endif
