/*
 *  kis_tool_rectangle.cc - part of Krita
 *
 *  Copyright (c) 2000 John Califf <jcaliff@compuzone.net>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Clarence Dang <dang@k.org>
 *  Copyright (c) 2009 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
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

#include "kis_tool_rectangle.h"

#include <kis_debug.h>
#include <brushengine/kis_paintop_registry.h>
#include "KoCanvasBase.h"
#include "kis_shape_tool_helper.h"
#include "kis_figure_painting_tool_helper.h"

#include <KoCanvasController.h>
#include <KoShapeStroke.h>


KisToolRectangle::KisToolRectangle(KoCanvasBase * canvas)
        : KisToolRectangleBase(canvas, KisToolRectangleBase::PAINT, KisCursor::load("tool_rectangle_cursor.png", 6, 6))
{
    setSupportOutline(true);
    setObjectName("tool_rectangle");
}

KisToolRectangle::~KisToolRectangle()
{
}

void KisToolRectangle::resetCursorStyle()
{
    KisToolRectangleBase::resetCursorStyle();

    overrideCursorIfNotEditable();
}

void KisToolRectangle::finishRect(const QRectF &rect, qreal roundCornersX, qreal roundCornersY)
{
    if (rect.isNull())
        return;

    const KisToolShape::ShapeAddInfo info =
        shouldAddShape(currentNode());

    if (!info.shouldAddShape) {
        KisFigurePaintingToolHelper helper(kundo2_i18n("Draw Rectangle"),
                                           image(),
                                           currentNode(),
                                           canvas()->resourceManager(),
                                           strokeStyle(),
                                           fillStyle());

        QPainterPath path;

        if (roundCornersX > 0 || roundCornersY > 0) {
            path.addRoundedRect(rect, roundCornersX, roundCornersY);
        } else {
            path.addRect(rect);
        }

        helper.paintPainterPath(path);
    } else {
        const QRectF r = convertToPt(rect);
        const qreal docRoundCornersX = convertToPt(roundCornersX);
        const qreal docRoundCornersY = convertToPt(roundCornersY);
        KoShape* shape = KisShapeToolHelper::createRectangleShape(r, docRoundCornersX, docRoundCornersY);

        KoShapeStrokeSP border;
        if (strokeStyle() != KisToolShapeUtils::StrokeStyleNone) {
            const QColor color = strokeStyle() == KisToolShapeUtils::StrokeStyleForeground ?
                        canvas()->resourceManager()->foregroundColor().toQColor() :
                        canvas()->resourceManager()->backgroundColor().toQColor();

            border = toQShared(new KoShapeStroke(currentStrokeWidth(), color));
        }
        shape->setStroke(border);

        info.markAsSelectionShapeIfNeeded(shape);

        addShape(shape);
    }
}

