/*
 *  kis_tool_star.cc -- part of Krita
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

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <kdebug.h>

#include "kis_doc.h"
#include "kis_view.h"
#include "kis_painter.h"
#include "kis_canvas_subject.h"
#include "kis_canvas_controller.h"
#include "kis_tool_star.h"
#include "kis_button_press_event.h"
#include "kis_button_release_event.h"
#include "kis_move_event.h"
#include "kis_paintop_registry.h"

KisToolStar::KisToolStar()
        : super(),
          m_dragging (false),
          m_currentImage (0)
{
	setName("tool_star");
	// initialize ellipse tool settings
//	m_lineThickness = 4;
// 	m_opacity = 255;
// 	m_usePattern = false;
// 	m_useGradient = false;
// 	m_fillSolid = false;
}

KisToolStar::~KisToolStar()
{
}

void KisToolStar::update (KisCanvasSubject *subject)
{
//         kdDebug (40001) << "KisToolStar::update(" << subject << ")" << endl;
        super::update (subject);
        if (m_subject)
            m_currentImage = m_subject->currentImg ();
}

void KisToolStar::buttonPress(KisButtonPressEvent *event)
{
//         kdDebug (40001) << "KisToolStar::buttonPress" << event->pos () << endl;
	if (m_currentImage && event -> button() == LeftButton) {
		m_dragging = true;
		m_dragStart = event -> pos();
		m_dragEnd = event -> pos();
	}
}

void KisToolStar::move(KisMoveEvent *event)
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

void KisToolStar::buttonRelease(KisButtonReleaseEvent *event)
{
        if (!m_subject || !m_currentImage)
            return;

	if (m_dragging && event -> button() == LeftButton) {
		// erase old lines on canvas
		draw(m_dragStart, m_dragEnd);
		m_dragging = false;

                m_dragEnd = event->pos ();
                if (m_dragStart == m_dragEnd)
                        return;

                if (!m_currentImage)
                        return;

                KisPaintDeviceSP device = m_currentImage->activeDevice ();;
                KisPainter painter (device);
                painter.beginTransaction (i18n ("star"));

                painter.setPaintColor(m_subject -> fgColor());
                painter.setBrush(m_subject -> currentBrush());
                //painter.setOpacity(m_opacity);
                //painter.setCompositeOp(m_compositeOp);
		KisPaintOp * op = KisPaintOpRegistry::instance() -> paintOp("paintbrush", &painter);
		painter.setPaintOp(op); // Painter takes ownership

                //painter.paintEllipse(m_dragStart, m_dragEnd, PRESSURE_DEFAULT/*event -> pressure()*/, event -> xTilt(), event -> yTilt());
                QPointArray coord = starCoordinates(m_vertices, m_dragStart.x(), m_dragStart.y(), m_dragEnd.x(), m_dragEnd.y());
                //kdDebug() << "Number of points:" << coord.size() << endl;
                QPoint start,end;
                for(int i=1;i<coord.size();i++)
                {
                        start=coord.point(i-1);
                        end=coord.point(i);
                        painter.paintLine(start, PRESSURE_DEFAULT, 0, 0, end, PRESSURE_DEFAULT, 0, 0);
                }  
                start=coord.point(coord.size()-1);
                end=coord.point(0);
                painter.paintLine(start, PRESSURE_DEFAULT, 0, 0, end, PRESSURE_DEFAULT, 0, 0);
                //painter.paintLine(m_dragStart, PRESSURE_DEFAULT, 0, 0, m_dragEnd, PRESSURE_DEFAULT, 0, 0);
                m_currentImage -> notify( painter.dirtyRect() );
		notifyModified();

                KisUndoAdapter *adapter = m_currentImage -> undoAdapter();
                if (adapter) {
                        adapter -> addCommand(painter.endTransaction());
                }
        }
}

void KisToolStar::draw(const KisPoint& start, const KisPoint& end )
{
        if (!m_subject || !m_currentImage)
            return;

        KisCanvasControllerInterface *controller = m_subject->canvasController ();
//         kdDebug (40001) << "KisToolStar::draw(" << start << "," << end << ")"
//                         << " windowToView: start=" << controller->windowToView (start)
//                         << " windowToView: end=" << controller->windowToView (end)
//                         << endl;
        QWidget *canvas = controller->canvas ();
        QPainter p (canvas);

        p.setRasterOp (Qt::NotROP);
        //p.drawEllipse (QRect (controller->windowToView (start).floorQPoint(), controller->windowToView (end).floorQPoint()));
        p.drawPolygon(starCoordinates(m_vertices, start.x(), start.y(), end.x(), end.y()));
        p.end ();
}

void KisToolStar::setup(KActionCollection *collection)
{
        m_action = static_cast<KRadioAction *>(collection -> action(name()));

	if (m_action == 0) {
		KShortcut shortcut(Qt::Key_Plus);
		shortcut.append(KShortcut(Qt::Key_F8));
		m_action = new KRadioAction(i18n("Tool &Star"),
					    "star",
					    shortcut,
					    this,
					    SLOT(activate()),
					    collection,
					    name());
		m_action -> setExclusiveGroup("tools");
		m_ownAction = true;
	        m_innerOuterRatio=40;
                m_vertices=5;
        }
}

QPointArray KisToolStar::starCoordinates(int N, int mx, int my, int x, int y)
{
        int R=0, r=0,n=0;
        
        QPointArray starCoordinatesArray(2*N);
        
        // the radius of the outer edges
        R=sqrt((x-mx)*(x-mx)+(y-my)*(y-my));
        //kdDebug() << "starCoordinates: Radius R: " << R << endl;
        
        // the radius of the inner edges 
        r=R*m_innerOuterRatio/100.0;
        //kdDebug() << "starCoordinates: Radius R: " << R << endl;
        
        //set outer edges
        for(n=0;n<N;n++){
                starCoordinatesArray.setPoint(2*n,mx+R*cos(n * 2.0 * M_PI / N),my+R*sin(n *2.0 * M_PI / N));  
        }
        
        //set inner edges
        for(n=0;n<N;n++){
                starCoordinatesArray.setPoint(2*n+1,mx+r*cos((n + 0.5) * 2.0 * M_PI / N),my+r*sin((n +0.5) * 2.0 * M_PI / N)); 
        }
        
        for(n=0;n<2*N;n++)
                //kdDebug() << "starCoordinatesArray: (x,y) " << starCoordinatesArray.point(n)  << endl;
        return starCoordinatesArray;
}


#include "kis_tool_star.moc"
