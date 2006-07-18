/*
 *  kis_tool_perspectivegrid.cc - part of Krita
 *
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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
#include <kis_tool_perspectivegrid.h>

#include <qapplication.h>
#include <qpainter.h>
#include <qregion.h>
#include <qwidget.h>
#include <qlayout.h>

#include <kaction.h>
#include <kdebug.h>
#include <kcommand.h>
#include <klocale.h>

#include <kis_button_press_event.h>
#include <kis_button_release_event.h>
#include <kis_canvas_controller.h>
#include <kis_canvas_painter.h>
#include <kis_canvas_subject.h>
#include <kis_cursor.h>
#include <kis_image.h>
#include <kis_move_event.h>
#include <kis_selected_transaction.h>
#include <kis_painter.h>
#include <kis_paintop_registry.h>
#include <kis_perspective_grid.h>
#include <kis_vec.h>

#include <kis_canvas.h>

KisToolPerspectiveGrid::KisToolPerspectiveGrid()
    : super(i18n("Perspective grid"))
{
    setName("tool_perspective_grid");

    m_subject = 0;
    m_dragging = false;
}

KisToolPerspectiveGrid::~KisToolPerspectiveGrid()
{
}

void KisToolPerspectiveGrid::activate()
{
    if( ! m_subject->currentImg()->perspectiveGrid()->hasSubGrids() )
    {
        m_points.clear();
    }
    super::activate();
}

void KisToolPerspectiveGrid::deactivate()
{
    drawGridCreation();
    m_points.clear();
    m_dragging = false;
}


void KisToolPerspectiveGrid::update (KisCanvasSubject *subject)
{
    m_subject = subject;
    super::update(m_subject);
}

void KisToolPerspectiveGrid::buttonPress(KisButtonPressEvent *event)
{
    if( ! m_subject->currentImg()->perspectiveGrid()->hasSubGrids() )
    {
        if (event->button() == LeftButton) {
            m_dragging = true;
    
            if (m_points.isEmpty())
            {
                m_dragStart = event->pos();
                m_dragEnd = event->pos();
                m_points.append(m_dragStart);
            } else {
                m_dragStart = m_dragEnd;
                m_dragEnd = event->pos();
                drawGridCreation();
            }
        }
    }
}


void KisToolPerspectiveGrid::move(KisMoveEvent *event)
{
    if (m_dragging) {
        // erase old lines on canvas
        drawGridCreation();
        // get current mouse position
        m_dragEnd = event->pos();
        // draw new lines on canvas
        drawGridCreation();
    }
}

void KisToolPerspectiveGrid::buttonRelease(KisButtonReleaseEvent *event)
{
    if (!m_subject)
        return;

    if( ! m_subject->currentImg()->perspectiveGrid()->hasSubGrids() )
    {
        if (m_dragging && event->button() == LeftButton)  {
            m_dragging = false;
            m_points.append (m_dragEnd);
            if( m_points.size() == 4)
            { // wow we have a grid, isn't that cool ?
                drawGridCreation(); // Clean
                m_subject->currentImg()->perspectiveGrid()->addNewSubGrid( new KisSubPerspectiveGrid( new KisPerspectiveGridNode(m_points[0]), new KisPerspectiveGridNode(m_points[1]), new KisPerspectiveGridNode(m_points[2]), new KisPerspectiveGridNode(m_points[3]) ) );
            }
        }
    }

/*    if (m_dragging && event->button() == RightButton) {

        }*/
}

void KisToolPerspectiveGrid::paint(KisCanvasPainter& gc)
{
    if( ! m_subject->currentImg()->perspectiveGrid()->hasSubGrids() )
    {
        drawGridCreation(gc);
    }
}

void KisToolPerspectiveGrid::paint(KisCanvasPainter& gc, const QRect&)
{
    if( ! m_subject->currentImg()->perspectiveGrid()->hasSubGrids() )
    {
        drawGridCreation(gc);
    }
}

void KisToolPerspectiveGrid::drawGridCreation()
{
    if (m_subject) {
        KisCanvasController *controller = m_subject->canvasController();
        KisCanvas *canvas = controller->kiscanvas();
        KisCanvasPainter gc(canvas);

        drawGridCreation(gc);
    }
}

void KisToolPerspectiveGrid::drawGridCreation(KisCanvasPainter& gc)
{
    if (!m_subject)
        return;

    QPen pen(Qt::white);

    gc.setPen(pen);
        gc.setRasterOp(Qt::XorROP);

    KisCanvasController *controller = m_subject->canvasController();
    KisPoint start, end;
    QPoint startPos;
    QPoint endPos;

    if (m_dragging) {
        startPos = controller->windowToView(m_dragStart.floorQPoint());
        endPos = controller->windowToView(m_dragEnd.floorQPoint());
        gc.drawLine(startPos, endPos);
    } else {
        for (KisPointVector::iterator it = m_points.begin(); it != m_points.end(); ++it) {

            if (it == m_points.begin())
            {
                start = (*it);
            } else {
                end = (*it);

                startPos = controller->windowToView(start.floorQPoint());
                endPos = controller->windowToView(end.floorQPoint());

                gc.drawLine(startPos, endPos);

                start = end;
            }
        }
    }
}


void KisToolPerspectiveGrid::setup(KActionCollection *collection)
{
    m_action = static_cast<KRadioAction *>(collection->action(name()));

    if (m_action == 0) {
        m_action = new KRadioAction(i18n("&Perspective Grid"),
                                    "KisToolPerspectiveGrid" ,
                        0,
                        this,
                        SLOT(activate()),
                        collection,
                        name());
        Q_CHECK_PTR(m_action);
        m_action->setExclusiveGroup("tools");
        m_action->setToolTip(i18n("Edit the perspective grid"));
        m_ownAction = true;
    }
}


// QWidget* KisToolPerspectiveGrid::createOptionWidget(QWidget* parent)
// {
//     return 0;
// }
// 
// QWidget* KisToolPerspectiveGrid::optionWidget()
// {
//         return 0;
// }


#include "kis_tool_perspectivegrid.moc"
