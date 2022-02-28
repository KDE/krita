/*
 *  kis_tool_select_freehand.h - part of Krayon^WKrita
 *
 *  SPDX-FileCopyrightText: 2000 John Califf <jcaliff@compuzone.net>
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2004 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *  SPDX-FileCopyrightText: 2015 Michael Abrahams <miabraha@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_tool_select_outline.h"

#include <kis_debug.h>
#include <klocalizedstring.h>

#include <KoPointerEvent.h>
#include <KoShapeController.h>
#include <KoPathShape.h>
#include <KoColorSpace.h>
#include <KoCompositeOp.h>
#include <KoViewConverter.h>

#include <kis_layer.h>
#include <kis_selection_options.h>
#include <kis_cursor.h>
#include <kis_image.h>

#include "kis_painter.h"
#include <brushengine/kis_paintop_registry.h>
#include "canvas/kis_canvas2.h"
#include "kis_pixel_selection.h"
#include "kis_selection_tool_helper.h"

#include "kis_algebra_2d.h"

__KisToolSelectOutlineLocal::__KisToolSelectOutlineLocal(KoCanvasBase * canvas)
    : KisToolOutlineBase(canvas, KisToolOutlineBase::SELECT,
                         KisCursor::load("tool_outline_selection_cursor.png", 5, 5))
{
    setObjectName("tool_select_outline");
}


KisToolSelectOutline::KisToolSelectOutline(KoCanvasBase * canvas)
    : KisToolSelectBase<__KisToolSelectOutlineLocal>(canvas, i18n("Freehand Selection"))
{}

void KisToolSelectOutline::finishOutline(const QVector<QPointF>& points)
{
    KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    KIS_ASSERT_RECOVER_RETURN(kisCanvas);
    kisCanvas->updateCanvas();

    const QRectF boundingRect = KisAlgebra2D::accumulateBounds(points);
    const QRectF boundingViewRect = pixelToView(boundingRect);

    KisSelectionToolHelper helper(kisCanvas, kundo2_i18n("Freehand Selection"));

    if (helper.tryDeselectCurrentSelection(boundingViewRect, selectionAction())) {
        endSelectInteraction();
        return;
    }

    if (points.count() > 2) {
        QApplication::setOverrideCursor(KisCursor::waitCursor());

        const SelectionMode mode =
            helper.tryOverrideSelectionMode(kisCanvas->viewManager()->selection(),
                                            selectionMode(),
                                            selectionAction());

        if (mode == PIXEL_SELECTION) {

            KisPixelSelectionSP tmpSel = KisPixelSelectionSP(new KisPixelSelection());

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
        QApplication::restoreOverrideCursor();
    }
}

void KisToolSelectOutline::beginShape()
{
    beginSelectInteraction();
}

void KisToolSelectOutline::endShape()
{
    endSelectInteraction();
}

bool KisToolSelectOutline::primaryActionSupportsHiResEvents() const
{
    return !isMovingSelection();
}

bool KisToolSelectOutline::alternateActionSupportsHiResEvents(AlternateAction action) const
{
    /**
     * In selection tools we abuse alternate actions to switch different
     * selection modes. So we should notify input manager that we need a
     * good precision for them
     */

    Q_UNUSED(action);
    return !isMovingSelection();
}

void KisToolSelectOutline::resetCursorStyle()
{
    if (selectionAction() == SELECTION_ADD) {
        useCursor(KisCursor::load("tool_outline_selection_cursor_add.png", 5, 5));
    } else if (selectionAction() == SELECTION_SUBTRACT) {
        useCursor(KisCursor::load("tool_outline_selection_cursor_sub.png", 5, 5));
    } else if (selectionAction() == SELECTION_INTERSECT) {
        useCursor(KisCursor::load("tool_outline_selection_cursor_inter.png", 5, 5));
    } else if (selectionAction() == SELECTION_SYMMETRICDIFFERENCE) {
        useCursor(KisCursor::load("tool_outline_selection_cursor_symdiff.png", 5, 5));
    } else {
        KisToolSelectBase<__KisToolSelectOutlineLocal>::resetCursorStyle();
    }
}

