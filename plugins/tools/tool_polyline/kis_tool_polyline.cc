/*
 *  SPDX-FileCopyrightText: 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *  SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2009 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_tool_polyline.h"

#include <QVector>

#include <KoCanvasBase.h>
#include <KoPathShape.h>
#include <KoShapeStroke.h>

#include <brushengine/kis_paintop_preset.h>
#include "kis_figure_painting_tool_helper.h"

KisToolPolyline::KisToolPolyline(KoCanvasBase * canvas)
        : KisToolPolylineBase(canvas, KisToolPolylineBase::PAINT, KisCursor::load("tool_polyline_cursor.png", 6, 6))
{
    setObjectName("tool_polyline");
    setSupportOutline(true);
}

KisToolPolyline::~KisToolPolyline()
{
}

void KisToolPolyline::resetCursorStyle()
{
    KisToolPolylineBase::resetCursorStyle();
    overrideCursorIfNotEditable();
}

QWidget* KisToolPolyline::createOptionWidget()
{
    return KisToolPolylineBase::createOptionWidget();
}

void KisToolPolyline::finishPolyline(const QVector<QPointF>& points)
{
    const KisToolShape::ShapeAddInfo info =
        shouldAddShape(currentNode());

    if (!info.shouldAddShape || info.shouldAddSelectionShape) {
        KisFigurePaintingToolHelper helper(kundo2_i18n("Draw Polyline"),
                                           image(),
                                           currentNode(),
                                           canvas()->resourceManager(),
                                           strokeStyle(),
                                           fillStyle(),
                                           fillTransform());
        helper.paintPolyline(points);
    } else {
        KoPathShape* path = new KoPathShape();
        path->setShapeId(KoPathShapeId);

        QTransform resolutionMatrix;
        resolutionMatrix.scale(1 / currentImage()->xRes(), 1 / currentImage()->yRes());
        path->moveTo(resolutionMatrix.map(points[0]));
        for (int i = 1; i < points.count(); i++)
            path->lineTo(resolutionMatrix.map(points[i]));
        path->normalize();

        addShape(path);
    }
}

