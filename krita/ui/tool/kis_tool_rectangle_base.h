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

#ifndef KIS_TOOL_RECTANGLE_BASE_H
#define KIS_TOOL_RECTANGLE_BASE_H

#include <kis_tool_shape.h>
#include <kis_cursor.h>

class KRITAUI_EXPORT KisToolRectangleBase : public KisToolShape
{
Q_OBJECT

signals:
    void rectangleChanged(const QRectF &newRect);
    
public slots:
    void constraintsChanged(bool forceRatio, bool forceWidth, bool forceHeight, float ratio, float width, float height);
    
public:
    enum ToolType {
        PAINT,
        SELECT
    };

    explicit KisToolRectangleBase(KoCanvasBase * canvas, KisToolRectangleBase::ToolType type, const QCursor & cursor=KisCursor::load("tool_rectangle_cursor.png", 6, 6));

    virtual void beginPrimaryAction(KoPointerEvent *event);
    virtual void continuePrimaryAction(KoPointerEvent *event);
    virtual void endPrimaryAction(KoPointerEvent *event);

    virtual void paint(QPainter& gc, const KoViewConverter &converter);
    virtual void deactivate();

    QList<QPointer<QWidget> > createOptionWidgets();
    
protected:
    virtual void finishRect(const QRectF&)=0;

    QPointF m_dragCenter;
    QPointF m_dragStart;
    QPointF m_dragEnd;
    ToolType m_type;
    
    bool m_isRatioForced;
    bool m_isWidthForced;
    bool m_isHeightForced;
    float m_forcedRatio;
    float m_forcedWidth;
    float m_forcedHeight;
    
    bool isFixedSize();
    void applyConstraints(QSizeF& area, bool overrideRatio);

    void updateArea();
    virtual void paintRectangle(QPainter &gc, const QRectF &imageRect);
    virtual QRectF createRect(const QPointF &start, const QPointF &end);
};

#endif // KIS_TOOL_RECTANGLE_BASE_H
