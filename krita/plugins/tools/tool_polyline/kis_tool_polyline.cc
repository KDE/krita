/*
 *  Copyright (c) 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_tool_polyline.h"

#include <QVector>

#include <KoCanvasBase.h>
#include <KoPathShape.h>
#include <KoShapeStroke.h>

#include <kis_paintop_preset.h>
#include "kis_figure_painting_tool_helper.h"

#include <recorder/kis_action_recorder.h>
#include <recorder/kis_recorded_path_paint_action.h>
#include <recorder/kis_node_query_path.h>

#include <kis_system_locker.h>


KisToolPolyline::KisToolPolyline(KoCanvasBase * canvas)
        : KisToolPolylineBase(canvas, KisToolPolylineBase::PAINT, KisCursor::load("tool_polyline_cursor.png", 6, 6))
{
    setObjectName("tool_polyline");
}

KisToolPolyline::~KisToolPolyline()
{
}

QWidget* KisToolPolyline::createOptionWidget()
{
    // there are no options there
    return KisTool::createOptionWidget();
}

void KisToolPolyline::finishPolyline(const QVector<QPointF>& points)
{
    if (image()) {
        KisRecordedPathPaintAction linePaintAction(KisNodeQueryPath::absolutePath(currentNode()), currentPaintOpPreset());
        setupPaintAction(&linePaintAction);
        linePaintAction.addPolyLine(points.toList());
        image()->actionRecorder()->addAction(linePaintAction);
    }
    if (!currentNode()->inherits("KisShapeLayer")) {
        KisSystemLocker locker(currentNode());
        KisFigurePaintingToolHelper helper(i18n("Polyline"),
                                           image(),
                                           canvas()->resourceManager(),
                                           strokeStyle(),
                                           fillStyle());
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

        KoShapeStroke* border = new KoShapeStroke(1.0, currentFgColor().toQColor());
        path->setStroke(border);

        addShape(path);
    }
    notifyModified();
}

#include "kis_tool_polyline.moc"
