/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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

#include <kdebug.h>

#include "kis_image.h"
#include "kis_canvas_subject.h"
#include "kis_canvas_controller.h"
#include "kis_tool_controller.h"
#include "kis_tool_non_paint.h"
//Added by qt3to4:
#include <QKeyEvent>
#include <QEvent>

KisToolNonPaint::KisToolNonPaint(const QString & UIName)
    : super(UIName)
{
    m_subject = 0;
}

KisToolNonPaint::~KisToolNonPaint()
{
}

void KisToolNonPaint::update(KisCanvasSubject *subject)
{
    m_subject = subject;
}

void KisToolNonPaint::paint(KisCanvasPainter&)
{
}

void KisToolNonPaint::paint(KisCanvasPainter&, const QRect&)
{
}

void KisToolNonPaint::deactivate()
{
}

void KisToolNonPaint::enter(QEvent *)
{
}

void KisToolNonPaint::leave(QEvent *)
{
}

void KisToolNonPaint::buttonPress(KisButtonPressEvent *)
{
}

void KisToolNonPaint::move(KisMoveEvent *)
{
}

void KisToolNonPaint::buttonRelease(KisButtonReleaseEvent *)
{
}

void KisToolNonPaint::doubleClick(KisDoubleClickEvent *)
{
}

void KisToolNonPaint::keyPress(QKeyEvent *)
{
}

void KisToolNonPaint::keyRelease(QKeyEvent *)
{
}

QCursor KisToolNonPaint::cursor()
{
    return m_cursor;
}

void KisToolNonPaint::setCursor(const QCursor& cursor)
{
    m_cursor = cursor;

    if (m_subject) {
        KisToolControllerInterface *controller = m_subject->toolController();

        if (controller && controller->currentTool() == this) {
            m_subject->canvasController()->setCanvasCursor(m_cursor);
        }
    }
}

void KisToolNonPaint::activate()
{
    if (m_subject) {
        KisToolControllerInterface *controller = m_subject->toolController();

        if (controller)
            controller->setCurrentTool(this);
    }
}

void KisToolNonPaint::notifyModified() const
{
    if (m_subject && m_subject->currentImg()) {
        
        m_subject->currentImg()->setModified();
    }
}

#include "kis_tool_non_paint.moc"
