/*
 *  kis_tool_select_elliptical.cc -- part of Krita
 *
 *  Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
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
#include <qpen.h>

#include <kdebug.h>
#include <kaction.h>
#include <kcommand.h>
#include <klocale.h>

#include "kis_canvas_controller.h"
#include "kis_canvas_subject.h"
#include "kis_cursor.h"
#include "kis_image.h"
#include "kis_floatingselection.h"
#include "kis_tool_select_elliptical.h"
#include "kis_undo_adapter.h"
#include "kis_button_press_event.h"
#include "kis_button_release_event.h"
#include "kis_move_event.h"

namespace {
	class EllipseSelectCmd : public KNamedCommand {
		typedef KNamedCommand super;

	public:
		EllipseSelectCmd(KisFloatingSelectionSP selection);
		virtual ~EllipseSelectCmd();

	public:
		virtual void execute();
		virtual void unexecute();

	private:
		KisFloatingSelectionSP m_selection;
		KisImageSP m_owner;
	};

	EllipseSelectCmd::EllipseSelectCmd(KisFloatingSelectionSP selection) : super(i18n("Elliptical Selection"))
	{
		m_selection = selection;
		m_owner = selection -> image();
	}

	EllipseSelectCmd::~EllipseSelectCmd()
	{
	}

	void EllipseSelectCmd::execute()
	{
		m_selection -> clearParentOnMove(true);
		m_owner -> setFloatingSelection(m_selection);
		m_owner -> notify(m_selection -> bounds());
	}

	void EllipseSelectCmd::unexecute()
	{
		m_owner -> unsetFloatingSelection(false);
	}
}

KisToolSelectElliptical::KisToolSelectElliptical()
{
	setName("tool_select_elliptical");
	setCursor(KisCursor::selectCursor());

	m_subject = 0;
	m_selecting = false;
	m_startPos = QPoint(0, 0);
	m_endPos = QPoint(0, 0);
}

KisToolSelectElliptical::~KisToolSelectElliptical()
{
}

void KisToolSelectElliptical::update(KisCanvasSubject *subject)
{
	m_subject = subject;
	super::update(m_subject);
}

void KisToolSelectElliptical::paint(QPainter& gc)
{
	if (m_selecting)
		paintOutline(gc, QRect());
}

void KisToolSelectElliptical::paint(QPainter& gc, const QRect& rc)
{
	if (m_selecting)
		paintOutline(gc, rc);
}

void KisToolSelectElliptical::clearSelection()
{
	if (m_subject) {
		KisCanvasControllerInterface *controller = m_subject -> canvasController();
		KisImageSP img = m_subject -> currentImg();

		Q_ASSERT(controller);

		if (img && img -> floatingSelection().data() != 0) {
			img -> unsetFloatingSelection();
                        controller -> canvas() -> update();
		}

		m_startPos = QPoint(0, 0);
		m_endPos = QPoint(0, 0);
		m_selecting = false;
	}
}

void KisToolSelectElliptical::buttonPress(KisButtonPressEvent *e)
{
	if (m_subject) {
		KisImageSP img = m_subject -> currentImg();

		if (img && img -> activeDevice() && e -> button() == LeftButton) {
			clearSelection();
			m_startPos = e -> pos();
			m_endPos = e -> pos();
			m_selecting = true;
		}
	}
}

void KisToolSelectElliptical::move(KisMoveEvent *e)
{
	if (m_subject && m_selecting) {

		if (m_startPos != m_endPos)
			paintOutline();

		m_endPos = e -> pos(); //controller -> windowToView(e -> pos());
		paintOutline();
	}
}

void KisToolSelectElliptical::buttonRelease(KisButtonReleaseEvent */*e*/)
{
	clearSelection();
	m_selecting = false;

// 	if (m_subject && m_selecting) {
// 		if (m_startPos == m_endPos) {
// 			clearSelection();
// 		} else {
// 			KisImageSP img = m_subject -> currentImg();

// 			if (!img)
// 				return;

// 			m_endPos = e -> pos();

// 			if (m_endPos.y() < 0)
// 				m_endPos.setY(0);

// 			if (m_endPos.y() > img -> height())
// 				m_endPos.setY(img -> height());

// 			if (m_endPos.x() < 0)
// 				m_endPos.setX(0);

// 			if (m_endPos.x() > img -> width())
// 				m_endPos.setX(img -> width());

// 			if (img) {
// 				KisPaintDeviceSP parent;
// 				KisFloatingSelectionSP selection;

//                                 QRect rc(m_startPos, m_endPos);

// 				parent = img -> activeDevice();

// 				if (parent) {
// 					rc = rc.normalize();
// 					selection = new KisFloatingSelection(parent, img, "elliptical selection tool frame", OPACITY_OPAQUE);
// 					selection -> setBounds(rc);
// 					img -> setSelection(selection);

// 					if (img -> undoAdapter())
// 						img -> undoAdapter() -> addCommand(new RectSelectCmd(selection));
// 				}
// 			}
// 		}
//
// 		m_selecting = false;
// 	}
}

void KisToolSelectElliptical::paintOutline()
{
	if (m_subject) {
		KisCanvasControllerInterface *controller = m_subject -> canvasController();
		QWidget *canvas = controller -> canvas();
		QPainter gc(canvas);
		QRect rc;

		paintOutline(gc, rc);
	}
}

void KisToolSelectElliptical::paintOutline(QPainter& gc, const QRect&)
{
	if (m_subject) {
		KisCanvasControllerInterface *controller = m_subject -> canvasController();
		RasterOp op = gc.rasterOp();
		QPen old = gc.pen();
		QPen pen(Qt::DotLine);
		QPoint start;
		QPoint end;

		Q_ASSERT(controller);
		start = controller -> windowToView(m_startPos).floorQPoint();
		end = controller -> windowToView(m_endPos).floorQPoint();

		gc.setRasterOp(Qt::NotROP);
		gc.setPen(pen);
		gc.drawEllipse(QRect(start, end));
		gc.setRasterOp(op);
		gc.setPen(old);
	}
}

void KisToolSelectElliptical::setup(KActionCollection *collection)
{
	m_action = static_cast<KRadioAction *>(collection -> action(name()));

	if (m_action == 0) {
		m_action = new KRadioAction(i18n("Tool &Elliptical Select"), 
					    "elliptical" , 
					    Qt::Key_J, 
					    this, 
					    SLOT(activate()),
					    collection, 
					    name());
		m_action -> setExclusiveGroup("tools");
		m_ownAction = true;
	}
}


#include "kis_tool_select_elliptical.moc"
