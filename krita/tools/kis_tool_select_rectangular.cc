/*
 *  Copyright (c) 1999 Michael Koch <koch@kde.org>
 *                2001 John Califf <jcaliff@compuzone.net>
 *                2002 Patrick Julien <freak@codepimps.org>
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include <qpainter.h>
#include <qpen.h>
#include <kaction.h>
#include <klocale.h>
#include "kis_doc.h"
#include "kis_selection.h"
#include "kis_view.h"
#include "kis_tool_select_rectangular.h"

KisToolRectangularSelect::KisToolRectangularSelect(KisView *view, KisDoc *doc) : super(view, doc)
{
	KToggleAction *toggle;

	m_cursor = KisCursor::selectCursor();
	m_view = view;
	m_doc = doc;
	m_selecting = false;
	m_startPos = QPoint(0, 0);
	m_endPos = QPoint(0, 0);
	toggle = new KToggleAction(i18n("&Rectangular Select"), "rectangular", 0, this, 
			SLOT(activateSelf()), view -> actionCollection(), "tool_select_rectangular");
	toggle -> setExclusiveGroup("tools");
}

KisToolRectangularSelect::~KisToolRectangularSelect()
{
}

void KisToolRectangularSelect::paint(QPainter& gc)
{
	if (m_selecting)
		paintOutline(gc, QRect());
}

void KisToolRectangularSelect::paint(QPainter& gc, const QRect& rc)
{
	if (m_selecting)
		paintOutline(gc, rc);
}

void KisToolRectangularSelect::clearSelection()
{
	KisImageSP img = m_view -> currentImg();

	if (img) {
		img -> unsetSelection();
		m_view -> updateCanvas();
	}

	m_startPos = QPoint(0, 0);
	m_endPos = QPoint(0, 0);
	m_selecting = false;
}

void KisToolRectangularSelect::mousePress(QMouseEvent *e)
{
	if (e -> button() == LeftButton) {
		clearSelection();
		m_startPos = e -> pos() * m_view -> zoom();
		m_endPos = e -> pos() * m_view -> zoom();
		m_selecting = true;
	}
}

void KisToolRectangularSelect::mouseMove(QMouseEvent *e)
{
	if (m_selecting) {
		if (m_startPos != m_endPos)
			paintOutline();

		m_endPos = e -> pos();
		m_endPos *= m_view -> zoom();
		paintOutline();
	}
}

void KisToolRectangularSelect::mouseRelease(QMouseEvent *e)
{
	if (m_selecting) {
		if (m_startPos == m_endPos) {
			clearSelection();
		} else {
			KisImageSP img = m_view -> currentImg();
			
			m_startPos /= m_view -> zoom();
			m_endPos = e -> pos();

			if (m_endPos.y() < 0)
				m_endPos.setY(0);

			if (m_endPos.y() > img -> height())
				m_endPos.setY(img -> height());

			if (m_endPos.x() < 0)
				m_endPos.setX(0);

			if (m_endPos.x() > img -> width())
				m_endPos.setX(img -> width());

			if (img) {
				KisPaintDeviceSP parent;
				KisSelectionSP selection;
				QRect rc(m_startPos.x(), m_startPos.y(), m_endPos.x() - m_startPos.x(), m_endPos.y() - m_startPos.y());

				rc = rc.normalize();
				img = m_view -> currentImg();
				Q_ASSERT(img);
				parent = img -> activeDevice();
				selection = new KisSelection(parent, img, "rectangular selection tool box", OPACITY_OPAQUE);
				selection -> setBounds(rc);
				img -> setSelection(selection);
			}
		}

		m_selecting = false;
	}
}

void KisToolRectangularSelect::paintOutline()
{
	QWidget *canvas = m_view -> canvas();
	QPainter gc(canvas);
	QRect rc;

	paintOutline(gc, rc);
}

void KisToolRectangularSelect::paintOutline(QPainter& gc, const QRect&)
{
	RasterOp op = gc.rasterOp();
	QPen old = gc.pen();
	QPen pen(Qt::DotLine);

	gc.setRasterOp(Qt::NotROP);
	gc.setPen(pen);
//	gc.setClipRect(rc);
	gc.drawRect(m_startPos.x(), m_startPos.y(), m_endPos.x() - m_startPos.x(), m_endPos.y() - m_startPos.y());
	gc.setRasterOp(op);
	gc.setPen(old);
}

void KisToolRectangularSelect::activateSelf()
{
	if (m_view)
		m_view -> activateTool(this);
}

void KisToolRectangularSelect::setCursor(const QCursor& cursor)
{
	m_cursor = cursor;
}

void KisToolRectangularSelect::cursor(QWidget *w) const
{
	if (w)
		w -> setCursor(m_cursor);
}

#include "kis_tool_select_rectangular.moc"

