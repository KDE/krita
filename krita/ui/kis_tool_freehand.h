/*
 *  kis_tool_brush.h - part of Krita
 *
 *  Copyright (c) 2003-2004 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KIS_TOOL_FREEHAND_H_
#define KIS_TOOL_FREEHAND_H_

#include "kis_types.h"
#include "kis_tool_paint.h"

#include "krita_export.h"

class KoPointerEvent;
class KoCanvasBase;

class KisPainter;

class FreehandPaintJobExecutor;
class FreehandPaintJob;
class KisRecordedPolyLinePaintAction;
class FreehandPaintJobExecutor;

class KRITAUI_EXPORT KisToolFreehand : public KisToolPaint
{

    Q_OBJECT

public:

    KisToolFreehand(KoCanvasBase * canvas, const QCursor & cursor, const QString & transactionText);
    virtual ~KisToolFreehand();

    virtual void mousePressEvent(KoPointerEvent *e);
    virtual void mouseMoveEvent(KoPointerEvent *e);
    virtual void mouseReleaseEvent(KoPointerEvent *e);

    virtual void setDirty(const QRegion& region);

protected:

    /// Paint a single brush footprint on the current layer
    virtual void paintAt(const KisPaintInformation &pi);

    /// Paint a line between the specified positions on the current layer
    virtual void paintLine(const KisPaintInformation &pi1,
                   const KisPaintInformation &pi2);
    virtual void paintBezierCurve(const KisPaintInformation &pi1,
                   const QPointF &control1,
                   const QPointF &control2,
                   const KisPaintInformation &pi2);

    virtual void initPaint(KoPointerEvent *e);
    virtual void endPaint();

    void paintOutline(const QPointF& point);
protected slots:
    void setSmooth(bool smooth);
private:
    void queuePaintJob(FreehandPaintJob* job, FreehandPaintJob* previousJob);
protected:

    KisPaintInformation m_previousPaintInformation;
    QPointF m_previousTangent;
    QPointF m_previousDrag;
    double m_dragDist;

    bool m_paintIncremental;
    bool m_paintOnSelection;

    KisPaintDeviceSP m_target;
    KisLayerSP m_tempLayer;
    KisPaintDeviceSP m_source;

    QString m_transactionText;
    enumBrushMode m_mode;
    KisPainter *m_painter;
    bool m_smooth;
    double m_smoothness;
private:

    bool m_paintedOutline;
    QRegion m_incrementalDirtyRegion;
    QList<FreehandPaintJob*> m_paintJobs;
    KisRecordedPolyLinePaintAction* m_polyLinePaintAction;
    FreehandPaintJobExecutor* m_executor;
};



#endif // KIS_TOOL_FREEHAND_H_

