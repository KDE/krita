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

