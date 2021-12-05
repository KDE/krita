/*
 *  SPDX-FileCopyrightText: 2003-2008 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_TOOL_FREEHAND_H_
#define KIS_TOOL_FREEHAND_H_

#include <brushengine/kis_paint_information.h>
#include <brushengine/kis_paintop_settings.h>
#include <kis_distance_information.h>

#include "kis_types.h"
#include "kis_tool_paint.h"
#include "kis_smoothing_options.h"
#include "kis_signal_compressor_with_param.h"

#include "kritaui_export.h"

class KoPointerEvent;
class KoCanvasBase;

class KisPaintingInformationBuilder;
class KisToolFreehandHelper;


class KRITAUI_EXPORT KisToolFreehand : public KisToolPaint
{

    Q_OBJECT

public:
    KisToolFreehand(KoCanvasBase * canvas, const QCursor & cursor, const KUndo2MagicString &transactionText);
    ~KisToolFreehand() override;
    int flags() const override;
    void mouseMoveEvent(KoPointerEvent *event) override;

public Q_SLOTS:
    void activate(const QSet<KoShape*> &shapes) override;
    void deactivate() override;

protected:
    bool trySampleByPaintOp(KoPointerEvent *event, AlternateAction action);

    bool primaryActionSupportsHiResEvents() const override;
    void beginPrimaryAction(KoPointerEvent *event) override;
    void continuePrimaryAction(KoPointerEvent *event) override;
    void endPrimaryAction(KoPointerEvent *event) override;

    void activateAlternateAction(AlternateAction action) override;
    void deactivateAlternateAction(AlternateAction action) override;

    void beginAlternateAction(KoPointerEvent *event, AlternateAction action) override;
    void continueAlternateAction(KoPointerEvent *event, AlternateAction action) override;
    void endAlternateAction(KoPointerEvent *event, AlternateAction action) override;

    bool wantsAutoScroll() const override;


    virtual void initStroke(KoPointerEvent *event);
    virtual void doStroke(KoPointerEvent *event);
    virtual void endStroke();

    QPainterPath getOutlinePath(const QPointF &documentPos,
                                        const KoPointerEvent *event,
                                        KisPaintOpSettings::OutlineMode outlineMode) override;


    KisPaintingInformationBuilder* paintingInformationBuilder() const;
    void resetHelper(KisToolFreehandHelper *helper);

protected Q_SLOTS:

    void explicitUpdateOutline();
    void resetCursorStyle() override;
    void setAssistant(bool assistant);
    void setOnlyOneAssistantSnap(bool assistant);
    void setSnapEraser(bool assistant);
    void slotDoResizeBrush(qreal newSize);

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
    friend class KisViewManager;
    friend class KisView;
    KisSmoothingOptionsSP smoothingOptions() const;
    bool m_assistant;
    double m_magnetism;
    bool m_only_one_assistant;
    bool m_eraser_snapping;

private:
    KisPaintingInformationBuilder *m_infoBuilder;
    KisToolFreehandHelper *m_helper;

    QPointF m_initialGestureDocPoint;
    QPointF m_lastDocumentPoint;
    qreal m_lastPaintOpSize;
    QPoint m_initialGestureGlobalPoint;

    bool m_paintopBasedSamplingInAction;
    KisSignalCompressorWithParam<qreal> m_brushResizeCompressor;
};



#endif // KIS_TOOL_FREEHAND_H_
