/*
 *  kis_tool_ellipse.cc - part of Krayon
 *
 *  Copyright (c) 2000 John Califf <jcaliff@compuzone.net>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Clarence Dang <dang@kde.org>
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

#include "kis_tool_ellipse.h"
#include <KoCanvasBase.h>
#include <KoShapeStroke.h>

#include <kis_shape_tool_helper.h>
#include "kis_figure_painting_tool_helper.h"
#include <brushengine/kis_paintop_preset.h>

KisToolEllipse::KisToolEllipse(KoCanvasBase * canvas)
        : KisToolEllipseBase(canvas, KisToolEllipseBase::PAINT, KisCursor::load("tool_ellipse_cursor.png", 6, 6))
{
    setObjectName("tool_ellipse");
    setSupportOutline(true);
}

KisToolEllipse::~KisToolEllipse()
{
}

void KisToolEllipse::resetCursorStyle()
{
    KisToolEllipseBase::resetCursorStyle();

    overrideCursorIfNotEditable();
}

void KisToolEllipse::finishRect(const QRectF& rect, qreal roundCornersX, qreal roundCornersY)
{
    Q_UNUSED(roundCornersX);
    Q_UNUSED(roundCornersY);

    if (rect.isEmpty())
        return;

    const KisToolShape::ShapeAddInfo info =
        shouldAddShape(currentNode());

    if (!info.shouldAddShape) {
        KisFigurePaintingToolHelper helper(kundo2_i18n("Draw Ellipse"),
                                           image(),
                                           currentNode(),
                                           canvas()->resourceManager(),
                                           strokeStyle(),
                                           fillStyle());
        helper.paintEllipse(rect);
    } else {
        QRectF r = convertToPt(rect);
        KoShape* shape = KisShapeToolHelper::createEllipseShape(r);
        KoShapeStrokeSP border(new KoShapeStroke(currentStrokeWidth(), currentFgColor().toQColor()));
        shape->setStroke(border);

        info.markAsSelectionShapeIfNeeded(shape);

        addShape(shape);
    }
}

