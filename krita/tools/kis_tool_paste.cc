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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include "kis_cursor.h"
#include "kis_doc.h"
#include "kis_painter.h"
#include "kis_tool_paste.h"
#include "kis_view.h"

KisToolPaste::KisToolPaste(KisView *view, KisDoc *doc) : super(view, doc)
{
	m_view = view;
	m_doc = doc;
	m_move = new KisToolMove(view, doc);
	setCursor(KisCursor::crossCursor());
	m_justEntered = false;
}

KisToolPaste::~KisToolPaste()
{
	delete m_move;
}

void KisToolPaste::clear()
{
	leave(0);
}

void KisToolPaste::enter(QEvent *)
{
	activate();
}

void KisToolPaste::leave(QEvent *)
{
	if (m_selection) {
		KisImageSP owner = m_view -> currentImg();
		QRect rc(m_selection -> bounds());

		Q_ASSERT(owner);
		m_selection -> visible(false);
		owner -> unsetSelection(false);
		m_selection = 0;
		owner -> invalidate(rc);
		m_view -> updateCanvas(rc);
	}
}

void KisToolPaste::mousePress(QMouseEvent *)
{
}

void KisToolPaste::mouseRelease(QMouseEvent *e)
{
	if (m_selection) {
		KisImageSP owner = m_view -> currentImg();
		KisPainter gc;

		m_move -> drag(e -> pos());
		owner -> unsetSelection(false);
		gc.begin(m_selection -> parent());
		gc.bitBlt(e -> x(), e -> y(), COMPOSITE_COPY, m_selection.data(), m_selection -> opacity(), 0, 0, m_selection -> width(), m_selection -> height());
		m_selection = 0;
		activate();
	}
}

void KisToolPaste::mouseMove(QMouseEvent *e)
{
	if (!m_selection)
		return;

	if (m_justEntered) {
		m_selection -> move(e -> x(), e -> y());
		m_move -> startDrag(e -> pos());
		m_justEntered = false;
	} else {
		m_move -> drag(e -> pos());
	}
}

void KisToolPaste::activate()
{
	if (!m_selection)
		m_selection = m_doc -> clipboardSelection();

	if (m_selection) {
		KisImageSP owner = m_view -> currentImg();

		if (owner) {
			m_selection -> setImage(owner);
			owner -> unsetSelection(false);
			m_selection -> setParent(owner -> activeDevice());
			owner -> setSelection(m_selection);
			m_justEntered = true;
			m_selection -> visible(true);
		}
	}
}

