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
#include <stdlib.h>
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
	m_view = view;
	m_doc = doc;
	setCursor(KisCursor::moveCursor());
	m_dragging = false;
}

KisToolMove::~KisToolMove()
{
}

void KisToolMove::mousePress(QMouseEvent *e)
{
	QPoint pos = e -> pos();
	KisImageSP img = m_view -> currentImg();
	KisPaintDeviceSP dev;

	if (!img || !(dev = img -> activeDevice()))
		return;

	if (e -> button() == QMouseEvent::LeftButton && dev -> contains(pos))
		startDrag(pos);
}

void KisToolMove::mouseMove(QMouseEvent *e)
{
	drag(e -> pos());
}

void KisToolMove::mouseRelease(QMouseEvent *e)
{
	if (m_dragging && e -> button() == QMouseEvent::LeftButton)
		endDrag(e -> pos());
}

void KisToolMove::keyPress(QKeyEvent *e)
{
	Q_INT32 dx = 0;
	Q_INT32 dy = 0;
	KisImageSP img;
	KisPaintDeviceSP dev;

	if (!(img = m_view -> currentImg()))
		return;

	if (!(dev = img -> activeDevice()))
		return;

	switch (e -> key()) {
	case Qt::Key_Home:
		dx = -dev -> x();
		dy = -dev -> y();
		break;
	case Qt::Key_Left:
		dx = -1;
		break;
	case Qt::Key_Right:
		dx = 1;
		break;
	case Qt::Key_Up:
		dy = -1;
		break;
	case Qt::Key_Down:
		dy = 1;
		break;
	default:
		return;
	}

	if (m_dragging) {
		//
	} else {
		QPoint pt(dev -> x(), dev -> y());
		QPoint dt(dev -> x() + dx, dev -> y() + dy);
		KCommand *cmd = new MoveCommand(m_view, img, img -> activeDevice(), pt, dt);

		dev -> move(dt);
		m_doc -> addCommand(cmd);
	}
}

void KisToolMove::startDrag(const QPoint& pos)
{
	KisImageSP img;
	KisPaintDeviceSP dev;

	if (!(img = m_view -> currentImg()))
		return;

	dev = img -> activeDevice();

	if (!dev || !dev -> visible())
		return;

	m_dragging = true;
	m_dragStart.setX(pos.x());
	m_dragStart.setY(pos.y());
	m_layerStart.setX(dev -> x());
	m_layerStart.setY(dev -> y());
	m_layerPosition = m_layerStart;
}

void KisToolMove::drag(const QPoint& original)
{
	if (m_dragging) {
		KisImageSP img = m_view -> currentImg();
		KisPaintDeviceSP dev;

		if (img && (dev = img -> activeDevice())) {
			QPoint pos = original;
			QRect rc;

			if (pos.x() < 0 || pos.y() < 0)
				return;

			if (pos.x() >= img -> width() || pos.y() >= img -> height())
				return;

			pos -= m_dragStart;
			rc.setRect(dev -> x(), dev -> y(), dev -> width(), dev -> height());
			dev -> move(dev -> x() + pos.x(), dev -> y() + pos.y());
			rc = rc.unite(QRect(dev -> x(), dev -> y(), dev -> width(), dev -> height()));
			rc.setX(QMAX(0, rc.x()));
			rc.setY(QMAX(0, rc.y()));
			img -> invalidate(rc);
			m_layerPosition = QPoint(dev -> x(), dev -> y());
 			m_dragStart = original;
#if 0
			rc.setX(static_cast<Q_INT32>(rc.x() * m_view -> zoom()));
			rc.setY(static_cast<Q_INT32>(rc.y() * m_view -> zoom()));
			rc.setWidth(static_cast<Q_INT32>(rc.width() * m_view -> zoom()));
			rc.setHeight(static_cast<Q_INT32>(rc.height() * m_view -> zoom()));
#endif
			m_view -> updateCanvas(); //rc);
		}
	}
}

void KisToolMove::endDrag(const QPoint& pos, bool undo)
{
	KisImageSP img = m_view -> currentImg();
	KisPaintDeviceSP dev;

	if (img && (dev = img -> activeDevice())) {
		drag(pos);
		m_dragging = false;

		if (undo) {
			KCommand *cmd = new MoveCommand(m_view, img, img -> activeDevice(), m_layerStart, m_layerPosition);

			m_doc -> addCommand(cmd);
		}
	}
}

void KisToolMove::setup()
{
	KToggleAction *toggle;

	toggle = new KToggleAction(i18n("&Move"), "move", 0, this, SLOT(activateSelf()), m_view -> actionCollection(), "tool_move");
	toggle -> setExclusiveGroup("tools");
}

