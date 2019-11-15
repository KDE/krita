/*
 *  kis_tool_polygon.cc -- part of Krita
 *
 *  Copyright (c) 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *  Copyright (c) 2009 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
 * Copyright (C) 2010 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_tool_polygon.h"

#include <KoPointerEvent.h>
#include <KoCanvasBase.h>
#include <KoPathShape.h>
#include <KoShapeStroke.h>

#include <brushengine/kis_paintop_registry.h>
#include "kis_figure_painting_tool_helper.h"

KisToolPolygon::KisToolPolygon(KoCanvasBase *canvas)
        : KisToolPolylineBase(canvas,  KisToolPolylineBase::PAINT, KisCursor::load("tool_polygon_cursor.png", 6, 6))
{
    setObjectName("tool_polygon");
    setSupportOutline(true);
}

KisToolPolygon::~KisToolPolygon()
{
}

void KisToolPolygon::resetCursorStyle()
{
    KisToolPolylineBase::resetCursorStyle();
    overrideCursorIfNotEditable();
}

void KisToolPolygon::finishPolyline(const QVector<QPointF>& points)
{
    const KisToolShape::ShapeAddInfo info =
        shouldAddShape(currentNode());

    if (!info.shouldAddShape) {
        KisFigurePaintingToolHelper helper(kundo2_i18n("Draw Polygon"),
                                           image(),
                                           currentNode(),
                                           canvas()->resourceManager(),
                                           strokeStyle(),
                                           fillStyle());
        helper.paintPolygon(points);
    } else {
        // remove the last point if it overlaps with the first
        QVector<QPointF> newPoints = points;
        if (newPoints.size() > 1 && newPoints.first() == newPoints.last()) {
            newPoints.removeLast();
        }
        KoPathShape* path = new KoPathShape();
        path->setShapeId(KoPathShapeId);

        QTransform resolutionMatrix;
        resolutionMatrix.scale(1 / currentImage()->xRes(), 1 / currentImage()->yRes());
        path->moveTo(resolutionMatrix.map(newPoints[0]));
        for (int i = 1; i < newPoints.size(); i++)
            path->lineTo(resolutionMatrix.map(newPoints[i]));
        path->close();
        path->normalize();

        info.markAsSelectionShapeIfNeeded(path);

        addShape(path);
    }
}

