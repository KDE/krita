/*
 *  kis_tool_transform.cc -- part of Krita
 *
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
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
}

void KisToolTransform::activate()
{
	if(m_subject)
	{
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
		m_endPos = QPoint(x+w-1, y+h-1);
		m_org_cenX = (m_startPos.x() + m_endPos.x()) / 2.0;
		m_org_cenY = (m_startPos.y() + m_endPos.y()) / 2.0;

		m_scaleX = 1.0;
		m_scaleY = 1.0;
		m_translateX = m_org_cenX;
		m_translateY = m_org_cenY;
		
		paintOutline();
	}
}

void KisToolTransform::paint(QPainter& gc)
{
	paintOutline(gc, QRect());
}

void KisToolTransform::paint(QPainter& gc, const QRect& rc)
{
	paintOutline(gc, rc);
}


void KisToolTransform::buttonPress(KisButtonPressEvent *e)
{
	if (m_subject) {
		KisImageSP img = m_subject -> currentImg();
		KisCanvasControllerInterface *controller = m_subject -> canvasController();

		if (img && img -> activeDevice() && e -> button() == LeftButton) {
			switch(m_function)
			{
				case ROTATE:
					m_clickoffset = e -> pos().floorQPoint() 
							- controller -> windowToView(QPoint(m_translateX,m_translateY));
					m_clickangle = -m_a - atan2(m_clickoffset.x(),m_clickoffset.y());
					m_clickoffset = QPoint(0, 0);
					break;
				case MOVE:
					m_clickoffset = e -> pos().floorQPoint() 
							- controller -> windowToView(QPoint(m_translateX,m_translateY));
					break;
				case TOPLEFTSCALE:
					m_clickoffset = e -> pos().floorQPoint() 
							- controller -> windowToView(m_topleft);
					break;
				case TOPSCALE:
					m_clickoffset = e -> pos().floorQPoint() 
							- controller -> windowToView((m_topleft + m_topright)/2);
					break;
				case TOPRIGHTSCALE:
					m_clickoffset = e -> pos().floorQPoint() 
							- controller -> windowToView(m_topright);
					break;
				case RIGHTSCALE:
					m_clickoffset = e -> pos().floorQPoint() 
							- controller -> windowToView((m_topright + m_bottomright)/2);
					break;
				case BOTTOMRIGHTSCALE:
					m_clickoffset = e -> pos().floorQPoint() 
							- controller -> windowToView(m_bottomright);
					break;
				case BOTTOMSCALE:
					m_clickoffset = e -> pos().floorQPoint() 
							- controller -> windowToView((m_bottomleft + m_bottomright)/2);
					break;
				case BOTTOMLEFTSCALE:
					m_clickoffset = e -> pos().floorQPoint() 
							- controller -> windowToView(m_bottomleft);
					break;
				case LEFTSCALE:
					m_clickoffset = e -> pos().floorQPoint() 
							- controller -> windowToView((m_topleft + m_bottomleft)/2);
					break;
			}
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

		Q_ASSERT(controller);
		QPoint topleft = controller -> windowToView(m_topleft);
		QPoint topright = controller -> windowToView(m_topright);
		QPoint bottomleft = controller -> windowToView(m_bottomleft);
		QPoint bottomright = controller -> windowToView(m_bottomright);
		
		QPoint mousePos = e -> pos().floorQPoint();
		
		if (m_subject && m_selecting) {
			paintOutline();
			
			mousePos -= m_clickoffset;
			
			// transform mousePos coords, so it seems like it isn't rotated and centered at 0,0
			double newX = invrotX(mousePos.x() - m_translateX, mousePos.y() - m_translateY);
			double newY = invrotY(mousePos.x() - m_translateX, mousePos.y() - m_translateY);
			double dx=0, dy=0;
			
			if(m_function == MOVE)
			{
				m_translateX += newX;
				m_translateY += newY;
			}

			if(m_function == ROTATE)
			{
				m_a = -atan2(mousePos.x() - m_translateX, mousePos.y() - m_translateY)
					- m_clickangle;
			}
			
			if(m_function == TOPSCALE
					|| m_function == TOPLEFTSCALE
					|| m_function == TOPRIGHTSCALE)
			{
				dy = (newY - m_scaleY * (m_startPos.y() - m_org_cenY)) / 2;
				m_scaleY = (newY - dy) / (m_startPos.y() - m_org_cenY);
			}
			
			if(m_function == RIGHTSCALE
					|| m_function == TOPRIGHTSCALE
					|| m_function == BOTTOMRIGHTSCALE)
			{
				dx = (newX - m_scaleX * (m_endPos.x() - m_org_cenX)) / 2;
				m_scaleX = (newX - dx) / (m_endPos.x() - m_org_cenX);
			}
			
			if(m_function == BOTTOMSCALE
					|| m_function == BOTTOMLEFTSCALE
					|| m_function == BOTTOMRIGHTSCALE)
			{
				dy = (newY - m_scaleY * (m_endPos.y() - m_org_cenY)) / 2;
				m_scaleY = (newY - dy) / (m_endPos.y() - m_org_cenY);
			}
			
			if(m_function == LEFTSCALE
					|| m_function == TOPLEFTSCALE
					|| m_function == BOTTOMLEFTSCALE)
			{
				dx = (newX - m_scaleX * (m_startPos.x() - m_org_cenX)) / 2;
				m_scaleX = (newX - dx) / (m_startPos.x() - m_org_cenX);
			}
			m_translateX += rotX(dx, dy);
			m_translateY += rotY(dx, dy);
			
			paintOutline();
		}
		else
		{
			m_function = ROTATE;
			
			if(det(mousePos - topleft, topright - topleft)>0)
				controller -> canvas() -> setCursor(KisCursor::crossCursor());
			else if(det(mousePos - topright, bottomright - topright)>0)
				controller -> canvas() -> setCursor(KisCursor::crossCursor());
			else if(det(mousePos - bottomright, bottomleft - bottomright)>0)
				controller -> canvas() -> setCursor(KisCursor::crossCursor());
			else if(det(mousePos - bottomleft, topleft - bottomleft)>0)
				controller -> canvas() -> setCursor(KisCursor::crossCursor());
			else
			{
				controller -> canvas() -> setCursor(KisCursor::moveCursor());
				m_function = MOVE;
			}
			
			if(distsq(mousePos, m_topleft)<25)
			{
				controller -> canvas() -> setCursor(KisCursor::sizeFDiagCursor());
				m_function = TOPLEFTSCALE;
			}
			if(distsq(mousePos, (m_topleft + m_topright)/2)<25)
			{
				controller -> canvas() -> setCursor(KisCursor::sizeVerCursor());
				m_function = TOPSCALE;
			}
			if(distsq(mousePos, m_topright)<25)
			{
				controller -> canvas() -> setCursor(KisCursor::sizeBDiagCursor());
				m_function = TOPRIGHTSCALE;
			}
			if(distsq(mousePos, (m_topright + m_bottomright)/2)<25)
			{
				controller -> canvas() -> setCursor(KisCursor::sizeHorCursor());
				m_function = RIGHTSCALE;
			}
			if(distsq(mousePos, m_bottomleft)<25)
			{
				controller -> canvas() -> setCursor(KisCursor::sizeBDiagCursor());
				m_function = BOTTOMLEFTSCALE;
			}
			if(distsq(mousePos, (m_bottomleft + m_bottomright)/2)<25)
			{
				controller -> canvas() -> setCursor(KisCursor::sizeVerCursor());
				m_function = BOTTOMSCALE;
			}
			if(distsq(mousePos, m_bottomright)<25)
			{
				controller -> canvas() -> setCursor(KisCursor::sizeFDiagCursor());
				m_function = BOTTOMRIGHTSCALE;
			}
			if(distsq(mousePos, (m_topleft + m_bottomleft)/2)<25)
			{
				controller -> canvas() -> setCursor(KisCursor::sizeHorCursor());
				m_function = LEFTSCALE;
			}
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

void KisToolTransform::recalcOutline()
{
	double x,y;
	
	m_sina = sin(m_a);
	m_cosa = cos(m_a);
	
	x = (m_startPos.x() - m_org_cenX) * m_scaleX;
	y = (m_startPos.y() - m_org_cenY) * m_scaleY;
	m_topleft = QPoint(int(rotX(x,y) + m_translateX), int(rotY(x,y) + m_translateY));
	
	x = (m_endPos.x() - m_org_cenX) * m_scaleX;
	y = (m_startPos.y() - m_org_cenY) * m_scaleY;
	m_topright = QPoint(int(rotX(x,y) + m_translateX), int(rotY(x,y) + m_translateY));
	
	x = (m_startPos.x() - m_org_cenX) * m_scaleX;
	y = (m_endPos.y() - m_org_cenY) * m_scaleY;
	m_bottomleft = QPoint(int(rotX(x,y) + m_translateX), int(rotY(x,y) + m_translateY));
	
	x = (m_endPos.x() - m_org_cenX) * m_scaleX;
	y = (m_endPos.y() - m_org_cenY) * m_scaleY;
	m_bottomright = QPoint(int(rotX(x,y) + m_translateX), int(rotY(x,y) + m_translateY));
}

void KisToolTransform::paintOutline(QPainter& gc, const QRect&)
{
	if (m_subject) {
		KisCanvasControllerInterface *controller = m_subject -> canvasController();
		RasterOp op = gc.rasterOp();
		QPen old = gc.pen();
		QPen pen(Qt::SolidLine);
		pen.setWidth(1);
		Q_ASSERT(controller);

		recalcOutline();		
		QPoint topleft = controller -> windowToView(m_topleft);
		QPoint topright = controller -> windowToView(m_topright);
		QPoint bottomleft = controller -> windowToView(m_bottomleft);
		QPoint bottomright = controller -> windowToView(m_bottomright);

		gc.setRasterOp(Qt::NotROP);
		gc.setPen(pen);
		gc.drawRect(topleft.x()-4, topleft.y()-4, 8, 8);
		gc.drawLine(topleft.x(), topleft.y(), (topleft.x()+topright.x())/2, (topleft.y()+topright.y())/2);
		gc.drawRect((topleft.x()+topright.x())/2-4, (topleft.y()+topright.y())/2-4, 8, 8);
		gc.drawLine((topleft.x()+topright.x())/2, (topleft.y()+topright.y())/2, topright.x(), topright.y());
		gc.drawRect(topright.x()-4, topright.y()-4, 8, 8);
		gc.drawLine(topright.x(), topright.y(), (topright.x()+bottomright.x())/2, (topright.y()+bottomright.y())/2);
		gc.drawRect((topright.x()+bottomright.x())/2-4, (topright.y()+bottomright.y())/2-4, 8, 8);
		gc.drawLine((topright.x()+bottomright.x())/2, (topright.y()+bottomright.y())/2,bottomright.x(), bottomright.y());
		gc.drawRect(bottomright.x()-4, bottomright.y()-4, 8, 8);
		gc.drawLine(bottomright.x(), bottomright.y(), (bottomleft.x()+bottomright.x())/2, (bottomleft.y()+bottomright.y())/2);
		gc.drawRect((bottomleft.x()+bottomright.x())/2-4, (bottomleft.y()+bottomright.y())/2-4, 8, 8);
		gc.drawLine((bottomleft.x()+bottomright.x())/2, (bottomleft.y()+bottomright.y())/2, bottomleft.x(), bottomleft.y());
		gc.drawRect(bottomleft.x()-4, bottomleft.y()-4, 8, 8);
		gc.drawLine(bottomleft.x(), bottomleft.y(), (topleft.x()+bottomleft.x())/2, (topleft.y()+bottomleft.y())/2);
		gc.drawRect((topleft.x()+bottomleft.x())/2-4, (topleft.y()+bottomleft.y())/2-4, 8, 8);
		gc.drawLine((topleft.x()+bottomleft.x())/2, (topleft.y()+bottomleft.y())/2, topleft.x(), topleft.y());
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
