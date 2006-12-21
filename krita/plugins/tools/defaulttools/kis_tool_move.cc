/*
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
 *                1999 Michael Koch    <koch@kde.org>
 *                2002 Patrick Julien <freak@codepimps.org>
 *                2004 Boudewijn Rempt <boud@valdyas.org>
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

#include <stdlib.h>
#include <QPoint>
#include <QColor>
#include <QMouseEvent>

#include <kaction.h>
#include <kactioncollection.h>
#include <klocale.h>

#include "kis_cursor.h"
//#include "kis_tool.h"
#include "kis_image.h"
#include "kis_paint_device.h"
#include "kis_tool_move.h"
#include "kis_tool_move.moc"
#include "KoCanvasBase.h"
#include "KoPointerEvent.h"
#include "kis_selection.h"
#include "kis_selection_manager.h"
#include "kis_layer.h"

KisToolMove::KisToolMove(KoCanvasBase * canvas)
    :  KisTool(canvas, KisCursor::load("tool_line_cursor.png", 6, 6))
{
    setObjectName("tool_move");

}

KisToolMove::~KisToolMove()
{
}

void KisToolMove::paint(QPainter& gc, KoViewConverter &converter)
{
}

void KisToolMove::mousePressEvent(KoPointerEvent *e)
{
    if (m_canvas && e->button() == Qt::LeftButton) {
        QPointF pos = convertToPixelCoord(e);

        KisLayerSP dev;

        if (!m_currentImage || !(dev = m_currentImage->activeLayer()))
            return;

        m_dragStart = QPoint(static_cast<int>(pos.x()), static_cast<int>(pos.y()));
        m_strategy.startDrag(m_dragStart);
    }
}

void KisToolMove::mouseMoveEvent(KoPointerEvent *e)
{
    if (m_canvas) {
	QPointF posf = convertToPixelCoord(e);
        QPoint pos = QPoint(static_cast<int>(posf.x()), static_cast<int>(posf.y()));
        if((e->modifiers() & Qt::AltModifier) || (e->modifiers() & Qt::ControlModifier)) {
            if(fabs(pos.x() - m_dragStart.x()) > fabs(pos.y() - m_dragStart.y()))
                pos.setY(m_dragStart.y());
            else
                pos.setX(m_dragStart.x());
        }
        m_strategy.drag(pos);
    }
}

void KisToolMove::mouseReleaseEvent(KoPointerEvent *e)
{
    if (m_canvas && e->button() == Qt::LeftButton) {
	QPointF pos = convertToPixelCoord(e);
	m_strategy.endDrag(QPoint(static_cast<int>(pos.x()), static_cast<int>(pos.y())));
    }
}

// void KisToolMove::setup(KActionCollection *collection)
// {
//     m_action = collection->action(objectName());

//     if (m_action == 0) {
//         m_action = new KAction(KIcon("move"),
//                                i18n("&Move"),
//                                collection,
//                                objectName());
//         m_action->setShortcut(QKeySequence(Qt::SHIFT+Qt::Key_V));
//         connect(m_action, SIGNAL(triggered()), this, SLOT(activate()));
//         m_action->setToolTip(i18n("Move"));
//         m_action->setActionGroup(actionGroup());
//         m_ownAction = true;
//     }
// }

