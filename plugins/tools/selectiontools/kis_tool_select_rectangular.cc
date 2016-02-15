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

#include "kis_system_locker.h"
#include "KisViewManager.h"
#include "kis_selection_manager.h"


__KisToolSelectRectangularLocal::__KisToolSelectRectangularLocal(KoCanvasBase * canvas)
    : KisToolRectangleBase(canvas, KisToolRectangleBase::SELECT,
                           KisCursor::load("tool_rectangular_selection_cursor.png", 6, 6))
{
        setObjectName("tool_select_rectangular");
}

void __KisToolSelectRectangularLocal::finishRect(const QRectF& rect)
{
    KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    if (!kisCanvas)
        return;

    KisSelectionToolHelper helper(kisCanvas, kundo2_i18n("Select Rectangle"));

    QRect rc(rect.normalized().toRect());
    helper.cropRectIfNeeded(&rc);

    // If the user just clicks on the canvas deselect
    if (rc.isEmpty()) {
        // Queueing this action to ensure we avoid a race condition when unlocking the node system
        QTimer::singleShot(0, kisCanvas->viewManager()->selectionManager(), SLOT(deselect()));
        return;
    }

    if (selectionMode() == PIXEL_SELECTION) {
        if (rc.isValid()) {
            KisPixelSelectionSP tmpSel = KisPixelSelectionSP(new KisPixelSelection());
            tmpSel->select(rc);

            QPainterPath cache;
            cache.addRect(rc);
            tmpSel->setOutlineCache(cache);

            helper.selectPixelSelection(tmpSel, selectionAction());
        }
    } else {
        QRectF documentRect = convertToPt(rc);
        helper.addSelectionShape(KisShapeToolHelper::createRectangleShape(documentRect));
    }
}

KisToolSelectRectangular::KisToolSelectRectangular(KoCanvasBase *canvas):
    SelectionActionHandler<__KisToolSelectRectangularLocal>(canvas, i18n("Rectangular Selection"))
{
    connect(&m_widgetHelper, SIGNAL(selectionActionChanged(int)), this, SLOT(setSelectionAction(int)));
}


void KisToolSelectRectangular::setSelectionAction(int newSelectionAction)
{
    if(newSelectionAction >= SELECTION_REPLACE && newSelectionAction <= SELECTION_INTERSECT && m_selectionAction != newSelectionAction)
    {
        if(m_widgetHelper.optionWidget())
        {
            m_widgetHelper.slotSetAction(newSelectionAction);
        }
        m_selectionAction = (SelectionAction)newSelectionAction;
        emit selectionActionChanged();
    }
}
