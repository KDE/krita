/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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
#include "kis_tool_non_paint.h"
#include "kis_tool_non_paint.moc"

KisToolNonPaint::KisToolNonPaint()
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

void KisToolNonPaint::paint(QPainter&)
{
}

void KisToolNonPaint::paint(QPainter&, const QRect&)
{
}

void KisToolNonPaint::clear()
{
}

void KisToolNonPaint::clear(const QRect&)
{
}

void KisToolNonPaint::enter(QEvent *)
{
}

void KisToolNonPaint::leave(QEvent *)
{
}

void KisToolNonPaint::mousePress(QMouseEvent *)
{
}

void KisToolNonPaint::mouseMove(QMouseEvent *)
{
}

void KisToolNonPaint::mouseRelease(QMouseEvent *)
{
}

void KisToolNonPaint::tabletEvent(QTabletEvent * /*e*/)
{
//     kdDebug() << "received tablet event at position ("
//               << e->pos().x()
//               << ", "
//               << e->pos().y()
//               << ")"
//               << endl;
}


void KisToolNonPaint::keyPress(QKeyEvent *)
{
}

void KisToolNonPaint::keyRelease(QKeyEvent *)
{
}

KDialog *KisToolNonPaint::options(QWidget * /*parent*/)
{
	return 0;
}

QWidget* KisToolNonPaint::optionWidget(QWidget* parent)
{
	return 0;
}

void KisToolNonPaint::cursor(QWidget *w) const
{
	if (w)
		w -> setCursor(m_cursor);
}

void KisToolNonPaint::setCursor(const QCursor& cursor)
{
	m_cursor = cursor;
}

void KisToolNonPaint::activate()
{
	if (m_subject) {
		KisToolControllerInterface *controller = m_subject -> toolController();

		if (controller)
			controller -> setCurrentTool(this);
	}
}

