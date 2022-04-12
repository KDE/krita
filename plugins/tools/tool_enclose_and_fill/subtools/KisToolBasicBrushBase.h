/*
 *  SPDX-FileCopyrightText: 2022 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISTOOLBASICBRUSHBASE_H
#define KISTOOLBASICBRUSHBASE_H

#include <kis_tool_shape.h>
#include <kis_cursor.h>

class KisToolBasicBrushBase : public KisToolShape
{
    Q_OBJECT

public:
    enum ToolType {
        PAINT,
        SELECT
    };

    KisToolBasicBrushBase(KoCanvasBase *canvas,
                          ToolType type,
                          const QCursor & cursor=KisCursor::load("tool_outline_selection_cursor.png", 6, 6));
    ~KisToolBasicBrushBase() override;

    void mouseMoveEvent(KoPointerEvent *event) override;
    void beginPrimaryAction(KoPointerEvent *event) override;
    void continuePrimaryAction(KoPointerEvent *event) override;
    void endPrimaryAction(KoPointerEvent *event) override;
    void activateAlternateAction(AlternateAction action) override;
    void deactivateAlternateAction(AlternateAction action) override;
    void beginAlternateAction(KoPointerEvent *event, AlternateAction action) override;
    void continueAlternateAction(KoPointerEvent *event, AlternateAction action) override;
    void endAlternateAction(KoPointerEvent *event, AlternateAction action) override;
    void paint(QPainter& gc, const KoViewConverter &converter) override;

    qreal pressureToCurve(qreal pressure);

public Q_SLOTS:
    void activate(const QSet<KoShape*> &shapes) override;
    void deactivate() override;
    void setPreviewColor(const QColor &color);

protected:
    virtual void finishStroke(const QPainterPath& stroke) = 0;
    QPainterPath getOutlinePath(const QPointF &documentPos,
                                const KoPointerEvent *event,
                                KisPaintOpSettings::OutlineMode outlineMode) override;

protected Q_SLOTS:
    void updateSettings();
    void resetCursorStyle() override;

private:
    static constexpr int levelOfPressureResolution = 1024;
    static constexpr int feedbackLineWidth{2};

    QPainterPath m_path;
    QPointF m_lastPosition;
    qreal m_lastPressure {1.0};
    ToolType m_type {PAINT};

    QVector<qreal> m_pressureSamples;
    OutlineStyle m_outlineStyle {OUTLINE_FULL};
    bool m_showOutlineWhilePainting {true};
    bool m_forceAlwaysFullSizedOutline {true};

    QPointF m_changeSizeInitialGestureDocPoint;
    QPointF m_changeSizeLastDocumentPoint;
    qreal m_changeSizeLastPaintOpSize {0.0};
    QPoint m_changeSizeInitialGestureGlobalPoint;

    QColor m_previewColor;

    QPainterPath generateSegment(const QPointF &point1, qreal radius1, const QPointF &point2, qreal radius2) const;
    void update(const QRectF &strokeSegmentRect);
};

#endif
