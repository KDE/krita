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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
        : super(i18n ("Polygon")),
          m_dragging (false),
          m_currentImage (0)
{
    setName("tool_polygon");
    // initialize ellipse tool settings
//    m_lineThickness = 4;
//     m_opacity = 255;
//     m_usePattern = false;
//     m_useGradient = false;
//     m_fillSolid = false;
}

KisToolPolygon::~KisToolPolygon()
{
}

void KisToolPolygon::update (KisCanvasSubject *subject)
{
        super::update (subject);
        if (m_subject)
            m_currentImage = m_subject->currentImg ();
}

void KisToolPolygon::buttonPress(KisButtonPressEvent *event)
{
    if (m_currentImage) {
        if (event -> button() == LeftButton) {

            m_dragging = true;

            if (m_points.isEmpty())
            {
                m_dragStart = event -> pos();
                m_dragEnd = event -> pos();
                m_points.append(m_dragStart);
            } else {
                m_dragStart = m_dragEnd;
                m_dragEnd = event -> pos();
                draw();
            }
        } else if (event -> button() == RightButton) {
            // erase old lines on canvas
            draw();
            m_dragging = false;
    
            KisPaintDeviceImplSP device = m_currentImage->activeDevice ();;
            KisPainter painter (device);
            painter.beginTransaction (i18n ("Polygon"));
    
            painter.setPaintColor(m_subject -> fgColor());
            painter.setBackgroundColor(m_subject -> bgColor());
            painter.setFillStyle(fillStyle());
            painter.setBrush(m_subject -> currentBrush());
            painter.setPattern(m_subject -> currentPattern());
            painter.setOpacity(m_opacity);
            painter.setCompositeOp(m_compositeOp);
            KisPaintOp * op = KisPaintOpRegistry::instance()->paintOp(m_subject->currentPaintop(), &painter);
            painter.setPaintOp(op); // Painter takes ownership

            painter.paintPolygon(m_points);

            m_points.clear();
            
            m_currentImage -> notify( painter.dirtyRect() );
            notifyModified();
    
            KisUndoAdapter *adapter = m_currentImage -> undoAdapter();
            if (adapter) {
                adapter -> addCommand(painter.endTransaction());
            }
        }
    }
}

void KisToolPolygon::move(KisMoveEvent *event)
{
    if (m_dragging) {
        // erase old lines on canvas
        draw();
        // get current mouse position
        m_dragEnd = event -> pos();
        // draw new lines on canvas
        draw();
    }
}

void KisToolPolygon::buttonRelease(KisButtonReleaseEvent *event)
{
        if (!m_subject || !m_currentImage)
            return;

        if (m_dragging && event -> button() == LeftButton)  {
                m_dragging = false;
                m_points.append (m_dragEnd);
    }

    if (m_dragging && event -> button() == RightButton) {
        
        }
}

void KisToolPolygon::paint(QPainter& gc)
{
    draw(gc);
}

void KisToolPolygon::paint(QPainter& gc, const QRect&)
{
    draw(gc);
}

void KisToolPolygon::draw()
{
    if (m_subject) {
        KisCanvasControllerInterface *controller = m_subject -> canvasController();
        QWidget *canvas = controller -> canvas();
        QPainter gc(canvas);

        draw(gc);
    }
}

void KisToolPolygon::draw(QPainter& gc)
{
        if (!m_subject || !m_currentImage)
            return;

        QPen pen(Qt::white, 0, Qt::SolidLine); 

    gc.setPen(pen);
        gc.setRasterOp(Qt::XorROP);

    KisCanvasControllerInterface *controller = m_subject -> canvasController();
    KisPoint start, end;
    QPoint startPos;
    QPoint endPos;

    if (m_dragging) {
        startPos = controller -> windowToView(m_dragStart.floorQPoint());
        endPos = controller -> windowToView(m_dragEnd.floorQPoint());
        gc.drawLine(startPos, endPos);
    } else {
        for (KisPointVector::iterator it = m_points.begin(); it != m_points.end(); ++it) {

            if (it == m_points.begin())
            {
                start = (*it);
            } else {
                end = (*it);

                startPos = controller -> windowToView(start.floorQPoint());
                endPos = controller -> windowToView(end.floorQPoint());

                gc.drawLine(startPos, endPos);

                start = end;
            }
        }
    }
}

void KisToolPolygon::setup(KActionCollection *collection)
{
        m_action = static_cast<KRadioAction *>(collection -> action(name()));

    if (m_action == 0) {
        KShortcut shortcut(Qt::Key_Plus);
        shortcut.append(KShortcut(Qt::Key_F8));
        m_action = new KRadioAction(i18n("&Polygon"),
                        "polygon",
                        shortcut,
                        this,
                        SLOT(activate()),
                        collection,
                        name());
        Q_CHECK_PTR(m_action);

        m_action -> setToolTip(i18n("Draw a polygon"));
        m_action -> setExclusiveGroup("tools");
        m_ownAction = true;
        }
}

#include "kis_tool_polygon.moc"
