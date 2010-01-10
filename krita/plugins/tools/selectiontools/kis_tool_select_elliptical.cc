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
#include <QApplication>
#include <QPainter>
#include <QPen>
#include <QLayout>
#include <QVBoxLayout>

#include <kis_debug.h>
#include <klocale.h>

#include <KoPointerEvent.h>
#include <KoShapeController.h>
#include <KoPathShape.h>
#include <KoShapeRegistry.h>
#include <KoShapeFactory.h>
#include <KoColorSpace.h>
#include <KoCompositeOp.h>

#include "kis_cursor.h"
#include "kis_image.h"
#include "kis_painter.h"
#include "kis_paintop_registry.h"
#include "kis_layer.h"
#include "kis_selection_options.h"
#include "kis_selected_transaction.h"
#include "canvas/kis_canvas2.h"
#include "kis_pixel_selection.h"
#include "kis_selection_tool_helper.h"
#include "kis_shape_tool_helper.h"


KisToolSelectElliptical::KisToolSelectElliptical(KoCanvasBase * canvas)
        : KisToolSelectBase(canvas, KisCursor::load("tool_elliptical_selection_cursor.png", 6, 6)), m_lokalTool(canvas, this)
{
}

KisToolSelectElliptical::~KisToolSelectElliptical()
{
}

QWidget* KisToolSelectElliptical::createOptionWidget()
{
    KisToolSelectBase::createOptionWidget();
    m_optWidget->setWindowTitle(i18n("Elliptical Selection"));
    return m_optWidget;
}

void KisToolSelectElliptical::LokalTool::finishEllipse(const QRectF &rect)
{
    if(rect.isNull()) return;

    KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    Q_ASSERT(kisCanvas);
    if (!kisCanvas)
        return;

    KisSelectionToolHelper helper(kisCanvas, currentNode(), i18n("Elliptical Selection"));

    if (m_selectingTool->m_selectionMode == PIXEL_SELECTION) {

        KisPixelSelectionSP tmpSel = new KisPixelSelection();

        KisPainter painter(tmpSel);
        painter.setBounds(currentImage()->bounds());
        painter.setPaintColor(KoColor(Qt::black, tmpSel->colorSpace()));
        painter.setFillStyle(KisPainter::FillStyleForegroundColor);
        painter.setStrokeStyle(KisPainter::StrokeStyleNone);
        painter.setAntiAliasPolygonFill(m_selectingTool->m_optWidget->antiAliasSelection());
        painter.setOpacity(OPACITY_OPAQUE);
        painter.setPaintOpPreset(m_selectingTool->currentPaintOpPreset(), currentImage()); // And now the painter owns the op and will destroy it.
        painter.setCompositeOp(tmpSel->colorSpace()->compositeOp(COMPOSITE_OVER));

        painter.paintEllipse(rect);

        QUndoCommand* cmd = helper.selectPixelSelection(tmpSel, m_selectingTool->m_selectAction);
        canvas()->addCommand(cmd);
    } else {
        QRectF rect = convertToPt(rect);
        KoShape* shape = KisShapeToolHelper::createEllipseShape(rect);

        helper.addSelectionShape(shape);
    }
}

#include "kis_tool_select_elliptical.moc"
