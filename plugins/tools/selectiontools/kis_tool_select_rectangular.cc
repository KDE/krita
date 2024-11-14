/*
 *  kis_tool_select_rectangular.cc -- part of Krita
 *
 *  SPDX-FileCopyrightText: 1999 Michael Koch <koch@kde.org>
 *  SPDX-FileCopyrightText: 2001 John Califf <jcaliff@compuzone.net>
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *  SPDX-FileCopyrightText: 2015 Michael Abrahams <miabraha@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_tool_select_rectangular.h"

#include "kis_painter.h"
#include <brushengine/kis_paintop_registry.h>
#include "kis_selection_options.h"
#include "kis_canvas2.h"
#include "kis_pixel_selection.h"
#include "kis_selection_tool_helper.h"
#include "kis_shape_tool_helper.h"
#include <kis_default_bounds.h>

#include "KisViewManager.h"
#include "kis_selection_manager.h"
#include <kis_command_utils.h>
#include <kis_selection_filters.h>

__KisToolSelectRectangularLocal::__KisToolSelectRectangularLocal(KoCanvasBase * canvas)
    : KisToolRectangleBase(canvas, KisToolRectangleBase::SELECT,
                           KisCursor::load("tool_rectangular_selection_cursor.png", 6, 6))
{
    setObjectName("tool_select_rectangular");
}


KisToolSelectRectangular::KisToolSelectRectangular(KoCanvasBase *canvas):
    KisToolSelectBase<__KisToolSelectRectangularLocal>(canvas, i18n("Rectangular Selection"))
{}

void KisToolSelectRectangular::finishRect(const QRectF& rect, qreal roundCornersX, qreal roundCornersY)
{
    KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    if (!kisCanvas)
        return;

    KisSelectionToolHelper helper(kisCanvas, kundo2_i18n("Select Rectangle"));

    QRect rc(rect.normalized().toRect());

    if (helper.tryDeselectCurrentSelection(pixelToView(rc), selectionAction())) {
        return;
    }

    if (helper.canShortcutToNoop(rc, selectionAction())) {
        return;
    }

    const SelectionMode mode =
        helper.tryOverrideSelectionMode(kisCanvas->viewManager()->selection(),
                                        selectionMode(),
                                        selectionAction());

    if (!rc.isValid()) {
        return;
    }
    if (mode == PIXEL_SELECTION) {
        KisProcessingApplicator applicator(currentImage(),
                                           currentNode(),
                                           KisProcessingApplicator::NONE,
                                           KisImageSignalVector(),
                                           kundo2_i18n("Select Rectangle"));

        KisPixelSelectionSP tmpSel =
            new KisPixelSelection(new KisDefaultBounds(currentImage()));

        const bool antiAlias = antiAliasSelection();
        const int grow = growSelection();
        const int feather = featherSelection();

        QPainterPath path;
        if (roundCornersX > 0 || roundCornersY > 0) {
            path.addRoundedRect(rc, roundCornersX, roundCornersY);
        } else {
            path.addRect(rc);
        }
        getRotatedPath(path, rc.center(), getRotationAngle());

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
        QRectF documentRect = convertToPt(rc);
        const qreal docRoundCornersX = convertToPt(roundCornersX);
        const qreal docRoundCornersY = convertToPt(roundCornersY);

        KoShape* shape = KisShapeToolHelper::createRectangleShape(documentRect,
                                                                  docRoundCornersX,
                                                                  docRoundCornersY);
        shape->rotate(qRadiansToDegrees(getRotationAngle()));
        helper.addSelectionShape(shape, selectionAction());
    }
}

void KisToolSelectRectangular::beginShape()
{
    beginSelectInteraction();
}

void KisToolSelectRectangular::endShape()
{
    endSelectInteraction();
}

void KisToolSelectRectangular::resetCursorStyle()
{
    if (selectionAction() == SELECTION_ADD) {
        useCursor(KisCursor::load("tool_rectangular_selection_cursor_add.png", 6, 6));
    } else if (selectionAction() == SELECTION_SUBTRACT) {
        useCursor(KisCursor::load("tool_rectangular_selection_cursor_sub.png", 6, 6));
    } else if (selectionAction() == SELECTION_INTERSECT) {
        useCursor(KisCursor::load("tool_rectangular_selection_cursor_inter.png", 6, 6));
    } else if (selectionAction() == SELECTION_SYMMETRICDIFFERENCE) {
        useCursor(KisCursor::load("tool_rectangular_selection_cursor_symdiff.png", 6, 6));
    } else {
        KisToolSelectBase<__KisToolSelectRectangularLocal>::resetCursorStyle();
    }
}

