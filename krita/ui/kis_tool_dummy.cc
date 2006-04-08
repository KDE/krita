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

#include <qwidget.h>
#include <qstring.h>
#include <kaction.h>
#include <qlabel.h>

#include <kaction.h>
#include <klocale.h>

#include "kis_canvas_controller.h"
#include "kis_canvas_subject.h"
#include "kis_cursor.h"
#include "kis_tool_dummy.h"
#include "kis_button_press_event.h"
#include "kis_button_release_event.h"
#include "kis_move_event.h"

KisToolDummy::KisToolDummy()
    : super(i18n("No Active Tool"))
{
    setName("tool_dummy");
    m_subject = 0;
    m_dragging = false;
    m_optionWidget = 0;
    setCursor(Qt::forbiddenCursor);
}

KisToolDummy::~KisToolDummy()
{
}

void KisToolDummy::update(KisCanvasSubject *subject)
{
    m_subject = subject;
    super::update(m_subject);
}

void KisToolDummy::buttonPress(KisButtonPressEvent *e)
{
    if (m_subject && !m_dragging && e->button() == Qt::LeftButton) {
        KisCanvasController *controller = m_subject->canvasController();

        m_origScrollX = controller->horzValue();
        m_origScrollY = controller->vertValue();
        m_dragPos = controller->windowToView(e->pos());
        m_dragging = true;
    }
}

void KisToolDummy::move(KisMoveEvent *e)
{
    if (m_subject && m_dragging) {
        KisCanvasController *controller = m_subject->canvasController();

        KisPoint currPos = controller->windowToView(e->pos());
        KisPoint delta = currPos - m_dragPos;
        controller->scrollTo(m_origScrollX - delta.floorX(), m_origScrollY - delta.floorY());
    }
}

void KisToolDummy::buttonRelease(KisButtonReleaseEvent *e)
{
    if (m_subject && m_dragging && e->button() == Qt::LeftButton) {
        m_dragging = false;
    }
}

void KisToolDummy::setup(KActionCollection *collection)
{
    m_action = collection->action(name());

    if (m_action == 0) {
        m_action = new KAction(i18n("&Dummy"), "tool_dummy", Qt::SHIFT+Qt::Key_H, this, SLOT(activate()), collection, name());
        m_action->setActionGroup(actionGroup());
        m_ownAction = true;
    }
}


QWidget* KisToolDummy::createOptionWidget(QWidget* parent)
{
    m_optionWidget = new QLabel(i18n("Layer is locked or invisible."), parent);
    m_optionWidget->setCaption(i18n("No Active Tool"));
    m_optionWidget->setAlignment(Qt::AlignCenter);
    return m_optionWidget;
}

QWidget* KisToolDummy::optionWidget()
{
    return m_optionWidget;
}


#include "kis_tool_dummy.moc"
