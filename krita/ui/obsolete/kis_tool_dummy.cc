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

#include <QWidget>
#include <QString>
#include <kaction.h>
#include <QLabel>
#include <kicon.h>

#include <kaction.h>
#include <kactioncollection.h>
#include <klocale.h>

#include "kis_canvas_controller.h"
#include "kis_canvas_subject.h"
#include "kis_cursor.h"
#include "kis_tool_dummy.h"
#include "KoPointerEvent.h"
#include "KoPointerEvent.h"
#include "KoPointerEvent.h"

KisToolDummy::KisToolDummy()
    : super(i18n("No Active Tool"))
{
    setObjectName("tool_dummy");
    m_subject = 0;
    m_dragging = false;
    m_optionWidget = 0;
    setCursor(Qt::ForbiddenCursor);
}

KisToolDummy::~KisToolDummy()
{
}

void KisToolDummy::update(KisCanvasSubject *subject)
{
    m_subject = subject;
    super::update(m_subject);
}

void KisToolDummy::buttonPress(KoPointerEvent *e)
{
    if (m_subject && !m_dragging && e->button() == Qt::LeftButton) {
        KisCanvasController *controller = m_subject->canvasController();

        m_origScrollX = controller->horzValue();
        m_origScrollY = controller->vertValue();
        m_dragPos = controller->windowToView(e->pos());
        m_dragging = true;
    }
}

void KisToolDummy::move(KoPointerEvent *e)
{
    if (m_subject && m_dragging) {
        KisCanvasController *controller = m_subject->canvasController();

        KoPoint currPos = controller->windowToView(e->pos());
        KoPoint delta = currPos - m_dragPos;
        controller->scrollTo(m_origScrollX - delta.floorX(), m_origScrollY - delta.floorY());
    }
}

void KisToolDummy::buttonRelease(KoPointerEvent *e)
{
    if (m_subject && m_dragging && e->button() == Qt::LeftButton) {
        m_dragging = false;
    }
}

void KisToolDummy::setup(KActionCollection *collection)
{
    m_action = collection->action(objectName());

    if (m_action == 0) {
        m_action = new KAction(KIcon("tool_dummy"), i18n("&Dummy"), collection, objectName());
        m_action->setShortcut(Qt::SHIFT+Qt::Key_H);
        connect(m_action, SIGNAL(triggered()), this, SLOT(activate()));
        m_action->setActionGroup(actionGroup());
        m_ownAction = true;
    }
}


QWidget* KisToolDummy::createOptionWidget()
{
    m_optionWidget = new QLabel(i18n("Layer is locked or invisible."), parent);
    m_optionWidget->setWindowTitle(i18n("No Active Tool"));
    m_optionWidget->setAlignment(Qt::AlignCenter);
    return m_optionWidget;
}

QWidget* KisToolDummy::optionWidget()
{
    return m_optionWidget;
}


#include "kis_tool_dummy.moc"
