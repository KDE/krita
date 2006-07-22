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
#include <kis_perspective_grid_manager.h>
#include <kis_selected_transaction.h>
#include <kis_painter.h>
#include <kis_paintop_registry.h>
#include <kis_vec.h>

#include <kis_canvas.h>

KisToolPerspectiveGrid::KisToolPerspectiveGrid()
    : super(i18n("Perspective grid")), m_handleSize(13), m_handleHalfSize(6)

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
    m_subject->perspectiveGridManager()->startEdition();
    if( ! m_subject->currentImg()->perspectiveGrid()->hasSubGrids() )
    {
        m_mode = MODE_CREATION;
        m_points.clear();
    } else {
        m_mode = MODE_EDITING;
        drawGrid();
    }
    super::activate();
}

void KisToolPerspectiveGrid::deactivate()
{
    m_subject->perspectiveGridManager()->stopEdition();
    if( m_mode == MODE_CREATION )
    {
        drawGridCreation();
        m_points.clear();
        m_dragging = false;
    } else {
        drawGrid();
    }
}


void KisToolPerspectiveGrid::update (KisCanvasSubject *subject)
{
    m_subject = subject;
    super::update(m_subject);
}

bool KisToolPerspectiveGrid::mouseNear(const QPoint& mousep, const QPoint point)
{
    return (QRect( (point.x() - m_handleHalfSize), (point.y() - m_handleHalfSize), m_handleSize, m_handleSize).contains(mousep) );
}

void KisToolPerspectiveGrid::buttonPress(KisButtonPressEvent *event)
{
    if( m_mode == MODE_CREATION && event->button() == LeftButton)
    {
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
    } else if(m_mode == MODE_EDITING && event->button() == LeftButton){
        // Look for the handle which was pressed
        if (!m_subject)
            return;
        KisPerspectiveGrid* pGrid = m_subject->currentImg()->perspectiveGrid();
        KisCanvasController *controller = m_subject->canvasController();
        Q_ASSERT(controller);
        QPoint mousep = controller->windowToView( event->pos().roundQPoint() );

        for( QValueList<KisSubPerspectiveGrid*>::const_iterator it = pGrid->begin(); it != pGrid->end(); ++it)
        {
            KisSubPerspectiveGrid* grid = *it;
            if( mouseNear( mousep, controller->windowToView(grid->topLeft()->roundQPoint() ) ) )
            {
                kdDebug() << " PRESS TOPLEFT HANDLE " << endl;
                m_mode = MODE_DRAGING_NODE;
                m_selectedNode1 = grid->topLeft();
                break;
            }
            else if( mouseNear( mousep, controller->windowToView(grid->topRight()->roundQPoint() ) ) )
            {
                kdDebug() << " PRESS TOPRIGHT HANDLE " << endl;
                m_mode = MODE_DRAGING_NODE;
                m_selectedNode1 = grid->topRight();
                break;
            }
            else if( mouseNear( mousep, controller->windowToView(grid->bottomLeft()->roundQPoint() ) ) )
            {
                kdDebug() << " PRESS BOTTOMLEFT HANDLE " << endl;
                m_mode = MODE_DRAGING_NODE;
                m_selectedNode1 = grid->bottomLeft();
                break;
            }
            else if( mouseNear( mousep, controller->windowToView(grid->bottomRight()->roundQPoint() ) ) )
            {
                kdDebug() << " PRESS BOTTOMRIGHT HANDLE " << endl;
                m_mode = MODE_DRAGING_NODE;
                m_selectedNode1 = grid->bottomRight();
                break;
            }
            else if( !grid->leftGrid() && mouseNear( mousep, controller->windowToView( ((*grid->topLeft() + *grid->bottomLeft() )*0.5) ).roundQPoint() ) )
            {
                kdDebug() << " PRESS LEFT HANDLE " << endl;
                m_mode = MODE_DRAGING_TRANSLATING_TWONODES;
                drawGrid();
                m_selectedNode1 = new KisPerspectiveGridNode( *grid->topLeft() );
                m_selectedNode2 = new KisPerspectiveGridNode( *grid->bottomLeft() );
                KisSubPerspectiveGrid* newsubgrid = new KisSubPerspectiveGrid( m_selectedNode1, grid->topLeft() , grid->bottomLeft(), m_selectedNode2);
                m_dragEnd = event->pos();
                newsubgrid->setRightGrid( grid);
                grid->setLeftGrid( newsubgrid);
                pGrid->addNewSubGrid( newsubgrid);
                drawGrid();
                break;
            }
            else if( !grid->rightGrid() && mouseNear( mousep, controller->windowToView( ((*grid->topRight() + *grid->bottomRight() )*0.5) ).roundQPoint() ) )
            {
                kdDebug() << " PRESS RIGHT HANDLE " << endl;
                m_mode = MODE_DRAGING_TRANSLATING_TWONODES;
                drawGrid();
                m_selectedNode1 = new KisPerspectiveGridNode( *grid->topRight() );
                m_selectedNode2 = new KisPerspectiveGridNode( *grid->bottomRight() );
                KisSubPerspectiveGrid* newsubgrid = new KisSubPerspectiveGrid( grid->topRight(), m_selectedNode1, m_selectedNode2, grid->bottomRight());
                m_dragEnd = event->pos();
                newsubgrid->setLeftGrid( grid);
                grid->setRightGrid( newsubgrid);
                pGrid->addNewSubGrid( newsubgrid);
                drawGrid();
                break;
            }
            else if( !grid->topGrid() && mouseNear( mousep, controller->windowToView( ((*grid->topLeft() + *grid->topRight() )*0.5) ).roundQPoint() ) )
            {
                kdDebug() << " PRESS TOP HANDLE " << endl;
                m_mode = MODE_DRAGING_TRANSLATING_TWONODES;
                drawGrid();
                m_selectedNode1 = new KisPerspectiveGridNode( *grid->topLeft() );
                m_selectedNode2 = new KisPerspectiveGridNode( *grid->topRight() );
                KisSubPerspectiveGrid* newsubgrid = new KisSubPerspectiveGrid( m_selectedNode1, m_selectedNode2,  grid->topRight(), grid->topLeft() );
                m_dragEnd = event->pos();
                newsubgrid->setBottomGrid( grid);
                grid->setTopGrid( newsubgrid);
                pGrid->addNewSubGrid( newsubgrid);
                drawGrid();
                break;
            }
            else if( !grid->bottomGrid() && mouseNear( mousep, controller->windowToView( ((*grid->bottomLeft() + *grid->bottomRight() )*0.5) ).roundQPoint() ) )
            {
                kdDebug() << " PRESS BOTTOM HANDLE " << endl;
                m_mode = MODE_DRAGING_TRANSLATING_TWONODES;
                drawGrid();
                m_selectedNode1 = new KisPerspectiveGridNode( *grid->bottomLeft() );
                m_selectedNode2 = new KisPerspectiveGridNode( *grid->bottomRight() );
                KisSubPerspectiveGrid* newsubgrid = new KisSubPerspectiveGrid( grid->bottomLeft(), grid->bottomRight(), m_selectedNode2, m_selectedNode1);
                m_dragEnd = event->pos();
                newsubgrid->setTopGrid( grid);
                grid->setBottomGrid( newsubgrid);
                pGrid->addNewSubGrid( newsubgrid);
                drawGrid();
                break;
            }
        }
    }
}


void KisToolPerspectiveGrid::move(KisMoveEvent *event)
{
    if( m_mode == MODE_CREATION )
    {
        if (m_dragging) {
            // erase old lines on canvas
            drawGridCreation();
            // get current mouse position
            m_dragEnd = event->pos();
            // draw new lines on canvas
            drawGridCreation();
        }
    } else {
        if( m_mode == MODE_DRAGING_NODE)
        {
            drawGrid();
            m_selectedNode1->setX( event->pos().x() );
            m_selectedNode1->setY( event->pos().y() );
            drawGrid();
        }
        if( m_mode == MODE_DRAGING_TRANSLATING_TWONODES)
        {
            drawGrid();
            KisPoint translate = event->pos() - m_dragEnd;
            m_dragEnd = event->pos();
            *m_selectedNode1 += translate;;
            *m_selectedNode2 += translate;;
            drawGrid();
        }
    }
}

void KisToolPerspectiveGrid::buttonRelease(KisButtonReleaseEvent *event)
{
    if (!m_subject)
        return;

    if( m_mode == MODE_CREATION  )
    {
        if (m_dragging && event->button() == LeftButton)  {
            m_dragging = false;
            m_points.append (m_dragEnd);
            if( m_points.size() == 4)
            { // wow we have a grid, isn't that cool ?
                drawGridCreation(); // Clean
                m_subject->currentImg()->perspectiveGrid()->addNewSubGrid( new KisSubPerspectiveGrid( new KisPerspectiveGridNode(m_points[0]), new KisPerspectiveGridNode(m_points[1]), new KisPerspectiveGridNode(m_points[2]), new KisPerspectiveGridNode(m_points[3]) ) );
                drawGrid();
                m_mode = MODE_EDITING;
            }
        }
    } else {
        m_mode = MODE_EDITING;
        m_selectedNode1 = 0;
        m_selectedNode2 = 0;
    }

/*    if (m_dragging && event->button() == RightButton) {

        }*/
}

void KisToolPerspectiveGrid::paint(KisCanvasPainter& gc)
{
    if( m_mode == MODE_CREATION )
    {
        drawGridCreation(gc);
    } else {
        drawGrid(gc);
    }
}

void KisToolPerspectiveGrid::paint(KisCanvasPainter& gc, const QRect&)
{
    if( m_mode == MODE_CREATION )
    {
        drawGridCreation(gc);
    } else {
        drawGrid(gc);
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

void KisToolPerspectiveGrid::drawSmallRectangle(KisCanvasPainter& gc, QPoint p)
{
    gc.drawRect( p.x() - m_handleHalfSize - 1, p.y() - m_handleHalfSize - 1, m_handleSize, m_handleSize);
}

void KisToolPerspectiveGrid::drawGrid(KisCanvasPainter& gc)
{
    
    if (!m_subject)
        return;
    
    KisCanvasController *controller = m_subject->canvasController();

    QPen pen(Qt::white);
    QPoint startPos;
    QPoint endPos;

    gc.setPen(pen);
    gc.setRasterOp(Qt::XorROP);
    KisPerspectiveGrid* pGrid = m_subject->currentImg()->perspectiveGrid();

    for( QValueList<KisSubPerspectiveGrid*>::const_iterator it = pGrid->begin(); it != pGrid->end(); ++it)
    {
        KisSubPerspectiveGrid* grid = *it;
        int index = grid->index();
        bool drawLeft = !(grid->leftGrid() && (index > grid->leftGrid()->index() ) );
        bool drawRight = !(grid->rightGrid() && (index > grid->rightGrid()->index() ) );
        bool drawTop = !(grid->topGrid() && (index > grid->topGrid()->index() ) );
        bool drawBottom = !(grid->bottomGrid() && (index > grid->bottomGrid()->index() ) );
        if(drawTop) {
            startPos = controller->windowToView(grid->topLeft()->roundQPoint());
            endPos = controller->windowToView(grid->topRight()->roundQPoint());
            gc.drawLine( startPos, endPos );
            if( !grid->topGrid() )
            {
                drawSmallRectangle(gc, (endPos + startPos) / 2);
            }
            if(drawLeft) {
                drawSmallRectangle(gc, startPos);
            }
            if(drawRight) {
                drawSmallRectangle(gc, endPos);
            }
        }
        if(drawRight) {
            startPos = controller->windowToView(grid->topRight()->roundQPoint());
            endPos = controller->windowToView(grid->bottomRight()->roundQPoint());
            gc.drawLine( startPos, endPos );
            if( !grid->rightGrid() )
            {
                drawSmallRectangle(gc, (endPos + startPos) / 2);
            }
        }
        if(drawBottom) {
            startPos = controller->windowToView(grid->bottomRight()->roundQPoint());
            endPos = controller->windowToView(grid->bottomLeft()->roundQPoint());
            gc.drawLine( startPos, endPos );
            if( !grid->bottomGrid() )
            {
                drawSmallRectangle(gc, (endPos + startPos) / 2);
            }
            if(drawLeft) {
                drawSmallRectangle(gc, endPos);
            }
            if(drawRight) {
                drawSmallRectangle(gc, startPos);
            }
        }
        if(drawLeft) {
            startPos = controller->windowToView(grid->bottomLeft()->roundQPoint());
            endPos = controller->windowToView(grid->topLeft()->roundQPoint());
            gc.drawLine( startPos, endPos );
            if( !grid->leftGrid() )
            {
                drawSmallRectangle(gc, (endPos + startPos) / 2);
            }
        }
    }
}

void KisToolPerspectiveGrid::drawGrid()
{
    if (m_subject) {
        KisCanvasController *controller = m_subject->canvasController();
        KisCanvas *canvas = controller->kiscanvas();
        KisCanvasPainter gc(canvas);

        drawGrid(gc);
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
