/*
 *  kis_tool_select_elliptical.cc -- part of Krita
 *
 *  SPDX-FileCopyrightText: 2004 Boudewijn Rempt (boud@valdyas.org)
 *  SPDX-FileCopyrightText: 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *  SPDX-FileCopyrightText: 2015 Michael Abrahams <miabraha@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_tool_select_elliptical.h"

#include <QVBoxLayout>

#include "KisViewManager.h"
#include "kis_canvas2.h"
#include "kis_painter.h"
#include "kis_pixel_selection.h"
#include "kis_selection_manager.h"
#include "kis_selection_options.h"
#include "kis_selection_tool_helper.h"
#include "kis_shape_tool_helper.h"
#include <brushengine/kis_paintop_registry.h>
#include <kis_command_utils.h>
#include <kis_selection_filters.h>
#include <kis_default_bounds.h>

__KisToolSelectEllipticalLocal::__KisToolSelectEllipticalLocal(KoCanvasBase *canvas)
    : KisToolEllipseBase(canvas, KisToolEllipseBase::SELECT,
                         KisCursor::load("tool_elliptical_selection_cursor.png", 6, 6))
{
    setObjectName("tool_select_elliptical");
}


KisToolSelectElliptical::KisToolSelectElliptical(KoCanvasBase *canvas):
    KisToolSelectBase<__KisToolSelectEllipticalLocal>(canvas, i18n("Elliptical Selection"))
{}

void KisToolSelectElliptical::finishRect(const QRectF &rect, qreal roundCornersX, qreal roundCornersY)
{
    Q_UNUSED(roundCornersX);
    Q_UNUSED(roundCornersY);

    KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    Q_ASSERT(kisCanvas);

    KisSelectionToolHelper helper(kisCanvas, kundo2_i18n("Select Ellipse"));

    if (helper.tryDeselectCurrentSelection(pixelToView(rect), selectionAction())) {
        return;
    }

    if (rect.isEmpty()) {
        return;
    }

    const SelectionMode mode =
        helper.tryOverrideSelectionMode(kisCanvas->viewManager()->selection(),
                                        selectionMode(),
                                        selectionAction());

    if (mode == PIXEL_SELECTION) {
        KisProcessingApplicator applicator(currentImage(),
                                           currentNode(),
                                           KisProcessingApplicator::NONE,
                                           KisImageSignalVector(),
                                           kundo2_i18n("Select Ellipse"));

        KisPixelSelectionSP tmpSel =
            new KisPixelSelection(new KisDefaultBounds(currentImage()));

        const bool antiAlias = antiAliasSelection();
        const int grow = growSelection();
        const int feather = featherSelection();

        QPainterPath path;
        path.addEllipse(rect);
        getRotatedPath(path, rect.center(), getRotationAngle());

        KUndo2Command *cmd = new KisCommandUtils::LambdaCommand(
            [tmpSel, antiAlias, grow, feather, path]() mutable
            -> KUndo2Command * {
                KisPainter painter(tmpSel);
                painter.setPaintColor(KoColor(Qt::black, tmpSel->colorSpace()));
                // Since the feathering already smooths the selection, the
                // antiAlias is not applied if we must feather
                painter.setAntiAliasPolygonFill(antiAlias && feather == 0);
                painter.setFillStyle(KisPainter::FillStyleForegroundColor);
                painter.setStrokeStyle(KisPainter::StrokeStyleNone);

                painter.paintPainterPath(path);

                if (grow > 0) {
                    KisGrowSelectionFilter biggy(grow, grow);
                    biggy.process(tmpSel,
                                  tmpSel->selectedRect().adjusted(-grow,
                                                                  -grow,
                                                                  grow,
                                                                  grow));
                } else if (grow < 0) {
                    KisShrinkSelectionFilter tiny(-grow, -grow, false);
                    tiny.process(tmpSel, tmpSel->selectedRect());
                }
                if (feather > 0) {
                    KisFeatherSelectionFilter feathery(feather);
                    feathery.process(tmpSel,
                                     tmpSel->selectedRect().adjusted(-feather,
                                                                     -feather,
                                                                     feather,
                                                                     feather));
                }

                if (grow == 0 && feather == 0) {
                    tmpSel->setOutlineCache(path);
                } else {
                    tmpSel->invalidateOutlineCache();
                }

                return 0;
            });

        applicator.applyCommand(cmd, KisStrokeJobData::SEQUENTIAL);
        helper.selectPixelSelection(applicator, tmpSel, selectionAction());
        applicator.end();

    } else {
        QRectF ptRect = convertToPt(rect);
        KoShape* shape = KisShapeToolHelper::createEllipseShape(ptRect);
        shape->rotate(qRadiansToDegrees(getRotationAngle()));

        helper.addSelectionShape(shape, selectionAction());
    }
}

void KisToolSelectElliptical::beginShape()
{
    beginSelectInteraction();
}

void KisToolSelectElliptical::endShape()
{
    endSelectInteraction();
}

void KisToolSelectElliptical::resetCursorStyle()
{
    if (selectionAction() == SELECTION_ADD) {
        useCursor(KisCursor::load("tool_elliptical_selection_cursor_add.png", 6, 6));
    } else if (selectionAction() == SELECTION_SUBTRACT) {
        useCursor(KisCursor::load("tool_elliptical_selection_cursor_sub.png", 6, 6));
    } else if (selectionAction() == SELECTION_INTERSECT) {
        useCursor(KisCursor::load("tool_elliptical_selection_cursor_inter.png", 6, 6));
    } else if (selectionAction() == SELECTION_SYMMETRICDIFFERENCE) {
        useCursor(KisCursor::load("tool_elliptical_selection_cursor_symdiff.png", 6, 6));
    } else {
        KisToolSelectBase<__KisToolSelectEllipticalLocal>::resetCursorStyle();
    }
}

