/*
 *  kis_tool_perspectivegrid.cc - part of Krita
 *
 *  Copyright (c) 2006,2008 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_tool_perspectivegrid.h"

#include <qapplication.h>
#include <qpainter.h>
#include <qregion.h>
#include <qwidget.h>
#include <qlayout.h>

#include <kaction.h>
#include <kactioncollection.h>
#include <kis_debug.h>
#include <klocale.h>

#include <KoCanvasController.h>

#include <kis_config.h>
#include <kis_cursor.h>
#include <kis_image.h>
#include <KoPointerEvent.h>
#include <KoViewConverter.h>
#include <canvas/kis_perspective_grid_manager.h>
#include <kis_canvas_resource_provider.h>
#include <kis_selected_transaction.h>
#include <kis_painter.h>
#include <kis_paintop_registry.h>
#include <kis_view2.h>

#include <canvas/kis_canvas2.h>

KisToolPerspectiveGrid::KisToolPerspectiveGrid(KoCanvasBase * canvas)
        : KisTool(canvas, KisCursor::load("tool_perspectivegrid_cursor.png", 6, 6)),
          m_handleSize(13), m_handleHalfSize(6), m_canvas(dynamic_cast<KisCanvas2*>(canvas))
{
    Q_ASSERT(m_canvas);
    setObjectName("tool_perspectivegrid");

    m_drawing = false;
}

KisToolPerspectiveGrid::~KisToolPerspectiveGrid()
{
}

void KisToolPerspectiveGrid::activate(bool)
{
    KisTool::activate();

    m_canvas->view()->perspectiveGridManager()->startEdition();
    if (! m_canvas->view()->resourceProvider()->currentImage()->perspectiveGrid()->hasSubGrids()) {
        m_mode = MODE_CREATION;
        m_points.clear();
    } else {
        m_mode = MODE_EDITING;
        useCursor(KisCursor::arrowCursor());
        m_canvas->view()->perspectiveGridManager()->setVisible(true);
        m_canvas->updateCanvas(); // TODO only the correct rect
    }
}

void KisToolPerspectiveGrid::deactivate()
{
    m_canvas->view()->perspectiveGridManager()->stopEdition();
    if (m_mode == MODE_CREATION) {
        m_points.clear();
        m_drawing = false;
    }
    m_canvas->updateCanvas();
}

bool KisToolPerspectiveGrid::mouseNear(const QPointF& mousep, const QPointF& point)
{
    QRectF handlerect((point.x() - m_handleHalfSize), (point.y() - m_handleHalfSize), m_handleSize, m_handleSize);
    return handlerect.contains(mousep);
}

KisPerspectiveGridNodeSP KisToolPerspectiveGrid::nodeNearPoint(KisSubPerspectiveGrid* grid, QPointF point)
{
    if (mouseNear(point, pixelToView(*grid->topLeft()))) {
        dbgPlugins << " NEAR TOPLEFT HANDLE";
        return grid->topLeft();
    } else if (mouseNear(point, pixelToView(*grid->topRight()))) {
        dbgPlugins << " NEAR TOPRIGHT HANDLE";
        return grid->topRight();
    } else if (mouseNear(point, pixelToView(*grid->bottomLeft()))) {
        dbgPlugins << " NEAR BOTTOMLEFT HANDLE";
        return grid->bottomLeft();
    } else if (mouseNear(point, pixelToView(*grid->bottomRight()))) {
        dbgPlugins << " NEAR BOTTOMRIGHT HANDLE";
        return grid->bottomRight();
    }
    return 0;
}


void KisToolPerspectiveGrid::mousePressEvent(KoPointerEvent *event)
{
    KisPerspectiveGrid* pGrid = m_canvas->view()->resourceProvider()->currentImage()->perspectiveGrid();
    if (!pGrid->hasSubGrids() && m_mode != MODE_CREATION) { // it's possible that the perspectiv grid was cleared
        m_mode = MODE_CREATION;
        m_points.clear();
    }
    if (m_mode == MODE_CREATION && event->button() == Qt::LeftButton) {
        m_drawing = true;
        m_currentPt = event->point;

        if (m_points.isEmpty()) {
            m_points.append(m_currentPt);
            m_isFirstPoint = true;
        } else {
            m_isFirstPoint = false;
        }
        m_canvas->updateCanvas(); // TODO update only the relevant part of the canvas
    } else if (m_mode == MODE_EDITING && event->button() == Qt::LeftButton) {
        // Look for the handle which was pressed
        QPointF mousep = m_canvas->viewConverter()->documentToView(event->point);
        for (QList<KisSubPerspectiveGrid*>::const_iterator it = pGrid->begin(); it != pGrid->end(); ++it) {
            KisSubPerspectiveGrid* grid = *it;
            QPointF gridCenter = grid->center();
            dbgKrita << "click at " << event->point << " top left at " << *grid->topLeft();
            if (m_selectedNode1 = nodeNearPoint(grid, mousep)) {
                m_mode = MODE_DRAGING_NODE;
                break;
            } else if (mouseNear(mousep, ((pixelToView(*grid->topLeft()) + pixelToView(*grid->bottomLeft()))*0.5))) {
                dbgPlugins << " PRESS LEFT HANDLE";
                m_mode = MODE_DRAGING_TRANSLATING_TWONODES;
                m_selectedNode1 = new KisPerspectiveGridNode(*grid->topLeft());
                m_selectedNode2 = new KisPerspectiveGridNode(*grid->bottomLeft());
                KisSubPerspectiveGrid* newsubgrid = new KisSubPerspectiveGrid(m_selectedNode1, grid->topLeft() , grid->bottomLeft(), m_selectedNode2);
                m_dragEnd = convertToPixelCoord(event->point);
                pGrid->addNewSubGrid(newsubgrid);
                m_canvas->updateCanvas(); // TODO update only the relevant part of the canvas
                break;
            } else if (mouseNear(mousep, ((pixelToView(*grid->topRight()) + pixelToView(*grid->bottomRight()))*0.5))) {
                dbgPlugins << " PRESS RIGHT HANDLE";
                m_mode = MODE_DRAGING_TRANSLATING_TWONODES;
                m_selectedNode1 = new KisPerspectiveGridNode(*grid->topRight());
                m_selectedNode2 = new KisPerspectiveGridNode(*grid->bottomRight());
                KisSubPerspectiveGrid* newsubgrid = new KisSubPerspectiveGrid(grid->topRight(), m_selectedNode1, m_selectedNode2, grid->bottomRight());
                m_dragEnd = convertToPixelCoord(event->point);
                pGrid->addNewSubGrid(newsubgrid);
                m_canvas->updateCanvas(); // TODO update only the relevant part of the canvas
                break;
            } else if (mouseNear(mousep, ((pixelToView(*grid->topLeft()) + pixelToView(*grid->topRight()))*0.5))) {
                dbgPlugins << " PRESS TOP HANDLE";
                m_mode = MODE_DRAGING_TRANSLATING_TWONODES;
                m_selectedNode1 = new KisPerspectiveGridNode(*grid->topLeft());
                m_selectedNode2 = new KisPerspectiveGridNode(*grid->topRight());
                KisSubPerspectiveGrid* newsubgrid = new KisSubPerspectiveGrid(m_selectedNode1, m_selectedNode2,  grid->topRight(), grid->topLeft());
                m_dragEnd = convertToPixelCoord(event->point);
                pGrid->addNewSubGrid(newsubgrid);
                m_canvas->updateCanvas(); // TODO update only the relevant part of the canvas
                break;
            } else if (mouseNear(mousep, ((pixelToView(*grid->bottomLeft()) + pixelToView(*grid->bottomRight()))*0.5))) {
                dbgPlugins << " PRESS BOTTOM HANDLE";
                m_mode = MODE_DRAGING_TRANSLATING_TWONODES;
                m_selectedNode1 = new KisPerspectiveGridNode(*grid->bottomLeft());
                m_selectedNode2 = new KisPerspectiveGridNode(*grid->bottomRight());
                KisSubPerspectiveGrid* newsubgrid = new KisSubPerspectiveGrid(grid->bottomLeft(), grid->bottomRight(), m_selectedNode2, m_selectedNode1);
                m_dragEnd = convertToPixelCoord(event->point);
                pGrid->addNewSubGrid(newsubgrid);
                m_canvas->updateCanvas(); // TODO update only the relevant part of the canvas
                break;
            } else if (pixelToView(QRectF((gridCenter.x() - 16), (gridCenter.y() - 16), 32, 32)).contains(mousep)) {
                dbgPlugins << " PRESS DELETE ICON";
                pGrid->deleteSubGrid(grid);
                m_canvas->updateCanvas(); // TODO update only the relevant part of the canvas
                if (!pGrid->hasSubGrids()) {
                    m_mode = MODE_CREATION;
                    useCursor(KisCursor::load("tool_perspectivegrid_cursor.png", 6, 6));
                    m_points.clear();
                }
                break;
            }
        }
    }
}


void KisToolPerspectiveGrid::mouseMoveEvent(KoPointerEvent *event)
{
    if (m_mode == MODE_CREATION) {
        if (!m_points.isEmpty()) {
            // get current mouse position
            m_currentPt = event->point;
            // draw new lines on canvas
            m_canvas->updateCanvas(); // TODO update only the relevant part of the canvas
        }
    } else {
        if (m_mode == MODE_DRAGING_NODE) {
            QPointF pos = convertToPixelCoord(event);
            m_selectedNode1->setX(pos.x());
            m_selectedNode1->setY(pos.y());
            m_canvas->updateCanvas(); // TODO update only the relevant part of the canvas
        }
        if (m_mode == MODE_DRAGING_TRANSLATING_TWONODES) {
            QPointF translate = convertToPixelCoord(event->point) - m_dragEnd;
            m_dragEnd = convertToPixelCoord(event->point);
            *m_selectedNode1 += translate;;
            *m_selectedNode2 += translate;;
            m_canvas->updateCanvas(); // TODO update only the relevant part of the canvas
        }
    }
    bool wasHiglightedNode = m_higlightedNode != 0;
    QPointF mousep = m_canvas->viewConverter()->documentToView(event->point);
    KisPerspectiveGrid* pGrid = m_canvas->view()->resourceProvider()->currentImage()->perspectiveGrid();
    for (QList<KisSubPerspectiveGrid*>::const_iterator it = pGrid->begin(); it != pGrid->end(); ++it) {
        KisSubPerspectiveGrid* grid = *it;
        if (m_higlightedNode = nodeNearPoint(grid, mousep)) {
            if (m_higlightedNode == m_selectedNode1 || m_higlightedNode == m_selectedNode2) {
                m_higlightedNode = 0;
            } else {
                m_canvas->updateCanvas(); // TODO update only the relevant part of the canvas
                break;
            }
        }
    }
    if (wasHiglightedNode && !m_higlightedNode) {
        m_canvas->updateCanvas(); // TODO update only the relevant part of the canvas
    }
}

void KisToolPerspectiveGrid::mouseReleaseEvent(KoPointerEvent *event)
{

    if (m_mode == MODE_CREATION) {
        if (m_drawing && event->button() == Qt::LeftButton && !m_isFirstPoint)  {
            m_points.append(m_currentPt);
            if (m_points.size() == 4) { // wow we have a grid, isn't that cool ?
                m_canvas->view()->resourceProvider()->currentImage()->perspectiveGrid()->addNewSubGrid(
                    new KisSubPerspectiveGrid(
                        new KisPerspectiveGridNode(convertToPixelCoord(m_points[0])),
                        new KisPerspectiveGridNode(convertToPixelCoord(m_points[1])),
                        new KisPerspectiveGridNode(convertToPixelCoord(m_points[2])),
                        new KisPerspectiveGridNode(convertToPixelCoord(m_points[3]))));
                m_canvas->view()->perspectiveGridManager()->setVisible(true);
                m_mode = MODE_EDITING;
                useCursor(KisCursor::arrowCursor());
            }
        }
        m_drawing = false;
        m_canvas->updateCanvas(); // TODO update only the relevant part of the canvas
    } else {
        m_mode = MODE_EDITING;
        // Check if there is a need for merging two nodes
        if (m_higlightedNode && m_selectedNode2 == 0) {
            m_higlightedNode->mergeWith(m_selectedNode1);
            m_canvas->updateCanvas(); // TODO update only the relevant part of the canvas
        }
        m_selectedNode1 = 0;
        m_selectedNode2 = 0;
    }

}

void KisToolPerspectiveGrid::paint(QPainter& gc, const KoViewConverter &converter)
{
    Q_UNUSED(converter);
    if (m_mode == MODE_CREATION) {
        drawGridCreation(gc);
    } else {
        drawGrid(gc);
    }
}

void KisToolPerspectiveGrid::drawGridCreation(QPainter& gc)
{
    dbgPlugins << "drawGridCreation";

    KisConfig cfg;
    QPen pen = QPen(cfg.getGridMainColor(), 1, Qt::SolidLine);

    gc.setPen(pen);
    gc.setRenderHint(QPainter::Antialiasing);

    for (QPointFVector::iterator iter = m_points.begin(); iter != m_points.end(); iter++) {
        if (iter + 1 == m_points.end())
            break;
        else
            gc.drawLine(m_canvas->viewConverter()->documentToView(*iter).toPoint(), m_canvas->viewConverter()->documentToView(*(iter + 1)).toPoint());
    }
    if (!m_points.isEmpty()) {
        gc.drawLine(m_canvas->viewConverter()->documentToView(*(m_points.end() - 1)).toPoint(), m_canvas->viewConverter()->documentToView(m_currentPt).toPoint());
        gc.drawLine(m_canvas->viewConverter()->documentToView(m_currentPt).toPoint(), m_canvas->viewConverter()->documentToView(*m_points.begin()).toPoint());
    }
}

void KisToolPerspectiveGrid::drawSmallRectangle(QPainter& gc, const QPointF& p)
{
    gc.drawRect(p.x() - m_handleHalfSize, p.y() - m_handleHalfSize, m_handleSize, m_handleSize);
}

void KisToolPerspectiveGrid::drawGrid(QPainter& gc)
{
    dbgPlugins << "drawGrid";

    KisConfig cfg;
    QPen pen = QPen(cfg.getGridMainColor(), 1, Qt::SolidLine);

    QPointF startPos;
    QPointF endPos;

    gc.setPen(pen);
    gc.setRenderHint(QPainter::Antialiasing);
//     gc.setRasterOp(Qt::XorROP);
    KisPerspectiveGrid* pGrid = m_canvas->view()->resourceProvider()->currentImage()->perspectiveGrid();

    for (QList<KisSubPerspectiveGrid*>::const_iterator it = pGrid->begin(); it != pGrid->end(); ++it) {
        KisSubPerspectiveGrid* grid = *it;

        { // Draw top
            startPos = pixelToView(*grid->topLeft());
            endPos = pixelToView(*grid->topRight());
            gc.drawLine(startPos, endPos);
            drawSmallRectangle(gc, (endPos + startPos) / 2.); // Draw top-middle handle
            drawSmallRectangle(gc, startPos); // draw top-left handle
            drawSmallRectangle(gc, endPos); // draw top-right handle
        }
        { // Draw Right
            startPos = pixelToView(*grid->topRight());
            endPos = pixelToView(*grid->bottomRight());
            gc.drawLine(startPos, endPos);
            drawSmallRectangle(gc, (endPos + startPos) / 2.); // Draw right-middle handle
        }
        { // Draw bottoom
            startPos = pixelToView(*grid->bottomRight());
            endPos = pixelToView(*grid->bottomLeft());
            gc.drawLine(startPos, endPos);
            drawSmallRectangle(gc, (endPos + startPos) / 2.); // Draw bottom-middle handle
            drawSmallRectangle(gc, endPos); // Draw bottom-left handle
            drawSmallRectangle(gc, startPos); // Draw bottom-right handle
        }
        { // Draw Left
            startPos = pixelToView(*grid->bottomLeft());
            endPos = pixelToView(*grid->topLeft());
            gc.drawLine(startPos, endPos);
            drawSmallRectangle(gc, (endPos + startPos) / 2.); // Draw left-middle handle
        }
        // Draw delete icon
        KIcon iconDelete("edit-delete");
        QPointF iconDeletePos = pixelToView(grid->center());
        gc.drawPixmap(iconDeletePos - QPointF(16, 16), iconDelete.pixmap(32, 32));
        // Draw Vanishing point
        QPointF tbVpf = grid->topBottomVanishingPoint();
        if (fabs(tbVpf.x()) < 30000000. && fabs(tbVpf.y()) < 30000000.) {
            QPointF tbVp = pixelToView(tbVpf);
            gc.drawLine(tbVp.x() - m_handleHalfSize, tbVp.y() - m_handleHalfSize, tbVp.x() + m_handleHalfSize, tbVp.y() + m_handleHalfSize);
            gc.drawLine(tbVp.x() - m_handleHalfSize, tbVp.y() + m_handleHalfSize, tbVp.x() + m_handleHalfSize, tbVp.y() - m_handleHalfSize);
        }
        // Draw Vanishing Point
        QPointF lrVpf = grid->leftRightVanishingPoint();
        if (fabs(lrVpf.x()) < 30000000. && fabs(lrVpf.y()) < 30000000.) { // Don't display it, if it is too far, or you get funny results
            QPointF lrVp = pixelToView(lrVpf);
            gc.drawLine(lrVp.x() - m_handleHalfSize, lrVp.y() - m_handleHalfSize, lrVp.x() + m_handleHalfSize, lrVp.y() + m_handleHalfSize);
            gc.drawLine(lrVp.x() - m_handleHalfSize, lrVp.y() + m_handleHalfSize, lrVp.x() + m_handleHalfSize, lrVp.y() - m_handleHalfSize);
        }
    }
    if (m_higlightedNode) {
        gc.setBrush(cfg.getGridMainColor());
        drawSmallRectangle(gc, pixelToView(*m_higlightedNode));
    }
}

#include "kis_tool_perspectivegrid.moc"
