/*
 *  kis_tool_select_elliptical.cc -- part of Krita
 *
 *  Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
 *  Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
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
#include "kis_paintop_registry.h"
#include "kis_selection_options.h"
#include "kis_canvas2.h"
#include "kis_pixel_selection.h"
#include "kis_selection_tool_helper.h"
#include "kis_shape_tool_helper.h"
#include "kis_view2.h"
#include "kis_selection_manager.h"
#include "kis_system_locker.h"

KisToolSelectElliptical::KisToolSelectElliptical(KoCanvasBase *canvas)
    : KisToolEllipseBase(canvas, KisToolEllipseBase::SELECT, KisCursor::load("tool_elliptical_selection_cursor.png", 6, 6)),
      m_widgetHelper(i18n("Elliptical Selection"))
{
}

QWidget* KisToolSelectElliptical::createOptionWidget()
{
    KisCanvas2* canvas = dynamic_cast<KisCanvas2*>(this->canvas());
    Q_ASSERT(canvas);

    m_widgetHelper.createOptionWidget(canvas, this->toolId());
    return m_widgetHelper.optionWidget();
}

void KisToolSelectElliptical::keyPressEvent(QKeyEvent *event)
{
    if (!m_widgetHelper.processKeyPressEvent(event)) {
        KisTool::keyPressEvent(event);
    }
}

void KisToolSelectElliptical::finishEllipse(const QRectF &rect)
{
    KisSystemLocker locker(currentNode());
    KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    Q_ASSERT(kisCanvas);

    // If the user just clicks on the canvas deselect
    if (rect.isEmpty()) {
        kisCanvas->view()->selectionManager()->deselect();
        return;
    }

    KisSelectionToolHelper helper(kisCanvas, currentNode(), i18n("Elliptical Selection"));

    if (m_widgetHelper.selectionMode() == PIXEL_SELECTION) {
        KisPixelSelectionSP tmpSel = new KisPixelSelection();

        KisPainter painter(tmpSel);
        painter.setBounds(currentImage()->bounds());
        painter.setPaintColor(KoColor(Qt::black, tmpSel->colorSpace()));
        painter.setPaintOpPreset(currentPaintOpPreset(), currentImage()); // And now the painter owns the op and will destroy it.
        painter.setAntiAliasPolygonFill(m_widgetHelper.optionWidget()->antiAliasSelection());
        painter.setFillStyle(KisPainter::FillStyleForegroundColor);
        painter.setStrokeStyle(KisPainter::StrokeStyleNone);

        painter.paintEllipse(rect);

        helper.selectPixelSelection(tmpSel, m_widgetHelper.selectionAction());
    } else {
        QRectF ptRect = convertToPt(rect);
        KoShape* shape = KisShapeToolHelper::createEllipseShape(ptRect);

        helper.addSelectionShape(shape);
    }
}
