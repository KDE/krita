/*
 *  kis_tool_select_rectangular.cc -- part of Krita
 *
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
#include "kis_tool_select_rectangular.h"
#include "kis_undo_adapter.h"
#include "kis_button_press_event.h"
#include "kis_button_release_event.h"
#include "kis_move_event.h"

namespace {
	class RectSelectCmd : public KNamedCommand {
		typedef KNamedCommand super;

	public:
		RectSelectCmd(KisFloatingSelectionSP selection);
		virtual ~RectSelectCmd();

	public:
		virtual void execute();
		virtual void unexecute();

	private:
		KisFloatingSelectionSP m_selection;
		KisImageSP m_owner;
	};

	RectSelectCmd::RectSelectCmd(KisFloatingSelectionSP selection) : super(i18n("Rectangular Selection"))
	{
		m_selection = selection;
		m_owner = selection -> image();
	}

	RectSelectCmd::~RectSelectCmd()
	{
	}

	void RectSelectCmd::execute()
	{
		m_selection -> clearParentOnMove(true);
		m_owner -> setFloatingSelection(m_selection);
		m_owner -> notify(m_selection -> bounds());
	}

	void RectSelectCmd::unexecute()
	{
		m_owner -> unsetFloatingSelection(false);
	}
}

KisToolSelectRectangular::KisToolSelectRectangular()
{
	setName("tool_select_rectangular");
	setCursor(KisCursor::selectCursor());
	m_subject = 0;
	m_selecting = false;
	m_startPos = QPoint(0, 0);
	m_endPos = QPoint(0, 0);
}

KisToolSelectRectangular::~KisToolSelectRectangular()
{
}

void KisToolSelectRectangular::update(KisCanvasSubject *subject)
{
	m_subject = subject;
	super::update(m_subject);
}

void KisToolSelectRectangular::paint(QPainter& gc)
{
	if (m_selecting)
		paintOutline(gc, QRect());
}

void KisToolSelectRectangular::paint(QPainter& gc, const QRect& rc)
{
	if (m_selecting)
		paintOutline(gc, rc);
}

void KisToolSelectRectangular::clearSelection()
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

void KisToolSelectRectangular::buttonPress(KisButtonPressEvent *e)
{
	if (m_subject) {
		KisImageSP img = m_subject -> currentImg();

		if (img && img -> activeDevice() && e -> button() == LeftButton) {
			clearSelection();
			m_startPos = e -> pos().floorQPoint();
			m_endPos = e -> pos().floorQPoint();
			m_selecting = true;
		}
	}
}

void KisToolSelectRectangular::move(KisMoveEvent *e)
{
	if (m_subject && m_selecting) {

		if (m_startPos != m_endPos)
			paintOutline();

		m_endPos = e -> pos().floorQPoint(); //controller -> windowToView(e -> pos());
		paintOutline();
	}
}

void KisToolSelectRectangular::buttonRelease(KisButtonReleaseEvent *e)
{
	if (m_subject && m_selecting) {
		if (m_startPos == m_endPos) {
			clearSelection();
		} else {
			KisImageSP img = m_subject -> currentImg();

			if (!img)
				return;

			m_endPos = e -> pos().floorQPoint();

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
				KisFloatingSelectionSP selection;

                                QRect rc(m_startPos, m_endPos);

				parent = img -> activeDevice();

				if (parent) {
					rc = rc.normalize();
					selection = new KisFloatingSelection(parent, img, "rectangular selection tool frame", OPACITY_OPAQUE);
					selection -> setBounds(rc);
					img -> setFloatingSelection(selection);

					if (img -> undoAdapter())
						img -> undoAdapter() -> addCommand(new RectSelectCmd(selection));
				}
			}
		}

		m_selecting = false;
	}
}

void KisToolSelectRectangular::paintOutline()
{
	if (m_subject) {
		KisCanvasControllerInterface *controller = m_subject -> canvasController();
		QWidget *canvas = controller -> canvas();
		QPainter gc(canvas);
		QRect rc;

		paintOutline(gc, rc);
	}
}

void KisToolSelectRectangular::paintOutline(QPainter& gc, const QRect&)
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

void KisToolSelectRectangular::setup(KActionCollection *collection)
{
	m_action = static_cast<KRadioAction *>(collection -> action(name()));

	if (m_action == 0) {
		m_action = new KRadioAction(i18n("Tool &Rectangular Select"), 
					    "rectangular", 
					    Qt::Key_R, 
					    this,
					    SLOT(activate()), 
					    collection, 
					    name());
		m_action -> setExclusiveGroup("tools");
		m_ownAction = true;
	}
}

#include "kis_tool_select_rectangular.moc"
