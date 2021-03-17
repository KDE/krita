/*
 *  kis_warptransform_worker.h - part of Krita
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
    KisWarpTransformWorker(WarpType warpType, KisPaintDeviceSP dev, QVector<QPointF> origPoint, QVector<QPointF> transfPoint, qreal alpha, KoUpdater *progress);
    ~KisWarpTransformWorker() override;
    // Perform the prepared transformation
    void run();

    QRect approxChangeRect(const QRect &rc);
    QRect approxNeedRect(const QRect &rc, const QRect &fullBounds);

private:
    struct FunctionTransformOp;
    typedef QPointF (*WarpMathFunction)(QPointF, QVector<QPointF>, QVector<QPointF>, qreal);

private:
    WarpMathFunction m_warpMathFunction;
    WarpCalculation m_warpCalc {GRID};
    QVector<QPointF> m_origPoint;
    QVector<QPointF> m_transfPoint;
    
    KoUpdater *m_progress;
    qreal m_alpha {1.0};
    KisPaintDeviceSP m_dev;
    KoUpdater *m_progress {0};
};

#endif
