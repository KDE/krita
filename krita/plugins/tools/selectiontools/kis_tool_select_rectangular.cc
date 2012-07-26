/*
 *  kis_tool_select_rectangular.cc -- part of Krita
 *
 *  Copyright (c) 1999 Michael Koch <koch@kde.org>
 *                2001 John Califf <jcaliff@compuzone.net>
 *                2002 Patrick Julien <freak@codepimps.org>
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

#include "kis_tool_select_rectangular.h"

#include "kis_painter.h"
#include "kis_paintop_registry.h"
#include "kis_selection_options.h"
#include "kis_canvas2.h"
#include "kis_pixel_selection.h"
#include "kis_selection_tool_helper.h"
#include "kis_shape_tool_helper.h"

#include "kis_system_locker.h"
#include "kis_view2.h"
#include "kis_selection_manager.h"


KisToolSelectRectangular::KisToolSelectRectangular(KoCanvasBase * canvas)
    : KisToolRectangleBase(canvas, KisToolRectangleBase::SELECT,
                           KisCursor::load("tool_rectangular_selection_cursor.png", 6, 6)),
      m_widgetHelper(i18n("Rectangular Selection"))
{
}

QWidget* KisToolSelectRectangular::createOptionWidget()
{
    KisCanvas2* canvas = dynamic_cast<KisCanvas2*>(this->canvas());
    Q_ASSERT(canvas);

    m_widgetHelper.createOptionWidget(canvas, this->toolId());
    m_widgetHelper.optionWidget()->disableAntiAliasSelectionOption();
    return m_widgetHelper.optionWidget();
}

void KisToolSelectRectangular::keyPressEvent(QKeyEvent *event)
{
    if (!m_widgetHelper.processKeyPressEvent(event)) {
        KisTool::keyPressEvent(event);
    }
}

void KisToolSelectRectangular::finishRect(const QRectF& rect)
{
    KisSystemLocker locker(currentNode());
    QRect rc(rect.toRect());
    rc = rc.intersected(currentImage()->bounds());
    rc = rc.normalized();

    KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    if (!kisCanvas)
        return;

    // If the user just clicks on the canvas deselect
    if (rc.isEmpty()) {
        kisCanvas->view()->selectionManager()->deselect();
        return;
    }

    KisSelectionToolHelper helper(kisCanvas, currentNode(), i18n("Rectangular Selection"));

    if (m_widgetHelper.selectionMode() == PIXEL_SELECTION) {
        if (rc.isValid()) {
            KisPixelSelectionSP tmpSel = KisPixelSelectionSP(new KisPixelSelection());
            tmpSel->select(rc);
            helper.selectPixelSelection(tmpSel, m_widgetHelper.selectionAction());
        }
    } else {
        QRectF documentRect = convertToPt(rect);
        helper.addSelectionShape(KisShapeToolHelper::createRectangleShape(documentRect));
    }
}
