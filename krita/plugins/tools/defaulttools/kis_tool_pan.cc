/*
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
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

#include <kaction.h>
#include <klocale.h>

#include "kis_canvas_controller.h"
#include "kis_canvas_subject.h"
#include "kis_cursor.h"
#include "kis_tool_pan.h"
#include "kis_button_press_event.h"
#include "kis_button_release_event.h"
#include "kis_move_event.h"

KisToolPan::KisToolPan()
    : super(i18n("Pan Tool"))
{
    setObjectName("tool_pan");
    m_subject = 0;
    m_dragging = false;
    m_openHandCursor = KisCursor::openHandCursor();
    m_closedHandCursor = KisCursor::closedHandCursor();
    setCursor(m_openHandCursor);
}

KisToolPan::~KisToolPan()
{
}

void KisToolPan::update(KisCanvasSubject *subject)
{
    m_subject = subject;
    super::update(m_subject);
}

void KisToolPan::buttonPress(KisButtonPressEvent *e)
{
    if (m_subject && !m_dragging && e->button() == Qt::LeftButton) {
        KisCanvasController *controller = m_subject->canvasController();

        m_origScrollX = controller->horzValue();
        m_origScrollY = controller->vertValue();
        m_dragPos = controller->windowToView(e->pos());
        m_dragging = true;
        setCursor(m_closedHandCursor);
    }
}

void KisToolPan::move(KisMoveEvent *e)
{
    if (m_subject && m_dragging) {
        KisCanvasController *controller = m_subject->canvasController();

        KisPoint currPos = controller->windowToView(e->pos());
        KisPoint delta = currPos - m_dragPos;
        controller->scrollTo(m_origScrollX - delta.floorX(), m_origScrollY - delta.floorY());
    }
}

void KisToolPan::buttonRelease(KisButtonReleaseEvent *e)
{
    if (m_subject && m_dragging && e->button() == Qt::LeftButton) {
        setCursor(m_openHandCursor);
        m_dragging = false;
    }
}

void KisToolPan::setup(KActionCollection *collection)
{
    m_action = collection->action(objectName());

    if (m_action == 0) {
        m_action = new KAction(KIcon("tool_pan"),
                               i18n("&Pan"),
                               collection,
                               objectName());
        m_action->setShortcut(Qt::SHIFT+Qt::Key_H);
        connect(m_action, SIGNAL(triggered()), this, SLOT(activate()));
        m_action->setToolTip(i18n("Pan"));
        m_action->setActionGroup(actionGroup());
        m_ownAction = true;
    }
}

#include "kis_tool_pan.moc"

