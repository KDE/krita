/*
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
 *                1999 Michael Koch    <koch@kde.org>
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
#include <qpoint.h>
#include <kaction.h>
#include <kcommand.h>
#include <klocale.h>
#include <koColor.h>
#include "kis_cursor.h"
#include "kis_doc.h"
#include "kis_image.h"
#include "kis_paint_device.h"
#include "kis_view.h"
#include "kis_tool_memento.h"
#include "kis_tool_move.h"
#include "kis_tool_move.moc"

namespace {
	class MoveCommand : public KNamedCommand {
		typedef KNamedCommand super;

	public:
		MoveCommand(KisView *view, KisImageSP img, KisPaintDeviceSP device, const QPoint& oldpos, const QPoint& newpos);
		virtual ~MoveCommand();

		virtual void execute();
		virtual void unexecute();

	private:
		void moveTo(const QPoint& pos);

	private:
		KisView *m_view;
		KisPaintDeviceSP m_device;
		QPoint m_oldPos;
		QPoint m_newPos;
		KisImageSP m_img;
	};


	MoveCommand::MoveCommand(KisView *view, KisImageSP img, KisPaintDeviceSP device, const QPoint& oldpos, const QPoint& newpos) :
		super(i18n("Move Painting Device"))
	{
		m_view = view;
		m_img = img;
		m_device = device;
		m_oldPos = oldpos;
		m_newPos = newpos;
	}

	MoveCommand::~MoveCommand()
	{
	}

	void MoveCommand::execute()
	{
		moveTo(m_newPos);
	}

	void MoveCommand::unexecute()
	{
		moveTo(m_oldPos);
	}

	void MoveCommand::moveTo(const QPoint& pos)
	{
		QRect rc;

		rc.setRect(m_device -> x(), m_device -> y(), m_device -> width(), m_device -> height());
		m_device -> move(pos.x(), pos.y());
		rc |= QRect(m_device -> x(), m_device -> y(), m_device -> width(), m_device -> height());
		m_img -> invalidate(rc);
		m_view -> updateCanvas(rc);
	}
}

KisToolMove::KisToolMove(KisView *view, KisDoc *doc) : super(view, doc)
{
	KToggleAction *toggle;

	m_view = view;
	m_doc = doc;
	m_cursor = KisCursor::moveCursor();
	m_dragging = false;
	toggle = new KToggleAction(i18n("&Move"), "move", 0, this, SLOT(activateSelf()), view -> actionCollection(), "tool_move");
	toggle -> setExclusiveGroup("tools");
}

KisToolMove::~KisToolMove()
{
}

void KisToolMove::mousePress(QMouseEvent *e)
{
	KisImageSP img;
	KisPaintDeviceSP dev;
	QPoint pos;

	if (e -> button() != QMouseEvent::LeftButton)
		return;

	if (!(img = m_view -> currentImg()))
		return;

	dev = img -> activeDevice();

	if (!dev || !dev -> visible())
		return;

	pos = e -> pos();

	if (!dev -> contains(pos))
		return;

	m_dragging = true;
	m_dragStart.setX(e -> x());
	m_dragStart.setY(e -> y());
	m_layerStart.setX(dev -> x());
	m_layerStart.setY(dev -> y());
	m_layerPosition = m_layerStart;
}

void KisToolMove::mouseMove(QMouseEvent *e)
{
	if (m_dragging) {
		KisImageSP img = m_view -> currentImg();
		KisPaintDeviceSP dev;

		if (img && (dev = img -> activeDevice())) {
			QPoint pos = e -> pos();
			QRect rc;

			if (pos.x() < 0 || pos.y() < 0)
				return;

			if (pos.x() >= img -> width() || pos.y() >= img -> height())
				return;

			pos -= m_dragStart;
			rc.setRect(dev -> x(), dev -> y(), dev -> width(), dev -> height());
			dev -> move(dev -> x() + pos.x(), dev -> y() + pos.y());
			rc |= QRect(dev -> x(), dev -> y(), dev -> width(), dev -> height());
			img -> invalidate(rc);

			m_layerPosition = QPoint(dev -> x(), dev -> y());
			m_dragStart = e -> pos();
			rc.setX(static_cast<Q_INT32>(rc.x() * m_view -> zoom()));
			rc.setY(static_cast<Q_INT32>(rc.y() * m_view -> zoom()));
			rc.setWidth(static_cast<Q_INT32>(rc.width() * m_view -> zoom()));
			rc.setHeight(static_cast<Q_INT32>(rc.height() * m_view -> zoom()));
			m_view -> updateCanvas(); //rc);
		}
	}
}

void KisToolMove::mouseRelease(QMouseEvent *e)
{
	if (m_dragging && e -> button() == QMouseEvent::LeftButton) {
		KisImageSP img = m_view -> currentImg();
		KisPaintDeviceSP dev;

		if (img && (dev = img -> activeDevice())) {
			KCommand *cmd;

			mouseMove(e);
			m_dragging = false;
			cmd = new MoveCommand(m_view, img, img -> activeDevice(), m_layerStart, m_layerPosition);
			m_doc -> addCommand(cmd);
		}
	}
}

void KisToolMove::keyPress(QKeyEvent *)
{
}

void KisToolMove::activateSelf()
{
	if (m_view)
		m_view -> activateTool(this);
}

void KisToolMove::setCursor(const QCursor& cursor)
{
	m_cursor = cursor;
}

void KisToolMove::cursor(QWidget *w) const
{
	if (w)
		w -> setCursor(m_cursor);
}

