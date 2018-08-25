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

#ifndef KIS_TOOL_ELLIPSE_BASE_H
#define KIS_TOOL_ELLIPSE_BASE_H

#include <kis_tool_rectangle_base.h>
#include <kis_cursor.h>

class KRITAUI_EXPORT KisToolEllipseBase : public KisToolRectangleBase
{
public:
    KisToolEllipseBase(KoCanvasBase * canvas, KisToolEllipseBase::ToolType type, const QCursor & cursor=KisCursor::load("tool_ellipse_cursor.png", 6, 6));

    void paintRectangle(QPainter &gc, const QRectF &imageRect) override;

protected:
    bool showRoundCornersGUI() const override;
};

#endif // KIS_TOOL_ELLIPSE_BASE_H
