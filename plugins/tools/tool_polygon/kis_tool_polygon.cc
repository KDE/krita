/*
 *  kis_tool_polygon.cc -- part of Krita
 *
 *  SPDX-FileCopyrightText: 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *  SPDX-FileCopyrightText: 2009 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2010 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
                                           fillStyle(),
                                           fillTransform());
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

