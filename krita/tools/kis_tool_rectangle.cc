/*
 *  kis_tool_rectangle.cc - part of Krita
 *
 *  Copyright (c) 2000 John Califf <jcaliff@compuzone.net>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Clarence Dang <dang@k.org>
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
#include <kdebug.h>
#include <klocale.h>

#include "kis_doc.h"
#include "kis_view.h"
#include "kis_painter.h"
#include "kis_canvas_subject.h"
#include "kis_canvas_controller.h"
#include "kis_tool_rectangle.h"
#include "kis_button_press_event.h"
#include "kis_button_release_event.h"
#include "kis_move_event.h"
#include "kis_brushop.h"

KisToolRectangle::KisToolRectangle()
	: super(),
          m_dragging (false),
          m_currentImage (0)
{
	setName("tool_rectangle");
// 	// initialize rectangle tool settings
// 	m_lineThickness = 4;
// 	m_opacity = 255;
// 	m_usePattern = false;
// 	m_useGradient = false;
// 	m_fillSolid = false;
}

KisToolRectangle::~KisToolRectangle()
{
}

void KisToolRectangle::update (KisCanvasSubject *subject)
{
//         kdDebug (40001) << "KisToolRectangle::update(" << subject << ")" << endl;
        super::update (subject);
        if (m_subject)
            m_currentImage = m_subject->currentImg ();
}

void KisToolRectangle::buttonPress(KisButtonPressEvent *event)
{
// 	kdDebug (40001) << "KisToolRectangle::buttonPress" << event->pos () << endl;
	if (m_currentImage && event -> button() == LeftButton) {
		m_dragging = true;
		m_dragStart = event -> pos();
		m_dragEnd = event -> pos();
	}
}

void KisToolRectangle::move(KisMoveEvent *event)
{
// 	kdDebug (40001) << "KisToolRectangle::move" << event->pos () << endl;
	if (m_dragging) {
		// erase old lines on canvas
		draw(m_dragStart, m_dragEnd);
		// get current mouse position
		m_dragEnd = event -> pos();
		// draw new lines on canvas
		draw(m_dragStart, m_dragEnd);
	}
}

void KisToolRectangle::buttonRelease(KisButtonReleaseEvent *event)
{
	if (!m_subject)
		return;

	if (m_dragging && event -> button() == LeftButton) {
		// erase old lines on canvas
		draw(m_dragStart, m_dragEnd);
		m_dragging = false;

		m_dragEnd = event -> pos();
		if (m_dragStart == m_dragEnd)
			return;

		if (!m_currentImage)
			return;

		KisPaintDeviceSP device = m_currentImage->activeDevice ();
		KisPainter painter (device);
		painter.beginTransaction (i18n ("rectangle"));
		
		KisPaintOp * op = new KisBrushOp(&painter); // XXX: add all paintops to the config widget
		painter.setPaintOp(op);
		painter.setPaintColor(m_subject -> fgColor());
		painter.setBrush(m_subject -> currentBrush());
		//painter.setOpacity(m_opacity);
		//painter.setCompositeOp(m_compositeOp);

		painter.paintRect(m_dragStart, m_dragEnd, PRESSURE_DEFAULT/*event -> pressure()*/, event -> xTilt(), event -> yTilt());
		m_currentImage -> notify( painter.dirtyRect() );
		notifyModified();

		KisUndoAdapter *adapter = m_currentImage -> undoAdapter();
		if (adapter) {
			adapter -> addCommand(painter.endTransaction());
		}
	}
}

void KisToolRectangle::draw(const KisPoint& start, const KisPoint& end )
{
	if (!m_subject)
		return;

	KisCanvasControllerInterface *controller = m_subject->canvasController ();
// 	kdDebug (40001) << "KisToolRectangle::draw(" << start << "," << end << ")"
// 			<< " windowToView: start=" << controller->windowToView (start)
// 			<< " windowToView: end=" << controller->windowToView (end)
// 			<< endl;
	QWidget *canvas = controller->canvas ();
	QPainter p (canvas);

	p.setRasterOp (Qt::NotROP);
	p.drawRect (QRect (controller->windowToView (start).floorQPoint(), controller->windowToView (end).floorQPoint()));
	p.end ();
}

void KisToolRectangle::setup(KActionCollection *collection)
{
	m_action = static_cast<KRadioAction *>(collection -> action(name()));

	if (m_action == 0) {
		m_action = new KRadioAction(i18n("Tool &Rectangle"),
					    "rectangle",
					    Qt::Key_F6,
					    this,
					    SLOT(activate()),
					    collection,
					    name());
		m_action -> setExclusiveGroup("tools");
		m_ownAction = true;
	}
}



#include "kis_tool_rectangle.moc"
