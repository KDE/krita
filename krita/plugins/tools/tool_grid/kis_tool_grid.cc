/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
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
#include "kis_tool_grid.h"

#include <KoViewConverter.h>
#include <KoCanvasController.h>
#include <KoPointerEvent.h>

#include <qpainter.h>

#include <kis_debug.h>
#include <klocale.h>

#include <canvas/kis_grid_manager.h>
#include <canvas/kis_canvas2.h>
#include <kis_config.h>
#include <kis_cursor.h>
#include <kis_view2.h>


KisToolGrid::KisToolGrid(KoCanvasBase * canvas)
        : KisTool(canvas, KisCursor::moveCursor()), m_canvas(dynamic_cast<KisCanvas2*>(canvas))
{
    Q_ASSERT(m_canvas);
    setObjectName("tool_grid");
    m_dragging = false;
}

KisToolGrid::~KisToolGrid()
{
}

void KisToolGrid::activate(bool)
{
    m_canvas->view()->gridManager()->setVisible(true);
    m_canvas->view()->gridManager()->checkVisibilityAction(true);
    m_canvas->updateCanvas();
    KisTool::activate();
}

void KisToolGrid::deactivate()
{
    // Add code here to initialize your tool when it got deactivated
    KisTool::deactivate();
}

void KisToolGrid::mousePressEvent(KoPointerEvent *event)
{
    m_dragging = true;
    m_dragStart = convertToPixelCoord(event);
    KisConfig cfg;
    if (event->modifiers() == Qt::ControlModifier) {
        m_currentMode = SCALE;
    } else {
        m_currentMode = TRANSLATION;
    }

    if (m_currentMode == TRANSLATION) {
        m_initialOffset = QPoint(cfg.getGridOffsetX(), cfg.getGridOffsetY());
    } else {
        m_initialSpacing = QPoint(cfg.getGridHSpacing(), cfg.getGridVSpacing());
    }
}


void KisToolGrid::mouseMoveEvent(KoPointerEvent *event)
{
    if (m_dragging) {
        KisConfig cfg;
        int subdivisions = cfg.getGridSubdivisions();
        m_dragEnd = convertToPixelCoord(event);
        if (m_currentMode == TRANSLATION) {
            QPoint newoffset = m_initialOffset + (m_dragEnd - m_dragStart).toPoint();
            cfg.setGridOffsetX(newoffset.x() % (cfg.getGridHSpacing() * subdivisions));
            cfg.setGridOffsetY(newoffset.y() % (cfg.getGridVSpacing() * subdivisions));
        } else { // SCALE
            QPoint newSize = m_initialSpacing + (m_dragEnd - m_dragStart).toPoint();
            if (newSize.x() >= 1) cfg.setGridHSpacing(newSize.x());
            if (newSize.y() >= 1) cfg.setGridVSpacing(newSize.y());
        }
        m_canvas->updateCanvas();
    } else {
        event->ignore();
    }
}

void KisToolGrid::mouseReleaseEvent(KoPointerEvent *event)
{
    Q_UNUSED(event);
    m_dragging = false;
}

void KisToolGrid::paint(QPainter& gc, const KoViewConverter &converter)
{
    Q_UNUSED(gc);
    Q_UNUSED(converter);
}

#include "kis_tool_grid.moc"
