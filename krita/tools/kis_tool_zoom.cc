/*
 *  Copyright (c) 1999 Matthias Elter <me@kde.org>
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

#include <qpainter.h>
#include <kaction.h>
#include <klocale.h>
#include "kis_canvas_controller.h"
#include "kis_canvas_subject.h"
#include "kis_cursor.h"
#include "kis_tool_zoom.h"
#include "kis_tool_zoom.moc"

KisToolZoom::KisToolZoom()
{
	setName("tool_zoom");
	m_subject = 0;
	m_dragging = false;
	m_startPos = QPoint(0, 0);
	m_endPos = QPoint(0, 0);
	setCursor(KisCursor::zoomCursor());
}

KisToolZoom::~KisToolZoom()
{
}

void KisToolZoom::update(KisCanvasSubject *subject)
{
	m_subject = subject;
	super::update(m_subject);
}

void KisToolZoom::paint(QPainter& gc)
{
	if (m_dragging)
		paintOutline(gc, QRect());
}

void KisToolZoom::paint(QPainter& gc, const QRect& rc)
{
	if (m_dragging)
		paintOutline(gc, rc);
}

void KisToolZoom::mousePress(QMouseEvent *e)
{
	if (m_subject && !m_dragging) {
		if (e -> button() == Qt::LeftButton) {
			m_startPos = e -> pos();
			m_endPos = e -> pos();
			m_dragging = true;
		}
		else if (e -> button() == Qt::RightButton) {

			KisCanvasControllerInterface *controller = m_subject -> canvasController();
			controller -> zoomOut(e -> pos().x(), e -> pos().y());
		}
	}
}

void KisToolZoom::mouseMove(QMouseEvent *e)
{
	if (m_subject && m_dragging) {
		if (m_startPos != m_endPos)
			paintOutline();

		m_endPos = e -> pos();
		paintOutline();
	}
}

void KisToolZoom::mouseRelease(QMouseEvent *e)
{
	if (m_subject && m_dragging && e -> button() == Qt::LeftButton) {

		KisCanvasControllerInterface *controller = m_subject -> canvasController();
		m_endPos = e -> pos();
		m_dragging = false;

		if (m_startPos == m_endPos) {
			controller -> zoomIn(e -> pos().x(), e -> pos().y());
		} else {
			controller -> zoomTo(QRect(m_startPos, m_endPos));
		}
	}
}

void KisToolZoom::paintOutline()
{
	if (m_subject) {
		KisCanvasControllerInterface *controller = m_subject -> canvasController();
		QWidget *canvas = controller -> canvas();
		QPainter gc(canvas);
		QRect rc;

		paintOutline(gc, rc);
	}
}

void KisToolZoom::paintOutline(QPainter& gc, const QRect&)
{
	if (m_subject) {
		KisCanvasControllerInterface *controller = m_subject -> canvasController();
		RasterOp op = gc.rasterOp();
		QPen old = gc.pen();
		QPen pen(Qt::DotLine);
		QPoint start;
		QPoint end;

		Q_ASSERT(controller);
		start = controller -> windowToView(m_startPos);
		end = controller -> windowToView(m_endPos);

		gc.setRasterOp(Qt::NotROP);
		gc.setPen(pen);
		gc.drawRect(QRect(start, end));
		gc.setRasterOp(op);
		gc.setPen(old);
	}
}

void KisToolZoom::setup(KActionCollection *collection)
{
	m_action = static_cast<KRadioAction *>(collection -> action(name()));

	if (m_action == 0) {
		m_action = new KRadioAction(i18n("&Zoom Tool"), "viewmag", 0, this, SLOT(activate()), collection, name());
		m_action -> setExclusiveGroup("tools");
		m_ownAction = true;
	}
}

