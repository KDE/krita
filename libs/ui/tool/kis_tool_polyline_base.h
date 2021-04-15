/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KIS_TOOL_POLYLINE_BASE_H
#define KIS_TOOL_POLYLINE_BASE_H

#include <kis_tool_shape.h>
#include <kis_cursor.h>

class KRITAUI_EXPORT KisToolPolylineBase : public KisToolShape
{
Q_OBJECT
public:
    enum ToolType {
        PAINT,
        SELECT
    };

    KisToolPolylineBase(KoCanvasBase * canvas, KisToolPolylineBase::ToolType type, const QCursor & cursor=KisCursor::load("tool_polygon_cursor.png", 6, 6));

    void beginPrimaryAction(KoPointerEvent *event) override;
    void endPrimaryAction(KoPointerEvent *event) override;
    void beginPrimaryDoubleClickAction(KoPointerEvent *event) override;
    void mouseMoveEvent(KoPointerEvent *event) override;

    void beginAlternateAction(KoPointerEvent *event, AlternateAction action) override;

    void paint(QPainter& gc, const KoViewConverter &converter) override;

    void activate(const QSet<KoShape*> &shapes) override;
    void deactivate() override;
    void listenToModifiers(bool listen) override;
    bool listeningToModifiers() override;
    void requestStrokeEnd() override;
    void requestStrokeCancellation() override;

    bool hasUserInteractionRunning() const;

protected:
    virtual void finishPolyline(const QVector<QPointF>& points) = 0;

private:
    void endStroke();
    void cancelStroke();
    void updateArea();
    QRectF dragBoundingRect();

private Q_SLOTS:
    virtual void undoSelection();

private:

    QPointF m_dragStart;
    QPointF m_dragEnd;
    bool m_dragging;
    bool m_listenToModifiers;
    vQPointF m_points;
    ToolType m_type;
    bool m_closeSnappingActivated;
};

#endif // KIS_TOOL_POLYLINE_BASE_H
