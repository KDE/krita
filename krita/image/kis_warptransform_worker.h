/*
 *  kis_warptransform_worker.h - part of Krita
 *
 *  Copyright (c) 2010 Marc Pegon
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

class KRITAIMAGE_EXPORT KisWarpTransformWorker : public QObject
{

    Q_OBJECT

public:
	static void quadInterpolation(QImage *src, QImage *dst, QPolygon pSrc, QPolygon pDst);
	static void quadInterpolation(KisPaintDeviceSP src, KisPaintDeviceSP dst, QPolygon pSrc, QPolygon pDst);
	static QImage affineTransformation(QImage *src, qint32 pointsPerLine, qint32 pointsPerColumn, QPointF *origPoint, QPointF *transfPoint, qreal alpha, QPointF originalTopLeft, QPointF *newTopLeft);

    KisWarpTransformWorker(KisPaintDeviceSP dev, qint32 pointsPerLine, qint32 pointsPerColumn, QPointF *origPoint, QPointF *transfPoint, qreal alpha, KoUpdater *progress);
    ~KisWarpTransformWorker();
    void run();

private:
    qreal m_progressTotalSteps;
    qreal m_lastProgressReport;
    qreal m_progressStep;
	qint32 m_pointsPerLine;
	qint32 m_pointsPerColumn;
	QPointF *m_origPoint;
	QPointF *m_transfPoint;
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
	static inline void switchVertices(QPoint **a, QPoint **b);
	static inline QPointF calcAffineTransformation(QPointF v, int nbPoints, QPointF *p, QPointF *q, qreal alpha);
};

#endif
