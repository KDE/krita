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

#include <kaction.h>
#include <kcommand.h>
#include <kdebug.h>
#include <klocale.h>

#include "kis_canvas.h"
#include "kis_cursor.h"
#include "kis_doc.h"
#include "kis_tool_move.h"
#include "kis_view.h"

class MoveCommand : public KNamedCommand {
	typedef KNamedCommand super;

public:
	MoveCommand(KisImageSP img, KisPaintDeviceSP device, const QPoint& oldpos, const QPoint& newpos);
	virtual ~MoveCommand();

	virtual void execute();
	virtual void unexecute();

private:
	void moveTo(const QPoint& pos);

private:
	KisPaintDeviceSP m_device;
	QPoint m_oldPos;
	QPoint m_newPos;
	KisImageSP m_img;
};

MoveCommand::MoveCommand(KisImageSP img, KisPaintDeviceSP device, const QPoint& oldpos, const QPoint& newpos) : 
	super(i18n("Move Painting Device"))
{
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
	QRect oldRect = m_device -> imageExtents();

	m_device -> moveTo(pos.x(), pos.y());
	m_img -> markDirty(m_device -> imageExtents());
	m_img -> markDirty(oldRect);
}

MoveTool::MoveTool(KisDoc *doc) : super(doc)
{
	setCursor();
	m_dragging = false;
}

MoveTool::~MoveTool()
{
}

void MoveTool::mousePress(QMouseEvent *e)
{
	if (e -> button() == LeftButton) {
		KisImageSP img = m_doc -> currentImg();
		KisPaintDeviceSP device;

		if (!img)
			return;

		device = img -> getCurrentPaintDevice();

		if (!device || !device -> visible())
			return;

		QPoint pos = zoomed(e -> pos());

		if (!device -> imageExtents().contains(pos))
			return;

		m_dragging = true;
		m_dragStart.setX(e -> x());
		m_dragStart.setY(e -> y());
		m_layerStart = device -> imageExtents().topLeft();
		m_layerPosition = m_layerStart;
	}
}

void MoveTool::mouseMove(QMouseEvent *e)
{
	if (!m_dragging)
		return;

	KisView *view = getCurrentView();
	KisImageSP img = m_doc -> currentImg();
	KisPaintDeviceSP device;

	if (!img) 
		return;

	if (!(device = img -> getCurrentPaintDevice()))
		return;

	QPoint pos = e -> pos();
	QPoint zoomedPos(pos - m_dragStart);

	m_dragPosition = zoomed(zoomedPos);

	QRect oldRect = device -> imageExtents();

	device -> moveBy(m_dragPosition.x(), m_dragPosition.y());
	img -> markDirty(device -> imageExtents());
	img -> markDirty(oldRect);

	m_layerPosition = device -> imageExtents().topLeft();
	m_dragStart = e -> pos();
	view -> slotRefreshPainter();
}

void MoveTool::mouseRelease(QMouseEvent *e)
{
	if (e -> button() == LeftButton) {
		KisImageSP img = m_doc -> currentImg();

		if (!img) 
			return;

		if (!m_dragging) 
			return;

		if (m_layerPosition != m_layerStart) {
			KCommand *cmd = new MoveCommand(img, img -> getCurrentPaintDevice(), m_layerStart, m_layerPosition);

			m_doc -> addCommand(cmd);
		}

		m_dragging = false;
	}
}

void MoveTool::setCursor()
{
	KisView *view = getCurrentView();

	view -> kisCanvas() -> setCursor(KisCursor::moveCursor());
	m_cursor = KisCursor::moveCursor();
}

void MoveTool::setupAction(QObject *collection)
{
	KToggleAction *toggle = new KToggleAction(i18n("&Move Tool"), "move", 0, this, SLOT(toolSelect()), collection, "tool_move");

        toggle -> setExclusiveGroup("tools");
}

