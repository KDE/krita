/*
 *  kis_tool_line.cc - part of Krayon
 *
 *  Copyright (c) 2000 John Califf 
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2003 Boudewijn Rempt <boud@valdyas.org>
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

#include <kdebug.h>
#include <kaction.h>
#include <kcommand.h>
#include <klocale.h>

#include "kis_cursor.h"
#include "kis_dlg_toolopts.h"
#include "kis_doc.h"
#include "kis_painter.h"
#include "kis_tool_line.h"
#include "kis_view.h"

KisToolLine::KisToolLine() 
	: super(),
	  m_dragging( false )
{
	setCursor(KisCursor::arrowCursor());

	m_painter = 0;
	m_currentImage = 0;
	m_startPos = QPoint(0, 0);
	m_endPos = QPoint(0, 0);
}

KisToolLine::~KisToolLine()
{
}

void KisToolLine::update(KisCanvasSubject *subject)
{
	m_subject = subject;
	m_currentImage = subject -> currentImg();

	super::update(m_subject);
}


void KisToolLine::paint(QPainter& gc)
{
	if (m_dragging)
		paintLine(gc, QRect());
}

void KisToolLine::paint(QPainter& gc, const QRect& rc)
{
	if (m_dragging)
		paintLine(gc, rc);
}

void KisToolLine::mousePress(QMouseEvent *e)
{
        if (!m_subject) return;

        if (!m_subject -> currentBrush()) return;

        if (e -> button() == QMouseEvent::LeftButton) {
		m_dragging = true;
		//KisCanvasControllerInterface *controller = m_subject -> canvasController();
		m_startPos = e -> pos(); //controller -> windowToView(e -> pos());
		m_endPos = e -> pos(); //controller -> windowToView(e -> pos());
	}
}

void KisToolLine::mouseMove(QMouseEvent *e)
{
	if (m_dragging) {
		if (m_startPos != m_endPos)
			paintLine();
		//KisCanvasControllerInterface *controller = m_subject -> canvasController();
		m_endPos = e -> pos();//controller -> windowToView(e -> pos());
		paintLine();
	}
}

void KisToolLine::mouseRelease(QMouseEvent *e)
{
	if (m_dragging && e -> button() == QMouseEvent::LeftButton) {
		m_dragging = false;
		KisCanvasControllerInterface *controller = m_subject -> canvasController();
		KisImageSP img = m_subject -> currentImg();

		if (m_startPos == m_endPos) {
			controller -> updateCanvas();
			m_dragging = false;
			return;
		}

		m_endPos = e -> pos();
		
		
		if (m_endPos.y() < 0)
			m_endPos.setY(0);
		
		if (m_endPos.y() > img -> height())
			m_endPos.setY(img -> height());
		
		if (m_endPos.x() < 0)
			m_endPos.setX(0);
		
		if (m_endPos.x() > img -> width())
			m_endPos.setX(img -> width());
		
		KisPaintDeviceSP device;
		if (m_currentImage && 
		    (device = m_currentImage -> activeDevice()) && 
		    m_subject && 
		    m_subject -> currentBrush()) 
		{
			if (m_painter)
				delete m_painter;
			m_painter = new KisPainter( device );
			m_painter -> beginTransaction(i18n("line"));
	
			m_painter -> setPaintColor(m_subject -> fgColor());
			m_painter -> setBrush(m_subject -> currentBrush());
			
			m_painter -> paintLine(PAINTOP_BRUSH, m_startPos, m_endPos, 128, 0, 0, 0);
			m_currentImage -> notify( m_painter -> dirtyRect() );
			
			KisUndoAdapter *adapter = m_currentImage -> undoAdapter();
			if (adapter && m_painter) {
				adapter -> addCommand(m_painter->endTransaction());
			}
			delete m_painter;
			m_painter = 0;
		}
               	else {
			// m_painter can be 0 here...!!!
			controller -> updateCanvas(m_painter -> dirtyRect()); // Removes the last remaining line.
		}
	}
    
}

void KisToolLine::tabletEvent(QTabletEvent */*event*/) 
{
	// Nothing yet -- I'm not sure how to handle this, perhaps
	// have thick-thin lines for pressure.
}

void KisToolLine::paintLine()
{
	if (m_subject) {
		KisCanvasControllerInterface *controller = m_subject -> canvasController();
		QWidget *canvas = controller -> canvas();
		QPainter gc(canvas);
		QRect rc;

		paintLine(gc, rc);
	}
}

void KisToolLine::paintLine(QPainter& gc, const QRect&)
{
	if (m_subject) {
//		KisCanvasControllerInterface *controller = m_subject -> canvasController();
		RasterOp op = gc.rasterOp();
		QPen old = gc.pen();
		QPen pen(Qt::SolidLine);
		QPoint start;
		QPoint end;

//		Q_ASSERT(controller);
		start = m_startPos; //controller -> viewToWindow(m_startPos);
		end = m_endPos; //controller -> viewToWindow(m_endPos);
//  		start.setX(start.x() - controller -> horzValue());
//  		start.setY(start.y() - controller -> vertValue());
//  		end.setX(end.x() - controller -> horzValue());
//  		end.setY(end.y() - controller -> vertValue());
//  		end.setX((end.x() - start.x()));
//  		end.setY((end.y() - start.y()));
 		start *= m_subject -> zoomFactor();
 		end *= m_subject -> zoomFactor();
		gc.setRasterOp(Qt::NotROP);
		gc.setPen(pen);
		gc.drawLine(start.x(), start.y(), end.x(), end.y());
		gc.setRasterOp(op);
		gc.setPen(old);
	}
}

KDialog *KisToolLine::options(QWidget * parent)
{
	ToolOptsStruct ts;    

	ts.usePattern = false; //m_usePattern;
	ts.useGradient = false; //m_useGradient;
	ts.opacity = OPACITY_OPAQUE; //m_opacity;

// 	bool old_usePattern = m_usePattern;
// 	bool old_useGradient = m_useGradient;
// 	unsigned int old_opacity = m_opacity;

	ToolOptionsDialog *d = new ToolOptionsDialog(tt_linetool, ts, parent);

	return d;

}

void KisToolLine::setup(KActionCollection *collection)
{
	KToggleAction *toggle = new KToggleAction(i18n("&Line Tool"), 
						  "line", 0, this, 
						  SLOT(activate()), collection,
						  "tool_line");

	toggle -> setExclusiveGroup("tools");
}
