/*
 *  kis_tool_transform.cc -- part of Krita
 *
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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
#include <qpushbutton.h>
#include <qobject.h>
#include <qcombobox.h>

#include <kdebug.h>
#include <kaction.h>
#include <kcommand.h>
#include <klocale.h>
#include <knuminput.h>

#include <kis_global.h>
#include <kis_painter.h>
#include <kis_canvas_controller.h>
#include <kis_canvas_subject.h>
#include <kis_cursor.h>
#include <kis_image.h>
#include <kis_undo_adapter.h>
#include <kis_button_press_event.h>
#include <kis_button_release_event.h>
#include <kis_move_event.h>
#include <kis_selection.h>

#include "kis_tool_transform.h"

namespace {
	class TransformCmd : public KNamedCommand {
		typedef KNamedCommand super;

	public:
		TransformCmd();
		virtual ~TransformCmd();

	public:
		virtual void execute();
		virtual void unexecute();

	private:
	};

	TransformCmd::TransformCmd() : super(i18n("Transform"))
	{
	}

	TransformCmd::~TransformCmd()
	{
	}

	void TransformCmd::execute()
	{
	}

	void TransformCmd::unexecute()
	{
	}
}

KisToolTransform::KisToolTransform()
{
	setName("tool_transform");
	setCursor(KisCursor::selectCursor());
	m_subject = 0;
	m_selecting = false;
	m_startPos = QPoint(0, 0);
	m_endPos = QPoint(0, 0);
}

KisToolTransform::~KisToolTransform()
{
}

void KisToolTransform::update(KisCanvasSubject *subject)
{
	m_subject = subject;
		
	super::update(m_subject);
}

void KisToolTransform::clear()
{
	paintOutline();
printf("clean in transform %d\n",this);
}

void KisToolTransform::activate()
{
printf("intro in transform %d\n",this);
	if(m_subject)
	{
printf("intro in transform has subject\n");
		KisToolControllerInterface *controller = m_subject -> toolController();

		if (controller)
			controller -> setCurrentTool(this);
			
		Q_INT32 x,y,w,h;
		KisImageSP img = m_subject -> currentImg();
		KisLayerSP layer = img -> activeLayer();
		if(layer->hasSelection())
		{
			KisSelectionSP sel = layer->selection();
			sel->exactBounds(x,y,w,h);
		}
		else
			layer->exactBounds(x,y,w,h);
		
		m_startPos = QPoint(x, y);
		m_endPos = QPoint(x+w, y+h);
		
		paintOutline();
	}
printf("intro in transform end\n");
}

void KisToolTransform::paint(QPainter& gc)
{
	paintOutline(gc, QRect());
}

void KisToolTransform::paint(QPainter& gc, const QRect& rc)
{
	paintOutline(gc, rc);
}

void KisToolTransform::clearRect()
{
	if (m_subject) {
		KisCanvasControllerInterface *controller = m_subject -> canvasController();
		KisImageSP img = m_subject -> currentImg();

		Q_ASSERT(controller);

		controller -> canvas() -> update();
		
		m_startPos = QPoint(0, 0);
		m_endPos = QPoint(0, 0);

		m_selecting = false;
	}
}

void KisToolTransform::buttonPress(KisButtonPressEvent *e)
{
	if (m_subject) {
		KisImageSP img = m_subject -> currentImg();

		if (img && img -> activeDevice() && e -> button() == LeftButton) {
			m_selecting = true;
		}
	}
}

int det(QPoint v,QPoint w)
{
	return v.x()*w.y()-v.y()*w.x();
}
int distsq(QPoint v,QPoint w)
{
	v -= w;
	return v.x()*v.x() + v.y()*v.y();
}

void KisToolTransform::move(KisMoveEvent *e)
{
	if (m_subject) {
		KisCanvasControllerInterface *controller = m_subject -> canvasController();
		QPoint start, end;

		Q_ASSERT(controller);
		start = controller -> windowToView(m_startPos);
		end = controller -> windowToView(m_endPos);
		QPoint topleft(start.x(),start.y());
		QPoint topright(end.x(),start.y());
		
		if(det(e -> pos().floorQPoint() - topleft, topright - topleft)>0)
			controller -> canvas() -> setCursor(KisCursor::crossCursor());
		else
			controller -> canvas() -> setCursor(KisCursor::moveCursor());
		
		if(distsq(e -> pos().floorQPoint(), topleft)<25)
			controller -> canvas() -> setCursor(KisCursor::sizeFDiagCursor());
		if(distsq(e -> pos().floorQPoint(), topright)<25)
			controller -> canvas() -> setCursor(KisCursor::sizeBDiagCursor());
		
		if (m_subject && m_selecting) {

			paintOutline();
			paintOutline();
		}
	}
}

void KisToolTransform::buttonRelease(KisButtonReleaseEvent *e)
{
	KisImageSP img = m_subject -> currentImg();

	if (!img)
		return;

	if (m_subject && m_selecting) {
		m_selecting = false;
	}
	transform();
}

void KisToolTransform::paintOutline()
{
	if (m_subject) {
		KisCanvasControllerInterface *controller = m_subject -> canvasController();
		QWidget *canvas = controller -> canvas();
		QPainter gc(canvas);
		QRect rc;

		paintOutline(gc, rc);
	}
}

void KisToolTransform::paintOutline(QPainter& gc, const QRect&)
{
	if (m_subject) {
		KisCanvasControllerInterface *controller = m_subject -> canvasController();
		RasterOp op = gc.rasterOp();
		QPen old = gc.pen();
		QPen pen(Qt::SolidLine);
		pen.setWidth(1);
		QPoint start;
		QPoint end;

		Q_ASSERT(controller);
		start = controller -> windowToView(m_startPos);
		end = controller -> windowToView(m_endPos);
		QPoint topleft(start.x(),start.y());
		QPoint topright(end.x(),start.y());

		gc.setRasterOp(Qt::NotROP);
		gc.setPen(pen);
		gc.drawRect(topleft.x()-4, topleft.y()-4, 8, 8);
		gc.drawLine(topleft.x(), topleft.y(), (topleft.x()+topright.x())/2, (topleft.y()+topright.y())/2);
		gc.drawRect((topleft.x()+topright.x())/2-4, (topleft.y()+topright.y())/2-4, 8, 8);
		gc.drawLine((topleft.x()+topright.x())/2, (topleft.y()+topright.y())/2, topright.x(), topright.y());
		gc.drawRect(topright.x()-4, topright.y()-4, 8, 8);
		gc.drawRect(end.x()-4, (start.y()+end.y())/2-4, 8, 8);
		gc.drawRect(end.x()-4, end.y()-4,8, 8);
		gc.drawRect((start.x()+end.x())/2-4, end.y()-4, 8, 8);
		gc.drawRect(start.x()-4, end.y()-4, 8, 8);
		gc.drawRect(start.x()-4, (start.y()+end.y())/2-4, 8, 8);
		gc.setRasterOp(op);
		gc.setPen(old);
	}
}

void KisToolTransform::transform() {
	KisImageSP img = m_subject -> currentImg();

	if (!img)
		return;

	KisProgressDisplayInterface *progress = 0;//view() -> progressDisplay();
	img->activeLayer()->transform(1, 1, 0, 0, 1, -256, 0, progress);
	
	QRect rc = img->activeLayer()->extent();
	rc = rc.normalize();
	
	img -> notify(rc);

	if (img -> undoAdapter())
		img -> undoAdapter() -> addCommand(new TransformCmd());
}

QWidget* KisToolTransform::createOptionWidget(QWidget* parent)
{
	m_optWidget = NULL;
	return m_optWidget;
}

QWidget* KisToolTransform::optionWidget()
{
	return m_optWidget;
}

void KisToolTransform::setup(KActionCollection *collection)
{
	m_action = static_cast<KRadioAction *>(collection -> action(name()));

	if (m_action == 0) {
		m_action = new KRadioAction(i18n("Tool &Transform"), 
					    "transform", 
					    0, 
					    this,
					    SLOT(activate()), 
					    collection, 
					    name());
		m_action -> setExclusiveGroup("tools");
		m_ownAction = true;
	}
}

#include "kis_tool_transform.moc"
