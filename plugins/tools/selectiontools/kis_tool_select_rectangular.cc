/*
 *  kis_tool_select_rectangular.cc -- part of Krita
 *
 *  Copyright (c) 1999 Michael Koch <koch@kde.org>
 *  Copyright (c) 2001 John Califf <jcaliff@compuzone.net>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *  Copyright (c) 2015 Michael Abrahams <miabraha@gmail.com>
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

#include "kis_tool_select_rectangular.h"

#include "kis_painter.h"
#include <brushengine/kis_paintop_registry.h>
#include "kis_selection_options.h"
#include "kis_canvas2.h"
#include "kis_pixel_selection.h"
#include "kis_selection_tool_helper.h"
#include "kis_shape_tool_helper.h"

#include "KisViewManager.h"
#include "kis_selection_manager.h"

__KisToolSelectRectangularLocal::__KisToolSelectRectangularLocal(KoCanvasBase * canvas)
    : KisToolRectangleBase(canvas, KisToolRectangleBase::SELECT,
                           KisCursor::load("tool_rectangular_selection_cursor.png", 6, 6))
{
        setObjectName("tool_select_rectangular");
}

bool __KisToolSelectRectangularLocal::hasUserInteractionRunning() const
{
    return false;
}

void __KisToolSelectRectangularLocal::finishRect(const QRectF& rect, qreal roundCornersX, qreal roundCornersY)
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

    if (mode == PIXEL_SELECTION) {
        if (rc.isValid()) {
            KisPixelSelectionSP tmpSel = KisPixelSelectionSP(new KisPixelSelection());

            QPainterPath cache;

            if (roundCornersX > 0 || roundCornersY > 0) {
                cache.addRoundedRect(rc, roundCornersX, roundCornersY);
            } else {
                cache.addRect(rc);
            }

            {
                KisPainter painter(tmpSel);
                painter.setPaintColor(KoColor(Qt::black, tmpSel->colorSpace()));
                painter.setAntiAliasPolygonFill(true);
                painter.setFillStyle(KisPainter::FillStyleForegroundColor);
                painter.setStrokeStyle(KisPainter::StrokeStyleNone);

                painter.paintPainterPath(cache);
            }


            tmpSel->setOutlineCache(cache);

            helper.selectPixelSelection(tmpSel, selectionAction());
        }
    } else {
        QRectF documentRect = convertToPt(rc);
        const qreal docRoundCornersX = convertToPt(roundCornersX);
        const qreal docRoundCornersY = convertToPt(roundCornersY);

        helper.addSelectionShape(KisShapeToolHelper::createRectangleShape(documentRect,
                                                                          docRoundCornersX,
                                                                          docRoundCornersY),
                                 selectionAction());
    }
}

KisToolSelectRectangular::KisToolSelectRectangular(KoCanvasBase *canvas):
    KisToolSelectBase<__KisToolSelectRectangularLocal>(canvas, i18n("Rectangular Selection"))
{
}

void KisToolSelectRectangular::resetCursorStyle()
{
    if (selectionAction() == SELECTION_ADD) {
        useCursor(KisCursor::load("tool_rectangular_selection_cursor_add.png", 6, 6));
    } else if (selectionAction() == SELECTION_SUBTRACT) {
        useCursor(KisCursor::load("tool_rectangular_selection_cursor_sub.png", 6, 6));
    } else {
        KisToolSelectBase<__KisToolSelectRectangularLocal>::resetCursorStyle();
    }
}

