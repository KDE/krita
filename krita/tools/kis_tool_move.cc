/*
 *  movetool.cc - part of Krayon
 *
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
 *                1999 Michael Koch    <koch@kde.org>
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
#include <klocale.h>

#include "kis_canvas.h"
#include "kis_cursor.h"
#include "kis_doc.h"
#include "kis_tool_move.h"
#include "kis_view.h"

#if 0
MoveCommand::MoveCommand(KisDoc *doc, int layer, const QPoint& oldpos, const QPoint& newpos)
  : KisCommand(i18n("Move Layer"), doc)
{
	m_layer = layer;
	m_oldPos = oldpos;
	m_newPos = newpos;
	m_doc = doc;
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
	KisImage* img = m_doc->currentImg();

	if (!img) 
		return;

	img->setCurrentLayer( m_layer );
	QRect oldRect = img->getCurrentLayer()->imageExtents();
	img->getCurrentLayer()->moveTo( pos.x(), pos.y() );
	img->markDirty( img->getCurrentLayer()->imageExtents() );
	img->markDirty( oldRect );
}
#endif

MoveTool::MoveTool(KisDoc *doc) : KisTool(doc)
{
	// set custom cursor.
	setCursor();
	m_dragging = false;
}

MoveTool::~MoveTool()
{
}

void MoveTool::mousePress( QMouseEvent *e )
{
	KisImage *img = m_doc -> currentImg();

	if (!img)
		return;

	if (e -> button() != LeftButton)
		return;

	if (!img -> getCurrentLayer() -> visible())
		return;

	QPoint pos = e -> pos();
	QPoint zoomedPos(zoomed(pos));

	if (!img -> getCurrentLayer() -> imageExtents().contains(zoomedPos))
		return;

	m_dragging = true;
	m_dragStart.setX(e -> x());
	m_dragStart.setY(e -> y());
	m_layerStart = img -> getCurrentLayer()->imageExtents().topLeft();
	m_layerPosition = m_layerStart;
}

void MoveTool::mouseMove( QMouseEvent *e )
{
	KisView *view = getCurrentView();
	KisImage* img = m_doc->currentImg();
	if (!img) return;

	if( m_dragging )
	{
		QPoint pos = e->pos();
		QPoint zoomedPos(pos - m_dragStart);
		m_dragPosition = zoomed(zoomedPos);

		QRect oldRect = img->getCurrentLayer()->imageExtents();
		img->getCurrentLayer()->moveBy(m_dragPosition.x(), m_dragPosition.y());
		img->markDirty( img->getCurrentLayer()->imageExtents() );
		img->markDirty( oldRect );

		m_layerPosition = img->getCurrentLayer()->imageExtents().topLeft();
		m_dragStart = e->pos();

		view->slotRefreshPainter();
	}
}


void MoveTool::mouseRelease(QMouseEvent *e )
{
	KisImage* img = m_doc->currentImg();
	if (!img) return;

	if( e->button() != LeftButton ) return;

	if( !m_dragging ) return;

#if 0
	if( m_layerPosition != m_layerStart )
	{
		MoveCommand *moveCommand = new MoveCommand( m_doc,
				img->getCurrentLayerIndex(), m_layerStart, m_layerPosition );

		//m_doc->commandHistory()->addCommand( moveCommand ); //jwc
	}
#endif

	m_dragging = false;
}

void MoveTool::setCursor()
{
	KisView *view = getCurrentView();

	view->kisCanvas()->setCursor( KisCursor::moveCursor() );
	m_cursor = KisCursor::moveCursor();
}

void MoveTool::setupAction(QObject *collection)
{
	KToggleAction *toggle = new KToggleAction(i18n("&Move Tool"), "move", 0, this, SLOT(toolSelect()), collection, "tool_move");

        toggle -> setExclusiveGroup("tools");
}

