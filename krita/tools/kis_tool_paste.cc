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
#include <kaction.h>
#include <kcommand.h>
#include <klocale.h>
#include "kis_paint_device.h"
#include "kis_cursor.h"
#include "kis_doc.h"
#include "kis_painter.h"
#include "kis_tool_paste.h"
#include "kis_view.h"

KisToolPaste::KisToolPaste(KisView *view, KisDoc *doc) : super(view, doc), KisStrategyMove(view, doc)
{
	m_view = view;
	m_doc = doc;
	setCursor(KisCursor::crossCursor());
	m_justEntered = false;
}

KisToolPaste::~KisToolPaste()
{
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
		owner -> unsetSelection(false);
		m_selection -> visible(false);
		m_selection = 0;
		owner -> notify(); //rc);
	}
}

void KisToolPaste::mouseRelease(QMouseEvent *e)
{
	if (m_selection) {
		KisImageSP owner = m_view -> currentImg();
		KisPaintDeviceSP dev;
		KisPainter gc;

		drag(e -> pos());
		owner -> unsetSelection(false);
		dev = owner -> activeDevice();
		gc.begin(m_selection -> parent());
		gc.beginTransaction(i18n("Paste Selection"));
		gc.bitBlt(e -> x() - dev -> x(), 
				e -> y() - dev -> y(), 
				COMPOSITE_COPY, 
				m_selection.data(), 
				m_selection -> opacity(), 
				0, 0, m_selection -> width(), m_selection -> height());
		m_doc -> addCommand(gc.endTransaction());
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
		startDrag(e -> pos());
		m_justEntered = false;
	} else {
		drag(e -> pos());
	}
}

void KisToolPaste::activate()
{
	m_toggle -> setChecked(true);

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

void KisToolPaste::setup()
{
	m_toggle = new KToggleAction(i18n("&Paste Tool"), "editpaste", 0, this, SLOT(activateSelf()), m_view -> actionCollection(), "tool_paste");
	m_toggle -> setExclusiveGroup("tools");
}

