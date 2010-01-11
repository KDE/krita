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
    KisToolPolylineBase(KoCanvasBase * canvas, const QCursor & cursor=KisCursor::load("tool_polygon_cursor.png", 6, 6));

    virtual void mousePressEvent(KoPointerEvent *event);
    virtual void mouseMoveEvent(KoPointerEvent *event);
    virtual void mouseReleaseEvent(KoPointerEvent *event);
    virtual void mouseDoubleClickEvent(KoPointerEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void paint(QPainter& gc, const KoViewConverter &converter);

protected:
    virtual void finishPolyline(const QVector<QPointF>& points)=0;
    void updateArea();
    QRectF dragBoundingRect();

private slots:
    void cancel();
    void finish();

private:

    QPointF m_dragStart;
    QPointF m_dragEnd;
    bool m_dragging;
    vQPointF m_points;
};

#endif // KIS_TOOL_POLYLINE_BASE_H
