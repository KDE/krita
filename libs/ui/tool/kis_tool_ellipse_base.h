/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
