/*
 *  kis_tool_select_polygonal.h - part of Krayon^WKrita
 *
 *  SPDX-FileCopyrightText: 2000 John Califf <jcaliff@compuzone.net>
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2004 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *  SPDX-FileCopyrightText: 2015 Michael Abrahams <miabraha@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_tool_select_polygonal.h"

#include <KoPathShape.h>

#include "kis_algebra_2d.h"
#include "kis_painter.h"
#include <brushengine/kis_paintop_registry.h>
#include "kis_selection_options.h"
#include "kis_canvas2.h"
#include "kis_pixel_selection.h"
#include "kis_selection_tool_helper.h"
#include "kis_shape_tool_helper.h"

#include "KisViewManager.h"
#include "kis_selection_manager.h"

__KisToolSelectPolygonalLocal::__KisToolSelectPolygonalLocal(KoCanvasBase *canvas)
    : KisToolPolylineBase(canvas, KisToolPolylineBase::SELECT,
                          KisCursor::load("tool_polygonal_selection_cursor.png", 6, 6))
{
    setObjectName("tool_select_polygonal");
}


void __KisToolSelectPolygonalLocal::finishPolyline(const QVector<QPointF> &points)
{
    KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    Q_ASSERT(kisCanvas);
    if (!kisCanvas)
        return;

    const QRectF boundingViewRect = pixelToView(KisAlgebra2D::accumulateBounds(points));

    KisSelectionToolHelper helper(kisCanvas, kundo2_i18n("Select Polygon"));

    if (helper.tryDeselectCurrentSelection(pixelToView(boundingViewRect), selectionAction())) {
        return;
    }

    const SelectionMode mode =
        helper.tryOverrideSelectionMode(kisCanvas->viewManager()->selection(),
                                        selectionMode(),
                                        selectionAction());

    if (mode == PIXEL_SELECTION) {
        KisPixelSelectionSP tmpSel = new KisPixelSelection();

        KisPainter painter(tmpSel);
        painter.setPaintColor(KoColor(Qt::black, tmpSel->colorSpace()));
        painter.setAntiAliasPolygonFill(antiAliasSelection());
        painter.setFillStyle(KisPainter::FillStyleForegroundColor);
        painter.setStrokeStyle(KisPainter::StrokeStyleNone);

        painter.paintPolygon(points);

        QPainterPath cache;
        cache.addPolygon(points);
        cache.closeSubpath();
        tmpSel->setOutlineCache(cache);

        helper.selectPixelSelection(tmpSel, selectionAction());
    } else {
        KoPathShape* path = new KoPathShape();
        path->setShapeId(KoPathShapeId);

        QTransform resolutionMatrix;
        resolutionMatrix.scale(1 / currentImage()->xRes(), 1 / currentImage()->yRes());
        path->moveTo(resolutionMatrix.map(points[0]));
        for (int i = 1; i < points.count(); i++)
            path->lineTo(resolutionMatrix.map(points[i]));
        path->close();
        path->normalize();

        helper.addSelectionShape(path, selectionAction());
    }
}


KisToolSelectPolygonal::KisToolSelectPolygonal(KoCanvasBase *canvas):
    KisToolSelectBase<__KisToolSelectPolygonalLocal>(canvas, i18n("Polygonal Selection"))
{
}

void KisToolSelectPolygonal::resetCursorStyle()
{
    if (selectionAction() == SELECTION_ADD) {
        useCursor(KisCursor::load("tool_polygonal_selection_cursor_add.png", 6, 6));
    } else if (selectionAction() == SELECTION_SUBTRACT) {
        useCursor(KisCursor::load("tool_polygonal_selection_cursor_sub.png", 6, 6));
    } else {
        KisToolSelectBase<__KisToolSelectPolygonalLocal>::resetCursorStyle();
    }
}

