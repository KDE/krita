/*
 *  Copyright (c) 2003-2008 Boudewijn Rempt <boud@valdyas.org>
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
#include "kis_paint_information.h"
#include "kis_resources_snapshot.h"
#include "kis_paintop_settings.h"
#include "kis_distance_information.h"
#include "kis_smoothing_options.h"

#include "krita_export.h"

class KoPointerEvent;
class KoCanvasBase;

class KisPainter;


class KisPaintingInformationBuilder;
class KisToolFreehandHelper;
class KisRecordingAdapter;


class KRITAUI_EXPORT KisToolFreehand : public KisToolPaint
{

    Q_OBJECT

public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    KisToolFreehand(KoCanvasBase * canvas, const QCursor & cursor, const KUndo2MagicString &transactionText);
    virtual ~KisToolFreehand();
    virtual int flags() const;

protected:
    bool tryPickByPaintOp(KoPointerEvent *event, AlternateAction action);

    bool primaryActionSupportsHiResEvents() const;
    void beginPrimaryAction(KoPointerEvent *event);
    void continuePrimaryAction(KoPointerEvent *event);
    void endPrimaryAction(KoPointerEvent *event);

    void activateAlternateAction(AlternateAction action);
    void deactivateAlternateAction(AlternateAction action);

    void beginAlternateAction(KoPointerEvent *event, AlternateAction action);
    void continueAlternateAction(KoPointerEvent *event, AlternateAction action);
    void endAlternateAction(KoPointerEvent *event, AlternateAction action);

    virtual bool wantsAutoScroll() const;
    void activate(ToolActivation activation, const QSet<KoShape*> &shapes);
    void deactivate();

    virtual void initStroke(KoPointerEvent *event);
    virtual void doStroke(KoPointerEvent *event);
    virtual void endStroke();

    virtual QPainterPath getOutlinePath(const QPointF &documentPos,
                                        const KoPointerEvent *event,
                                        KisPaintOpSettings::OutlineMode outlineMode);


    KisPaintingInformationBuilder* paintingInformationBuilder() const;
    KisRecordingAdapter* recordingAdapter() const;
    void resetHelper(KisToolFreehandHelper *helper);

protected slots:

    void explicitUpdateOutline();
    virtual void resetCursorStyle();
    void setAssistant(bool assistant);

private:
    friend class KisToolFreehandPaintingInformationBuilder;

    /**
     * Adjusts a coordinates according to a KisPaintingAssitant,
     * if available.
     */
    QPointF adjustPosition(const QPointF& point, const QPointF& strokeBegin);

    /**
     * Calculates a coefficient for KisPaintInformation
     * according to perspective grid values
     */
    qreal calculatePerspective(const QPointF &documentPoint);

protected:
    friend class KisView2;
    friend class KisSketchView;
    KisSmoothingOptionsSP smoothingOptions() const;
    bool m_assistant;
    double m_magnetism;

private:
    KisPaintingInformationBuilder *m_infoBuilder;
    KisToolFreehandHelper *m_helper;
    KisRecordingAdapter *m_recordingAdapter;

    QPointF m_initialGestureDocPoint;
    QPointF m_lastDocumentPoint;
    QPoint m_initialGestureGlobalPoint;
};



#endif // KIS_TOOL_FREEHAND_H_

