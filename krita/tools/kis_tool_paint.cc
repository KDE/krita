/*
 *  Copyright (c) 2003 Boudewijn Rempt
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <qwidget.h>
#include <kdebug.h>
#include "kis_canvas_subject.h"
#include "kis_tool_controller.h"
#include "kis_tool_paint.h"

KisToolPaint::KisToolPaint()
{
	m_subject = 0;
}

KisToolPaint::~KisToolPaint()
{
}

void KisToolPaint::update(KisCanvasSubject *subject)
{
	m_subject = subject;
}

void KisToolPaint::paint(QPainter&)
{
}

void KisToolPaint::paint(QPainter&, const QRect&)
{
}

void KisToolPaint::clear()
{
}

void KisToolPaint::clear(const QRect&)
{
}

void KisToolPaint::enter(QEvent *)
{
}

void KisToolPaint::leave(QEvent *)
{
}

void KisToolPaint::mousePress(QMouseEvent *)
{
}

void KisToolPaint::mouseMove(QMouseEvent *)
{
}

void KisToolPaint::mouseRelease(QMouseEvent *)
{
}

void KisToolPaint::tabletEvent(QTabletEvent *)
{
//     kdDebug() << "received tablet event at position ("
//               << e->pos().x()
//               << ", "
//               << e->pos().y()
//               << ")"
//               << endl;
}


void KisToolPaint::keyPress(QKeyEvent *)
{
}

void KisToolPaint::keyRelease(QKeyEvent *)
{
}

QWidget* KisToolPaint::createoptionWidget(QWidget* parent)
{
	return 0;
}

QWidget* KisToolPaint::optionWidget()
{
	return 0;
}

void KisToolPaint::cursor(QWidget *w) const
{
	if (w)
		w -> setCursor(m_cursor);
}

void KisToolPaint::setCursor(const QCursor& cursor)
{
	m_cursor = cursor;
}

void KisToolPaint::activate()
{
	if (m_subject) {
		KisToolControllerInterface *controller = m_subject -> toolController();

		if (controller)
			controller -> setCurrentTool(this);
	}
}

#include "kis_tool_paint.moc"
