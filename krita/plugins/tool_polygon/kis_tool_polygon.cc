/*
 *  kis_tool_polygon.cc -- part of Krita
 *
 *  Copyright (c) 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
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


#include <math.h> 
 
#include <qpainter.h>
#include <qspinbox.h>

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <kdebug.h>
#include <knuminput.h>

#include "kis_doc.h"
#include "kis_view.h"
#include "kis_painter.h"
#include "kis_canvas_subject.h"
#include "kis_canvas_controller.h"
#include "kis_button_press_event.h"
#include "kis_button_release_event.h"
#include "kis_move_event.h"
#include "kis_paintop_registry.h"

#include "kis_tool_polygon.h"

KisToolPolygon::KisToolPolygon()
        : super(),
          m_dragging (false),
          m_currentImage (0),
          m_polyLineStarted (false)
{
	setName("tool_polygon");
	// initialize ellipse tool settings
//	m_lineThickness = 4;
// 	m_opacity = 255;
// 	m_usePattern = false;
// 	m_useGradient = false;
// 	m_fillSolid = false;
        m_points = new KisPointVector();
}

KisToolPolygon::~KisToolPolygon()
{
}

void KisToolPolygon::update (KisCanvasSubject *subject)
{
//         kdDebug (40001) << "KisToolStar::update(" << subject << ")" << endl;
        super::update (subject);
        if (m_subject)
            m_currentImage = m_subject->currentImg ();
}

void KisToolPolygon::buttonPress(KisButtonPressEvent *event)
{
//         kdDebug (40001) << "KisToolStar::buttonPress" << event->pos () << endl;
	if (m_currentImage && event -> button() == LeftButton) {
		if ( !(m_polyLineStarted) )
                {
                        m_dragging = true;
                        m_polyLineStarted = true;
                        m_dragStart = event -> pos();
                        m_dragEnd = event -> pos();
                        m_points -> append (m_dragEnd);
                } else {
                        m_dragging = true;
                        m_dragStart = m_dragEnd;
                        m_dragEnd = event -> pos();
                        // erase old lines on canvas
		        draw(m_dragStart, m_dragEnd);
                }
        } else if (m_currentImage && event -> button() == RightButton) {
                // erase old lines on canvas
		draw(m_dragStart, m_dragEnd);
		m_dragging = false;
                m_polyLineStarted = false;

                m_dragEnd = event->pos ();
                if (m_dragStart == m_dragEnd)
                        return;

                if (!m_currentImage)
                        return;

                KisPaintDeviceSP device = m_currentImage->activeDevice ();;
                KisPainter painter (device);
                painter.beginTransaction (i18n ("polygon"));

                painter.setPaintColor(m_subject -> fgColor());
                painter.setBrush(m_subject -> currentBrush());
                //painter.setOpacity(m_opacity);
                //painter.setCompositeOp(m_compositeOp);
		KisPaintOp * op = KisPaintOpRegistry::instance() -> paintOp("paintbrush", &painter);
		painter.setPaintOp(op); // Painter takes ownership

                KisPoint polygonStart,start,end;
                KisPointVector::iterator it;
                for( it = m_points -> begin(); it != m_points -> end(); ++it )
                {
                        if( it == m_points -> begin() )
                        {
                                start = (*it);
                                polygonStart = start; 
                        } else {
                                end = (*it);
                                painter.paintLine(start, PRESSURE_DEFAULT, 0, 0, end, PRESSURE_DEFAULT, 0, 0);
                                start = end;
                        }
                }
                painter.paintLine(polygonStart, PRESSURE_DEFAULT, 0, 0, end, PRESSURE_DEFAULT, 0, 0);
                m_points -> clear();
                
                //painter.paintLine(m_dragStart, PRESSURE_DEFAULT, 0, 0, m_dragEnd, PRESSURE_DEFAULT, 0, 0);
                //painter.paintLine(m_dragStart, PRESSURE_DEFAULT, 0, 0, m_dragEnd, PRESSURE_DEFAULT, 0, 0);
                m_currentImage -> notify( painter.dirtyRect() );
		notifyModified();

                KisUndoAdapter *adapter = m_currentImage -> undoAdapter();
                if (adapter) {
                        adapter -> addCommand(painter.endTransaction());
                }
        }
        
}

void KisToolPolygon::move(KisMoveEvent *event)
{
//         kdDebug (40001) << "KisToolStar::move" << event->pos () << endl;
	if (m_dragging) {
		// erase old lines on canvas
		draw(m_dragStart, m_dragEnd);
		// get current mouse position
		m_dragEnd = event -> pos();
		// draw new lines on canvas
		draw(m_dragStart, m_dragEnd);
	}
}

void KisToolPolygon::buttonRelease(KisButtonReleaseEvent *event)
{
        if (!m_subject || !m_currentImage)
            return;

        if (m_dragging && event -> button() == LeftButton)  {
                m_dragging = false;
                m_points -> append (m_dragEnd);
}

	if (m_dragging && event -> button() == RightButton) {
		
        }
}

void KisToolPolygon::draw(const KisPoint& start, const KisPoint& end )
{
        if (!m_subject || !m_currentImage)
            return;

        KisCanvasControllerInterface *controller = m_subject -> canvasController();
        QWidget *canvas = controller->canvas ();	
        QPainter p (canvas);
        QPen pen(Qt::SolidLine); 

        KisPoint startPos;
        KisPoint endPos;	
        startPos = controller -> windowToView(start);
        endPos = controller -> windowToView(end);

        p.setRasterOp (Qt::NotROP);
        
        p.drawLine(startPos.floorQPoint(), endPos.floorQPoint());
        p.end ();
}

void KisToolPolygon::setup(KActionCollection *collection)
{
        m_action = static_cast<KRadioAction *>(collection -> action(name()));

	if (m_action == 0) {
		KShortcut shortcut(Qt::Key_Plus);
		shortcut.append(KShortcut(Qt::Key_F8));
		m_action = new KRadioAction(i18n("Tool &Polygon"),
					    "polygon",
					    shortcut,
					    this,
					    SLOT(activate()),
					    collection,
					    name());
		m_action -> setExclusiveGroup("tools");
		m_ownAction = true;
        }
}

#include "kis_tool_polygon.moc"
