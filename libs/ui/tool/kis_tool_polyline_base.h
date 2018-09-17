/* This file is part of the KDE project
 * Copyright (C) 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
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

    void activate(ToolActivation activation, const QSet<KoShape*> &shapes) override;
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
