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
#include "krita_export.h"
#include "kis_paint_device.h"

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
	typedef enum WarpType_ {AFFINE_TRANSFORM = 0, SIMILITUDE_TRANSFORM, RIGID_TRANSFORM} WarpType;

    static QPointF affineTransformMath(QPointF v, QVector<QPointF> p, QVector<QPointF> q, qreal alpha);
    static QPointF similitudeTransformMath(QPointF v, QVector<QPointF> p, QVector<QPointF> q, qreal alpha);
    static QPointF rigidTransformMath(QPointF v, QVector<QPointF> p, QVector<QPointF> q, qreal alpha);
    // Convenience method : calls one of the 3 math function above depending on warpType
    static QPointF transformMath(WarpType warpType, QPointF v, QVector<QPointF> p, QVector<QPointF> q, qreal alpha);
    // Puts in dst the transformed quad pDst interpolated using quad pSrc and pixels of image src
    static void quadInterpolation(QImage *src, QImage *dst, QPolygon pSrc, QPolygon pDst);
    static void quadInterpolation(KisPaintDeviceSP src, KisPaintDeviceSP dst, QPolygon pSrc, QPolygon pDst);

    // Apply the transform on a QImage
    static QImage transformation(WarpType warpType, QImage *src, QVector<QPointF> origPoint, QVector<QPointF> transfPoint, qreal alpha, QPointF originalTopLeft, QPointF *newTopLeft);

    // Prepare the transformation on dev
    KisWarpTransformWorker(WarpType warpType, KisPaintDeviceSP dev, QVector<QPointF> origPoint, QVector<QPointF> transfPoint, qreal alpha, KoUpdater *progress);
    ~KisWarpTransformWorker();
    // Perform the prepated transformation
    void run();

private:
    typedef QPointF (*WarpMathFunction)(QPointF, QVector<QPointF>, QVector<QPointF>, qreal);

private:
    qreal m_progressTotalSteps;
    qreal m_lastProgressReport;
    qreal m_progressStep;
    WarpMathFunction m_warpMathFunction;
    QVector<QPointF> m_origPoint;
    QVector<QPointF> m_transfPoint;
    qreal m_alpha;
    KisPaintDeviceSP m_dev;
    KoUpdater *m_progress;

private:
    struct s_Fraction;
    typedef struct s_Fraction Fraction;

    class Side;

    struct s_ExtendedSide;
    typedef struct s_ExtendedSide ExtendedSide;

    enum ClipperSide_ {
        LEFTSIDE = 0, RIGHTSIDE = 1, UPSIDE = 2, DOWNSIDE = 3
    };
    typedef enum ClipperSide_ ClipperSide;

    static inline Side CalcSide(QPoint P1, QPoint P2, Side *next);
    static inline Side CalcSide(QPoint *P1, QPoint *P2, Side *next);
    static inline void AddExtSide(ExtendedSide **S, QPoint P0, QPoint P1);
    static void CreateExtSides(ExtendedSide **sides, QPolygon polygon);
    static inline void Insert(Side **L, Side *NewSide);
    static inline void InsertInSortedList(Side **L, Side *NewSide);
    static void FreeSidesList(Side *L);
    static void FreeSidesTable(Side *TC[], int top, int bottom);
    static void FreeExtSide(ExtendedSide *S);
    static void FreeExtSides(ExtendedSide **S);
    static inline void AddExtSide(ExtendedSide **dest, QPoint P, QPoint S, Side C);
    static inline void setRegion(bool reg[4], int x0, int y0, QRect clipRect);
    static void Sutherland_Hodgman(ExtendedSide **Dest, ExtendedSide *ExtSide, QRect clipRect, ClipperSide CS, bool &PreviousPointOut);
    static void ClipPolygone(ExtendedSide **ExtSides, QRect *clipper);
    static inline bool equals(qreal a, qreal b, qreal tolerance);
    static inline bool valInRange(qreal val, qreal range_min, qreal range_max, qreal tolerance);
    static int inverseBilinInterp(QVector2D p0_minus_p, QVector2D p1_minus_p, QVector2D p0_minus_p2, QVector2D p1_minus_p3, QPointF& sol1, QPointF& sol2);
    static inline void bilinInterp(QPointF p0, QPointF p1, QPointF p2, QPointF p3, QPointF st, QPointF& p);
    static inline qreal det(QVector2D v0, QVector2D v1);
    static inline void switchVertices(QPoint **a, QPoint **b);
    static QImage aux_transformation(WarpMathFunction warpMathFunction, QImage *src, QVector<QPointF> origPoint, QVector<QPointF> transfPoint, qreal alpha, QPointF originalTopLeft, QPointF *newTopLeft);
};

#endif
