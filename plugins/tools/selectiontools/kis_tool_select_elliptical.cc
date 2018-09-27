/*
 *  kis_tool_select_elliptical.cc -- part of Krita
 *
 *  Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
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

#include "kis_tool_select_elliptical.h"

#include <QVBoxLayout>

#include "kis_painter.h"
#include <brushengine/kis_paintop_registry.h>
#include "kis_selection_options.h"
#include "kis_canvas2.h"
#include "kis_pixel_selection.h"
#include "kis_selection_tool_helper.h"
#include "kis_shape_tool_helper.h"
#include "KisViewManager.h"
#include "kis_selection_manager.h"

__KisToolSelectEllipticalLocal::__KisToolSelectEllipticalLocal(KoCanvasBase *canvas)
    : KisToolEllipseBase(canvas, KisToolEllipseBase::SELECT,
                         KisCursor::load("tool_elliptical_selection_cursor.png", 6, 6))
{
    setObjectName("tool_select_elliptical");
}

bool __KisToolSelectEllipticalLocal::hasUserInteractionRunning() const
{
    return false;
}

void __KisToolSelectEllipticalLocal::finishRect(const QRectF &rect, qreal roundCornersX, qreal roundCornersY)
{
    Q_UNUSED(roundCornersX);
    Q_UNUSED(roundCornersY);

    KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    Q_ASSERT(kisCanvas);

    KisSelectionToolHelper helper(kisCanvas, kundo2_i18n("Select Ellipse"));

    if (helper.tryDeselectCurrentSelection(pixelToView(rect), selectionAction())) {
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

        painter.paintEllipse(rect);

        QPainterPath cache;
        cache.addEllipse(rect);
        tmpSel->setOutlineCache(cache);

        helper.selectPixelSelection(tmpSel, selectionAction());
    } else {
        QRectF ptRect = convertToPt(rect);
        KoShape* shape = KisShapeToolHelper::createEllipseShape(ptRect);

        helper.addSelectionShape(shape, selectionAction());
    }
}


KisToolSelectElliptical::KisToolSelectElliptical(KoCanvasBase *canvas):
    KisToolSelectEllipticalTemplate(canvas, i18n("Elliptical Selection"))
{
}

void KisToolSelectElliptical::resetCursorStyle()
{
    if (selectionAction() == SELECTION_ADD) {
        useCursor(KisCursor::load("tool_elliptical_selection_cursor_add.png", 6, 6));
    } else if (selectionAction() == SELECTION_SUBTRACT) {
        useCursor(KisCursor::load("tool_elliptical_selection_cursor_sub.png", 6, 6));
    } else {
        KisToolSelectBase<__KisToolSelectEllipticalLocal>::resetCursorStyle();
    }
}

