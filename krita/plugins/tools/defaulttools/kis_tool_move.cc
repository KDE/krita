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
#include <qpoint.h>
#include <kaction.h>
#include <klocale.h>
#include <qcolor.h>
#include "kis_canvas_subject.h"
#include "kis_cursor.h"
#include "kis_image.h"
#include "kis_paint_device.h"
#include "kis_tool_move.h"
#include "kis_tool_move.moc"
#include "kis_button_press_event.h"
#include "kis_button_release_event.h"
#include "kis_move_event.h"
#include "kis_selection.h"
#include "kis_selection_manager.h"
#include "kis_layer.h"

KisToolMove::KisToolMove()
    : super(i18n("Move Tool"))
{
    setName("tool_move");
    m_subject = 0;
    setCursor(KisCursor::moveCursor());
}

KisToolMove::~KisToolMove()
{
}

void KisToolMove::update(KisCanvasSubject *subject)
{
    m_subject = subject;
    m_strategy.reset(subject);
    super::update(subject);
}

void KisToolMove::buttonPress(KisButtonPressEvent *e)
{
    if (m_subject && e->button() == QMouseEvent::LeftButton) {
        QPoint pos = e->pos().floorQPoint();
        KisImageSP img = m_subject->currentImg();
        KisLayerSP dev;

        if (!img || !(dev = img->activeLayer()))
            return;

        m_dragStart = pos;
        m_strategy.startDrag(pos);
    }
}

void KisToolMove::move(KisMoveEvent *e)
{
    if (m_subject) {
        QPoint pos = e->pos().floorQPoint();
        if((e->state() & Qt::AltButton) || (e->state() & Qt::ControlButton)) {
            if(fabs(pos.x() - m_dragStart.x()) > fabs(pos.y() - m_dragStart.y()))
                pos.setY(m_dragStart.y());
            else
                pos.setX(m_dragStart.x());
        }
        m_strategy.drag(pos);
    }
}

void KisToolMove::buttonRelease(KisButtonReleaseEvent *e)
{
    if (m_subject && e->button() == QMouseEvent::LeftButton) {
        m_strategy.endDrag(e->pos().floorQPoint());
    }
}

void KisToolMove::setup(KActionCollection *collection)
{
    m_action = static_cast<KRadioAction *>(collection->action(name()));

    if (m_action == 0) {
        m_action = new KRadioAction(i18n("&Move"),
                        "tool_move",
                        Qt::SHIFT+Qt::Key_V,
                        this,
                        SLOT(activate()),
                        collection,
                        name());
        m_action->setToolTip(i18n("Move"));
        m_action->setExclusiveGroup("tools");
        m_ownAction = true;
    }
}

